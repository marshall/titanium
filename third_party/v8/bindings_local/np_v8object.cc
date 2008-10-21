/*
 * Copyright (C) 2004, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Google, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Forked with few modifications from:
// https://svn/chrome/trunk/webkit/port/bindings/v8/np_v8object.cpp@16743

#include "np_v8object.h"

#include "gears/base/common/js_types.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/npapi/browser_utils.h"
#include "v8_np_utils.h"
#include "v8_npobject.h"
#include "v8_helpers.h"
#include "v8_npruntime.h"

namespace {

// TODO(mbelshe): comments on why use malloc and free.
static NPObject* AllocV8NPObject(NPP, NPClass*) {
  return static_cast<NPObject*>(malloc(sizeof(V8NPObject)));
}

static void FreeV8NPObject(NPObject* npobj) {
  // Use v8::Locker to allow reentrancy.  This function can be called
  // recursively.
  v8::Locker locker;
  V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);
  object->v8_object.Dispose();
  free(object);
}

static v8::Handle<v8::Value>* listFromVariantArgs(const NPVariant* args,
                                                  uint32_t argCount,
                                                  NPObject *owner) {
  v8::Handle<v8::Value>* argv = new v8::Handle<v8::Value>[argCount];
  for (uint32_t index = 0; index < argCount; index++) {
    const NPVariant *arg = &args[index];
    argv[index] = ConvertNPVariantToV8Object(arg, owner);
  }
  return argv;
}

// Create an identifier (null terminated utf8 char*) from the NPIdentifier.
static void NPIdentifierToV8Identifier(NPIdentifier name, std::string &string) {
  PrivateIdentifier* identifier = static_cast<PrivateIdentifier*>(name);
  if (identifier->isString) {
    string = static_cast<const char *>(identifier->value.string);
  } else {
    string = IntegerToString(identifier->value.number);
  }
}

static NPClass V8NPObjectClass = { NP_CLASS_STRUCT_VERSION,
                                   AllocV8NPObject,
                                   FreeV8NPObject,
                                   0, 0, 0, 0, 0, 0, 0, 0, 0 };

}  // namespace

static bool V8_NPN_EvaluateInternal(NPP npp, NPObject *npobj,
                                    NPString *npscript, NPVariant *result);

//
// NPAPI's npruntime functions
//
NPClass* NPScriptObjectClass = &V8NPObjectClass;

NPObject *V8_NPN_GetGlobalObject(NPP npp) {
  v8::Locker::AssertIsLocked();
  v8::HandleScope handle_scope;
  v8::Handle<v8::Context> context = GetV8Context(npp, NULL);
  if (context.IsEmpty())
    return 0;

  v8::Context::Scope scope(context);
  v8::Handle<v8::Object> global = context->Global();
  return V8_NPN_CreateScriptObject(0, context->Global());
}

NPObject* V8_NPN_CreateScriptObject(NPP npp, v8::Handle<v8::Object> object) {
  v8::Locker::AssertIsLocked();
  // Check to see if this object is already wrapped.
  if (object->InternalFieldCount() == 3 &&
      object->GetInternalField(1)->IsNumber() &&
      object->GetInternalField(1)->Uint32Value() == V8ClassIndex::NPOBJECT) {
    NPObject* rv = V8Proxy::ToNativeObject<NPObject>(V8ClassIndex::NPOBJECT,
                                                     object);
    V8_NPN_RetainObject(rv);
    return rv;
  }

  V8NPObject* obj =
      reinterpret_cast<V8NPObject*>(V8_NPN_CreateObject(npp, &V8NPObjectClass));
  obj->v8_object = v8::Persistent<v8::Object>::New(object);
  return reinterpret_cast<NPObject*>(obj);
}

bool V8_NPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName,
		   const NPVariant *args, uint32_t argCount, NPVariant *result) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    PrivateIdentifier *identifier = static_cast<PrivateIdentifier*>(methodName);
    if (!identifier->isString)
      return false;

    V8Locker locker;
    v8::HandleScope handle_scope;
    // TODO: should use the plugin's owner frame as the security context
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;

    v8::Context::Scope scope(context);

    // Special case the "eval" method.
    if (methodName == V8_NPN_GetStringIdentifier("eval")) {
      if (argCount != 1)
        return false;
      if (args[0].type != NPVariantType_String)
        return false;
      return V8_NPN_EvaluateInternal(npp, npobj,
      	 const_cast<NPString*>(&args[0].value.stringValue), result);
    }

    v8::Handle<v8::Value> func_obj =
        object->v8_object->Get(v8::String::New(identifier->value.string));
    if (func_obj.IsEmpty() || func_obj->IsNull()) {
      NULL_TO_NPVARIANT(*result);
      return false;
    }
    if (func_obj->IsUndefined()) {
      VOID_TO_NPVARIANT(*result);
      return false;
    }

    // TODO: fix variable naming
    // Call the function object
    v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(func_obj);
    // Create list of args to pass to v8
    v8::Handle<v8::Value>* argv = listFromVariantArgs(args, argCount, npobj);
    v8::Local<v8::Value> resultObj = func->Call(object->v8_object, argCount, argv);
    delete[] argv;

    // If we had an error, return false.  The spec is a little unclear here, but
    // says "Returns true if the method was successfully invoked".  If we get an
    // error return value, was that successfully invoked?
    if (resultObj.IsEmpty()) return false;

    // Convert the result back to an NPVariant
    ConvertV8ObjectToNPVariant(resultObj, npobj, result);
    return true;
  }

  if (npobj->_class->invoke)
    return npobj->_class->invoke(npobj, methodName, args, argCount, result);

  VOID_TO_NPVARIANT(*result);
  return true;
}


// TODO: Fix it same as V8_NPN_Invoke (HandleScope and such)
bool V8_NPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args,
			  uint32_t argCount, NPVariant *result) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    VOID_TO_NPVARIANT(*result);

    V8Locker locker;
    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;

    v8::Context::Scope scope(context);

    // Lookup the function object
    v8::Handle<v8::Object> funcObj(object->v8_object);
    if (!funcObj->IsFunction())
      return false;

    // Call the function object
    v8::Local<v8::Value> resultObj;
    v8::Handle<v8::Function> func(v8::Function::Cast(*funcObj));
    if (!func->IsNull()) {
      // Create list of args to pass to v8
      v8::Handle<v8::Value>* argv = listFromVariantArgs(args, argCount, npobj);
      resultObj = func->Call(funcObj, argCount, argv);
      delete[] argv;
    }

    // If we had an error, return false.  The spec is a little unclear here, but
    // says "Returns true if the method was successfully invoked".  If we get an
    // error return value, was that successfully invoked?
    if (resultObj.IsEmpty()) return false;

    // Convert the result back to an NPVariant
    ConvertV8ObjectToNPVariant(resultObj, npobj, result);
    return true;
  }

  if (npobj->_class->invokeDefault)
    return npobj->_class->invokeDefault(npobj, args, argCount, result);

  VOID_TO_NPVARIANT(*result);
  return true;
}

bool V8_NPN_Evaluate(NPP npp, NPObject *npobj, NPString *npscript, NPVariant *result) {
  V8Locker locker;
  return V8_NPN_EvaluateInternal(npp, npobj, npscript, result);
}

static bool V8_NPN_EvaluateInternal(NPP npp, NPObject *npobj,
                                    NPString *npscript, NPVariant *result) {
  v8::Locker::AssertIsLocked();
  VOID_TO_NPVARIANT(*result);

  v8::HandleScope handle_scope;
  v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty())
      return false;

  v8::Context::Scope scope(context);

  v8::Local<v8::Script> script = v8::Script::Compile(
      v8::String::New(npscript->UTF8Characters, npscript->UTF8Length));
  if (script.IsEmpty()) return false;

  v8::Local<v8::Value> v8result = script->Run();

  // If we had an error, return false.
  if (v8result.IsEmpty()) return false;

  ConvertV8ObjectToNPVariant(v8result, npobj, result);
  return true;
}

bool V8_NPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                        NPVariant *result) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    V8Locker locker;
    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;

    v8::Context::Scope scope(context);

    v8::Handle<v8::Object> obj(object->v8_object);

    std::string identifier;
    NPIdentifierToV8Identifier(propertyName, identifier);
    v8::Local<v8::Value> v8result =
        obj->Get(v8::String::New(identifier.c_str()));

    ConvertV8ObjectToNPVariant(v8result, npobj, result);
    return true;
  }

  if (npobj->_class->hasProperty && npobj->_class->getProperty)
    if (npobj->_class->hasProperty(npobj, propertyName))
      return npobj->_class->getProperty(npobj, propertyName, result);

  VOID_TO_NPVARIANT(*result);
  return false;
}

bool V8_NPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                        const NPVariant *value) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    V8Locker locker;
    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;

    v8::Context::Scope scope(context);

    v8::Handle<v8::Object> obj(object->v8_object);
    std::string identifier;
    NPIdentifierToV8Identifier(propertyName, identifier);
    obj->Set(v8::String::New(identifier.c_str()),
        ConvertNPVariantToV8Object(value, GetGlobalNPObject(npp)));
    return true;
  }

  if (npobj->_class->setProperty)
    return npobj->_class->setProperty(npobj, propertyName, value);

  return false;
}

bool V8_NPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    V8Locker locker;
    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;
    v8::Context::Scope scope(context);

    v8::Handle<v8::Object> obj(object->v8_object);
    std::string identifier;
    NPIdentifierToV8Identifier(propertyName, identifier);
    // TODO(mbelshe) - verify that setting to undefined is right.
    obj->Set(v8::String::New(identifier.c_str()), v8::Undefined());
    return true;
  }

  return false;
}

bool V8_NPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    V8Locker locker;
    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;
    v8::Context::Scope scope(context);

    v8::Handle<v8::Object> obj(object->v8_object);
    std::string identifier;
    NPIdentifierToV8Identifier(propertyName, identifier);
    return obj->Has(v8::String::New(identifier.c_str()));
  }

  if (npobj->_class->hasProperty)
    return npobj->_class->hasProperty(npobj, propertyName);

  return false;
}

bool V8_NPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName) {
  if (npobj == NULL) return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    V8Locker locker;
    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;
    v8::Context::Scope scope(context);

    v8::Handle<v8::Object> obj(object->v8_object);
    std::string identifier;
    NPIdentifierToV8Identifier(methodName, identifier);
    v8::Handle<v8::Value> prop = obj->Get(v8::String::New(identifier.c_str()));
    return prop->IsFunction();
  }

  if (npobj->_class->hasMethod)
    return npobj->_class->hasMethod(npobj, methodName);

  return false;
}

void V8_NPN_SetException(NPObject *npobj, const NPUTF8 *message) {
  NPP npp = BrowserUtils::GetCurrentJsCallContext()->js_context();

  V8Locker locker;
  v8::HandleScope handle_scope;
  v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
  if (context.IsEmpty()) return;

  v8::Context::Scope scope(context);
  V8Proxy::ThrowError(V8Proxy::GENERAL_ERROR, message);
}

bool V8_NPN_Enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
                      uint32_t *count) {
  if (npobj == NULL) return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8Locker locker;
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;
    v8::Context::Scope scope(context);

    v8::Handle<v8::Object> obj(object->v8_object);

    // TODO(fqian): http://b/issue?id=1210340: Use a v8::Object::Keys() method
    // when it exists, instead of evaluating javascript.

    // TODO(mpcomplete): figure out how to cache this helper function.
    // Run a helper function that collects the properties on the object into
    // an array.
    const char kEnumeratorCode[] =
      "(function (obj) {"
      "  var props = [];"
      "  for (var prop in obj) {"
      "    props[props.length] = prop;"
      "  }"
      "  return props;"
      "});";
    v8::Handle<v8::String> source = v8::String::New(kEnumeratorCode);
    v8::Handle<v8::Script> script = v8::Script::Compile(source, NULL);
    v8::Handle<v8::Value> enumerator_obj = script->Run();
    v8::Handle<v8::Function> enumerator =
        v8::Handle<v8::Function>::Cast(enumerator_obj);
    v8::Handle<v8::Value> argv[] = { obj };
    v8::Local<v8::Value> props_obj =
        enumerator->Call(v8::Handle<v8::Object>::Cast(enumerator_obj),
                         ARRAYSIZE(argv), argv);
    if (props_obj.IsEmpty())
      return false;

    // Convert the results into an array of NPIdentifiers.
    v8::Handle<v8::Array> props = v8::Handle<v8::Array>::Cast(props_obj);
    *count = props->Length();
    *identifier =
        static_cast<NPIdentifier*>(malloc(sizeof(NPIdentifier*) * *count));
    for (uint32_t i = 0; i < *count; ++i) {
      v8::Local<v8::Value> name = props->Get(v8::Integer::New(i));
      (*identifier)[i] = GetStringIdentifier(v8::Local<v8::String>::Cast(name));
    }
    return true;
  }

  if (NP_CLASS_STRUCT_VERSION_HAS_ENUM(npobj->_class) &&
      npobj->_class->enumerate) {
     return npobj->_class->enumerate(npobj, identifier, count);
  }

  return false;
}
