// Copyright (2007) Google Inc. All Rights Reserved.

// Forked with few modifications from:
// https://svn/chrome/trunk/webkit/port/bindings/v8/v8_npobject.cpp@16743

#include "v8_npobject.h"

#include "dom_wrapper_map.h"
#include "np_v8object.h"
#include "v8_np_utils.h"
#include "v8_helpers.h"
#include "v8_npruntime.h"

// Turn this on if we should use the V8 memory hint API.  The version of V8
// we're releasing does not have this API yet, so this is temporarily disabled.
#ifdef USE_MEMORY_HINTS
// For each Gears object that has a V8 wrapper, we tell V8 that memory usage has
// gone up by this amount.  Calculating the real value is tricky, so instead we
// just use a constant.  This effectively puts a soft cap on the number of Gears
// objects that can be around before V8 will attempt to GC.
static const int kGearsObjectSize = 1024;
#endif

enum InvokeFunctionType {
  INVOKE_METHOD = 1,
  INVOKE_DEFAULT = 2
};

// TODO(mbelshe): need comments.
// Params: holder could be HTMLEmbedElement or NPObject
static v8::Handle<v8::Value> NPObjectInvokeImpl(
    const v8::Arguments& args, InvokeFunctionType func_id) {
   ASSERT(args.Holder()->InternalFieldCount() == 3);
  NPObject* npobject = V8Proxy::ToNativeObject<NPObject>(V8ClassIndex::NPOBJECT,
                                                         args.Holder());
  // Verify that our wrapper wasn't using a NPObject which
  // has already been deleted.
  if (!npobject || !_NPN_IsAlive(npobject)) {
    V8Proxy::ThrowError(V8Proxy::REFERENCE_ERROR, "NPObject deleted");
    return v8::Undefined();
  }

  // wrap up parameters
  int argc = args.Length();
  NPVariant* np_args = new NPVariant[argc];

  for (int i = 0; i < argc; i++) {
    ConvertV8ObjectToNPVariant(args[i], npobject, &np_args[i]);
  }

  NPVariant result;
  VOID_TO_NPVARIANT(result);

  switch (func_id) {
    case INVOKE_METHOD:
      if (npobject->_class->invoke) {
        v8::Handle<v8::String> function_name(v8::String::Cast(*args.Data()));
        NPIdentifier ident = GetStringIdentifier(function_name);
        V8Unlocker unlocker;
        npobject->_class->invoke(npobject, ident, np_args, argc, &result);
      }
      break;
    case INVOKE_DEFAULT:
      if (npobject->_class->invokeDefault) {
      	V8Unlocker unlocker;
        npobject->_class->invokeDefault(npobject, np_args, argc, &result);
      }
      break;
    default:
      break;
  }

  for (int i=0; i < argc; i++) {
    V8_NPN_ReleaseVariantValue(&np_args[i]);
  }
  delete[] np_args;

  // unwrap return values
  v8::Handle<v8::Value> rv = ConvertNPVariantToV8Object(&result, npobject);
  V8_NPN_ReleaseVariantValue(&result);

  return rv;
}

// V8 callback - V8 is locked at this point.
v8::Handle<v8::Value> NPObjectMethodHandler(const v8::Arguments& args) {
  return NPObjectInvokeImpl(args, INVOKE_METHOD);
}


v8::Handle<v8::Value> NPObjectInvokeDefaultHandler(const v8::Arguments& args) {
  return NPObjectInvokeImpl(args, INVOKE_DEFAULT);
}


static void WeakTemplateCallback(v8::Persistent<v8::Object> obj, void* param);

// NPIdentifier is PrivateIdentifier*.
static WeakReferenceMap<PrivateIdentifier, v8::FunctionTemplate> \
    static_template_map(&WeakTemplateCallback);

static void WeakTemplateCallback(v8::Persistent<v8::Object> obj,
                                 void* param) {
  PrivateIdentifier* iden = static_cast<PrivateIdentifier*>(param);
  ASSERT(iden != NULL);
  ASSERT(static_template_map.contains(iden));

  static_template_map.forget(iden);
}

