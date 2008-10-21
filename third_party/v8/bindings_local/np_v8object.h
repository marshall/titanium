// Copyright 2007 Google Inc. All Rights Reserved.

// Forked with few modifications from:
// https://svn/chrome/trunk/webkit/port/bindings/v8/np_v8object.h@16743

#ifndef GEARS_V8_BINDINGS_NP_V8OBJECT_H__
#define GEARS_V8_BINDINGS_NP_V8OBJECT_H__

#include "gears/base/common/common.h"
#include "third_party/v8/public/v8.h"
#include "v8_npruntime.h"

extern NPClass* NPScriptObjectClass;

// A V8NPObject is a NPObject which carries additional V8-specific
// information.  It is allocated and deallocated by AllocV8NPObject()
// and FreeV8NPObject() methods.
struct V8NPObject {
    NPObject object;
    v8::Persistent<v8::Object> v8_object;
};

struct PrivateIdentifier {
    union {
        const NPUTF8* string;
        int32_t number;
    } value;
    bool isString;
};

NPObject *V8_NPN_GetGlobalObject(NPP npp);
NPObject* V8_NPN_CreateScriptObject(NPP npp, v8::Handle<v8::Object>);

#endif  // NP_V8OBJECT_H__
