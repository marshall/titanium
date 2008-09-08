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

#include "config.h"

#define max max
#define min min
#include <string>
#include <sstream>
#include <v8.h>
#include "np_v8object.h"
#include "Frame.h"
#include "bindings/npruntime.h"
#include "npruntime_priv.h"
#include "PlatformString.h"
#include "v8_helpers.h"
#include "v8_np_utils.h"
#include "v8_proxy.h"
#include "V8Bridge.h"
#include "DOMWindow.h"

using WebCore::V8ClassIndex;
using WebCore::V8Proxy;

namespace {

// TODO(mbelshe): comments on why use malloc and free.
static NPObject* AllocV8NPObject(NPP, NPClass*) {
  return static_cast<NPObject*>(malloc(sizeof(V8NPObject)));
}

static void FreeV8NPObject(NPObject* npobj) {
  V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);
#ifndef NDEBUG
  V8Proxy::UnregisterGlobalHandle(object, object->v8_object);
#endif
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
    std::ostringstream o;
    o << identifier->value.number;
    string = o.str();
  }
}

static NPClass V8NPObjectClass = { NP_CLASS_STRUCT_VERSION,
                                   AllocV8NPObject,
                                   FreeV8NPObject,
                                   0, 0, 0, 0, 0, 0, 0, 0, 0 };

}  // namespace

//
// NPAPI's npruntime functions
//
NPClass* NPScriptObjectClass = &V8NPObjectClass;

NPObject* NPN_CreateScriptObject(NPP npp, v8::Handle<v8::Object> object,
                                 WebCore::DOMWindow* root) {
  // Check to see if this object is already wrapped.
  if (object->InternalFieldCount() == 3 &&
      object->GetInternalField(1)->IsNumber() &&
      object->GetInternalField(1)->Uint32Value() == V8ClassIndex::NPOBJECT) {
    NPObject* rv = V8Proxy::ToNativeObject<NPObject>(V8ClassIndex::NPOBJECT,
                                                     object);
    NPN_RetainObject(rv);
    return rv;
  }

  V8NPObject* obj =
      reinterpret_cast<V8NPObject*>(NPN_CreateObject(npp, &V8NPObjectClass));
  obj->v8_object = v8::Persistent<v8::Object>::New(object);
#ifndef NDEBUG
  V8Proxy::RegisterGlobalHandle(WebCore::NPOBJECT, obj, obj->v8_object);
#endif
  obj->root_object = root;
  return reinterpret_cast<NPObject*>(obj);
}

bool NPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName,
                const NPVariant *args, uint32_t argCount, NPVariant *result) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    PrivateIdentifier *identifier = static_cast<PrivateIdentifier*>(methodName);
    if (!identifier->isString)
      return false;

    v8::HandleScope handle_scope;
    // TODO: should use the plugin's owner frame as the security context
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;

    v8::Context::Scope scope(context);

    // Special case the "eval" method.
    if (methodName == NPN_GetStringIdentifier("eval")) {
      if (argCount != 1)
        return false;
      if (args[0].type != NPVariantType_String)
        return false;
      return NPN_Evaluate(npp, npobj,
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

    WebCore::V8Proxy* proxy = GetV8Proxy(npobj);
    ASSERT(proxy);  // must not be null

    // TODO: fix variable naming
    // Call the function object
    v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(func_obj);
    // Create list of args to pass to v8
    v8::Handle<v8::Value>* argv = listFromVariantArgs(args, argCount, npobj);
    v8::Local<v8::Value> resultObj =
        proxy->CallFunction(func, object->v8_object, argCount, argv);
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


// TODO: Fix it same as NPN_Invoke (HandleScope and such)
bool NPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args,
                       uint32_t argCount, NPVariant *result) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    VOID_TO_NPVARIANT(*result);

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
      WebCore::V8Proxy* proxy = GetV8Proxy(npobj);
      ASSERT(proxy);

      // Create list of args to pass to v8
      v8::Handle<v8::Value>* argv = listFromVariantArgs(args, argCount, npobj);
      resultObj = proxy->CallFunction(func, funcObj, argCount, argv);
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

bool NPN_Evaluate(NPP npp, NPObject *npobj, NPString *npscript,
                  NPVariant *result) {
  VOID_TO_NPVARIANT(*result);
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty())
      return false;

    WebCore::V8Proxy* proxy = GetV8Proxy(npobj);
    ASSERT(proxy);

    v8::Context::Scope scope(context);

    WebCore::String filename(L"npscript");
    // Convert UTF-8 stream to WebCore::String.
    WebCore::String script = WebCore::String::fromUTF8(
        npscript->UTF8Characters, npscript->UTF8Length);
    v8::Local<v8::Value> v8result =
        proxy->Evaluate(filename, 0, script, NULL);

    // If we had an error, return false.
    if (v8result.IsEmpty()) return false;

    ConvertV8ObjectToNPVariant(v8result, npobj, result);
    return true;
  }

  return false;
}

bool NPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     NPVariant *result) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

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

bool NPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName,
                     const NPVariant *value) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(npp, npobj);
    if (context.IsEmpty()) return false;

    v8::Context::Scope scope(context);

    v8::Handle<v8::Object> obj(object->v8_object);
    std::string identifier;
    NPIdentifierToV8Identifier(propertyName, identifier);
    obj->Set(v8::String::New(identifier.c_str()),
        ConvertNPVariantToV8Object(value,
            object->root_object->frame()->windowScriptNPObject()));
    return true;
  }

  if (npobj->_class->setProperty)
    return npobj->_class->setProperty(npobj, propertyName, value);

  return false;
}

bool NPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

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

bool NPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName) {
  if (npobj == NULL)
    return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

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

bool NPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName) {
  if (npobj == NULL) return false;

  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

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

void NPN_SetException(NPObject *npobj, const NPUTF8 *message) {
  if (npobj->_class == NPScriptObjectClass) {
    V8NPObject *object = reinterpret_cast<V8NPObject*>(npobj);

    v8::HandleScope handle_scope;
    v8::Handle<v8::Context> context = GetV8Context(NULL, npobj);
    if (context.IsEmpty()) return;

    v8::Context::Scope scope(context);

    V8Proxy::ThrowError(V8Proxy::GENERAL_ERROR, message);
  }
}

bool NPN_Enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
                   uint32_t *count) {
  if (npobj == NULL) return false;

  if (npobj->_class == NPScriptObjectClass) {
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
                         arraysize(argv), argv);
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