static v8::Handle<v8::Value> NPObjectGetProperty(v8::Local<v8::Object> self,
                                                 NPIdentifier ident,
                                                 v8::Local<v8::Value> key) {
  NPObject* npobject = V8Proxy::ToNativeObject<NPObject>(V8ClassIndex::NPOBJECT,
                                                         self);

  // Verify that our wrapper wasn't using a NPObject which
  // has already been deleted.
  if (!npobject || !_NPN_IsAlive(npobject)) {
    V8Proxy::ThrowError(V8Proxy::REFERENCE_ERROR, "NPObject deleted");
    return v8::Handle<v8::Value>();
  }

  if (npobject->_class->hasProperty &&
      npobject->_class->hasProperty(npobject, ident) &&
      npobject->_class->getProperty) {
    NPVariant result;
    VOID_TO_NPVARIANT(result);
    {
      V8Unlocker unlocker;
      if (!npobject->_class->getProperty(npobject, ident, &result)) {
        return v8::Handle<v8::Value>();
      }
    }
    v8::Handle<v8::Value> rv = ConvertNPVariantToV8Object(&result, npobject);
    V8_NPN_ReleaseVariantValue(&result);

    return rv;
  } else if (key->IsString() &&
             npobject->_class->hasMethod &&
             npobject->_class->hasMethod(npobject, ident)) {
    PrivateIdentifier* id = static_cast<PrivateIdentifier*>(ident);
    v8::Persistent<v8::FunctionTemplate> desc = static_template_map.get(id);
    // Cache templates using identifier as the key.
    if (desc.IsEmpty()) {
      // Create a new template
      v8::Local<v8::FunctionTemplate> temp = v8::FunctionTemplate::New();
      temp->SetCallHandler(NPObjectMethodHandler, key);
      desc = v8::Persistent<v8::FunctionTemplate>::New(temp);
      static_template_map.set(id, desc);
    }

    // FunctionTemplate caches function for each context.
    v8::Local<v8::Function> func = desc->GetFunction();
    func->SetName(v8::Handle<v8::String>::Cast(key));
    return func;
  }

  return v8::Handle<v8::Value>();
}

v8::Handle<v8::Value> NPObjectNamedPropertyGetter(
    v8::Local<v8::String> name, const v8::AccessorInfo& info) {
  NPIdentifier ident = GetStringIdentifier(name);
  return NPObjectGetProperty(info.Holder(), ident, name);
}

v8::Handle<v8::Value> NPObjectIndexedPropertyGetter(
    uint32_t index, const v8::AccessorInfo& info) {
  NPIdentifier ident = V8_NPN_GetIntIdentifier(index);
  return NPObjectGetProperty(info.Holder(), ident, v8::Number::New(index));
}

// V8 callback - V8 is locked at this point.
static v8::Handle<v8::Value> NPObjectSetProperty(v8::Local<v8::Object> self,
                                                 NPIdentifier ident,
                                                 v8::Local<v8::Value> value) {
  NPObject* npobject = V8Proxy::ToNativeObject<NPObject>(V8ClassIndex::NPOBJECT,
                                                         self);

  // Verify that our wrapper wasn't using a NPObject which
  // has already been deleted.
  if (!npobject || !_NPN_IsAlive(npobject)) {
    V8Proxy::ThrowError(V8Proxy::REFERENCE_ERROR, "NPObject deleted");
    return value;  // intercepted, but an exception was thrown
  }

  if (npobject->_class->hasProperty &&
      npobject->_class->hasProperty(npobject, ident) &&
      npobject->_class->setProperty) {
    NPVariant npvalue;
    VOID_TO_NPVARIANT(npvalue);
    ConvertV8ObjectToNPVariant(value, npobject, &npvalue);

    bool result;
    {
      V8Unlocker unlocker;
      result = npobject->_class->setProperty(npobject, ident, &npvalue);
      V8_NPN_ReleaseVariantValue(&npvalue);
    }

    if (result)
      return value;  // intercept the call
  }
  return v8::Handle<v8::Value>();  // do not intercept the call
}

v8::Handle<v8::Value> NPObjectNamedPropertySetter(
    v8::Local<v8::String> name, v8::Local<v8::Value> value,
    const v8::AccessorInfo& info) {
  NPIdentifier ident = GetStringIdentifier(name);
  return NPObjectSetProperty(info.Holder(), ident, value);
}


v8::Handle<v8::Value> NPObjectIndexedPropertySetter(
    uint32_t index, v8::Local<v8::Value> value, const v8::AccessorInfo& info) {
  NPIdentifier ident = V8_NPN_GetIntIdentifier(index);
  return NPObjectSetProperty(info.Holder(), ident, value);
}

static void WeakNPObjectCallback(v8::Persistent<v8::Object> obj, void* param);

static DOMWrapperMap<NPObject> static_npobject_map(&WeakNPObjectCallback);

