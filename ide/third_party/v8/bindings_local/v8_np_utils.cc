// Copyright 2007 Google Inc. All Rights Reserved.

// Forked with few modifications from:
// https://svn/chrome/trunk/webkit/port/bindings/v8/v8_np_utils.cpp@16743

#include "v8_np_utils.h"

#include "gears/base/common/string16.h"
#include "gears/base/npapi/np_utils.h"
#include "np_v8object.h"
#include "v8_npobject.h"
#include "v8_helpers.h"

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
    NPString np_string =
        NPN_StringDup(reinterpret_cast<char16*>(buf), str->Length());
    NPSTRING_TO_NPVARIANT(np_string, *result);
    delete[] buf;

  } else if (object->IsObject()) {
    NPObject* npobject =
        V8_NPN_CreateScriptObject(0, v8::Handle<v8::Object>::Cast(object));
    if (npobject) {
      _NPN_RegisterObject(npobject, owner);
    }
    OBJECT_TO_NPVARIANT(npobject, *result);
  }
}


v8::Handle<v8::Value> ConvertNPVariantToV8Object(const NPVariant* variant,
                                                 NPObject* owner) {
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
    return CreateV8ObjectForNPObject(obj, owner);
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
