// Copyright 2007 Google Inc. All Rights Reserved.

#include "config.h"

#include "v8_np_utils.h"

#include "base/string_util.h"
#include "DOMWindow.h"
#include "Frame.h"
#include "npruntime_priv.h"
#include "np_v8object.h"
#include "v8_npobject.h"
#include "v8_proxy.h"

void ConvertV8ObjectToNPVariant(v8::Local<v8::Value> object, NPObject *owner,
                                NPVariant* result) {
  VOID_TO_NPVARIANT(*result);

  // It is really the caller's responsibility to deal with the empty handle
  // case because there could be different actions to take in different
  // contexts.
  ASSERT(!object.IsEmpty());

  if (object.IsEmpty()) return;

  if (object->IsNumber()) {
    DOUBLE_TO_NPVARIANT(object->NumberValue(), *result);

  } else if (object->IsBoolean()) {
    BOOLEAN_TO_NPVARIANT(object->BooleanValue(), *result);

  } else if (object->IsNull()) {
    NULL_TO_NPVARIANT(*result);

  } else if (object->IsUndefined()) {
    VOID_TO_NPVARIANT(*result);

  } else if (object->IsString()) {
    v8::Handle<v8::String> str = object->ToString();
    uint16_t *buf = new uint16_t[str->Length()+1];
    str->Write(buf);
    std::string utf8 = WideToUTF8(reinterpret_cast<wchar_t*>(buf));
    char* utf8_chars = strdup(utf8.c_str());
    STRINGN_TO_NPVARIANT(utf8_chars, utf8.length(), *result);
    delete[] buf;

  } else if (object->IsObject()) {
    WebCore::DOMWindow* window = WebCore::V8Proxy::retrieveWindow();
    NPObject* npobject = NPN_CreateScriptObject(
        0, v8::Handle<v8::Object>::Cast(object), window);
    if (npobject) {
      _NPN_RegisterObject(npobject, owner);
    }
    OBJECT_TO_NPVARIANT(npobject, *result);
  }
}


v8::Handle<v8::Value> ConvertNPVariantToV8Object(const NPVariant* variant,
                                                 NPObject* npobject) {
  NPVariantType type = variant->type;

  if (type == NPVariantType_Int32) {
    return v8::Integer::New(NPVARIANT_TO_INT32(*variant));
  }
  if (type == NPVariantType_Double) {
    return v8::Number::New(NPVARIANT_TO_DOUBLE(*variant));
  }
  if (type == NPVariantType_Bool) {
    return NPVARIANT_TO_BOOLEAN(*variant) ? v8::True() : v8::False();
  }
  if (type == NPVariantType_Null) {
    return v8::Null();
  }
  if (type == NPVariantType_Void) {
    return v8::Undefined();
  }
  if (type == NPVariantType_String) {
    NPString src = NPVARIANT_TO_STRING(*variant);
    return v8::String::New(src.UTF8Characters, src.UTF8Length);
  }
  if (type == NPVariantType_Object) {
    NPObject* obj = NPVARIANT_TO_OBJECT(*variant);
    if (obj->_class == NPScriptObjectClass) {
      return reinterpret_cast<V8NPObject*>(obj)->v8_object;
    }
    return CreateV8ObjectForNPObject(obj, npobject);
  }
  return v8::Undefined();
}

// Helper function to create an NPN String Identifier from a v8 string.
NPIdentifier GetStringIdentifier(v8::Handle<v8::String> str) {
  char *buf = new char[str->Length() + 1];
  str->WriteAscii(buf);
  NPIdentifier ident = NPN_GetStringIdentifier(buf);
  delete[] buf;
  return ident;
}