// This function is called when the V8 object that we created to wrap an
// NPObject (see CreateV8ObjectForNPObject) is garbage collected.  It may be
// called on any thread, so we should take special care to delete the NPObject
// on the thread that created it.
static void WeakNPObjectCallback(v8::Persistent<v8::Object> obj, void* param) {
  v8::Locker::AssertIsLocked();
  NPObject* npobject = static_cast<NPObject*>(param);
  ASSERT(npobject != NULL);
  ASSERT(static_npobject_map.contains(npobject));

  // Must remove from our map before calling NPN_ReleaseObject().
  // NPN_ReleaseObject can call ForgetV8ObjectForNPObject, which
  // uses the table as well.
  ThreadId creation_thread = static_npobject_map.forget(npobject);
  if (_NPN_IsAlive(npobject)) {
    if (creation_thread !=
        ThreadMessageQueue::GetInstance()->GetCurrentThreadId()) {
      // The object was created on a different thread.  Add it to the list
      // of objects to release for that thread.
      NPObjectReleaseList::AddForThread(npobject, creation_thread);
    } else {
      V8_NPN_ReleaseObject(npobject);
    }
  }

#ifdef USE_MEMORY_HINTS
  v8::V8::AdjustAmountOfExternalAllocatedMemory(-kGearsObjectSize);
#endif
}

v8::Local<v8::Object> CreateV8ObjectForNPObject(NPObject* object,
                                                NPObject* root) {
  static v8::Persistent<v8::FunctionTemplate> np_object_desc;

  v8::Locker::AssertIsLocked();
  ASSERT(v8::Context::InContext());

  // If this is a v8 object, just return it.
  if (object->_class == NPScriptObjectClass) {
    V8NPObject* v8npobject = reinterpret_cast<V8NPObject*>(object);
    return v8::Local<v8::Object>::New(v8npobject->v8_object);
  }

  // If we've already wrapped this object, just return it.
  if (static_npobject_map.contains(object))
    return v8::Local<v8::Object>::New(static_npobject_map.get(object));

  // TODO: we should create a Wrapper type as a subclass of JSObject.
  // It has two internal fields, field 0 is the wrapped pointer,
  // and field 1 is the type. There should be an api function that
  // returns unused type id.
  // The same Wrapper type can be used by DOM bindings.
  if (np_object_desc.IsEmpty()) {
    np_object_desc =
        v8::Persistent<v8::FunctionTemplate>::New(v8::FunctionTemplate::New());
    np_object_desc->InstanceTemplate()->SetInternalFieldCount(3);
    np_object_desc->InstanceTemplate()->SetNamedPropertyHandler(
        NPObjectNamedPropertyGetter, NPObjectNamedPropertySetter);
    np_object_desc->InstanceTemplate()->SetIndexedPropertyHandler(
        NPObjectIndexedPropertyGetter, NPObjectIndexedPropertySetter);
    np_object_desc->InstanceTemplate()->SetCallAsFunctionHandler(
        NPObjectInvokeDefaultHandler); 
  }

  v8::Handle<v8::Function> func = np_object_desc->GetFunction();
  v8::Local<v8::Object> value = SafeAllocation::NewInstance(func);
  
  // If we were unable to allocate the instance we avoid wrapping 
  // and registering the NP object. 
  if (value.IsEmpty()) 
      return value;

  WrapNPObject(value, object);

  V8_NPN_RetainObject(object);

  _NPN_RegisterObject(object, root);

  // Maintain a weak pointer for v8 so we can cleanup the object.
  v8::Persistent<v8::Object> weak_ref = v8::Persistent<v8::Object>::New(value);
  static_npobject_map.set(object, weak_ref);

#ifdef USE_MEMORY_HINTS
  v8::V8::AdjustAmountOfExternalAllocatedMemory(kGearsObjectSize);
#endif

  return value;
}

void ForgetV8ObjectForNPObject(NPObject* object) {
  // Use v8::Locker to allow reentrancy.  This function can be called
  // recursively.
  v8::Locker locker;
  if (static_npobject_map.contains(object)) {
    v8::HandleScope scope;
    v8::Persistent<v8::Object> handle(static_npobject_map.get(object));
    handle->SetInternalField(0, V8Proxy::WrapCPointer(NULL));
    static_npobject_map.forget(object);
    V8_NPN_ReleaseObject(object);
#ifdef USE_MEMORY_HINTS
    v8::V8::AdjustAmountOfExternalAllocatedMemory(-kGearsObjectSize);
#endif
  }
}
