// Copyright 2007 Google Inc. All Rights Reserved.

#include "config.h"

#define max max
#define min min
#include "v8_helpers.h"
#include "v8_proxy.h"
#include "v8_index.h"
#include "np_v8object.h"

#include "DOMWindow.h"

void WrapNPObject(v8::Handle<v8::Object> obj, NPObject* npobj) {
  ASSERT(obj->InternalFieldCount() >= 3);

  WebCore::V8Proxy::SetDOMWrapper(obj, WebCore::V8ClassIndex::NPOBJECT, npobj);

  // Create a JS object as a hash map for functions
  obj->SetInternalField(2, v8::Object::New());
}

v8::Local<v8::Context> GetV8Context(NPP npp, NPObject* npobj) {
  V8NPObject* object = reinterpret_cast<V8NPObject*>(npobj);
  return WebCore::V8Proxy::GetContext(object->root_object->frame());
}

WebCore::V8Proxy* GetV8Proxy(NPObject* npobj) {
  V8NPObject* object = reinterpret_cast<V8NPObject*>(npobj);
  WebCore::Frame* frame = object->root_object->frame();
  return WebCore::V8Proxy::retrieve(frame);
}
