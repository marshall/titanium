// Copyright 2007 Google Inc. All Rights Reserved.

#ifndef NP_V8OBJECT_H__
#define NP_V8OBJECT_H__

#include "bindings/npruntime.h"
#include <v8.h>

namespace WebCore {
    class DOMWindow;
}

extern NPClass* NPScriptObjectClass;

// A V8NPObject is a NPObject which carries additional V8-specific
// information.  It is allocated and deallocated by AllocV8NPObject()
// and FreeV8NPObject() methods.
struct V8NPObject {
    NPObject object;
    v8::Persistent<v8::Object> v8_object;
    WebCore::DOMWindow* root_object;
};

struct PrivateIdentifier {
    union {
        const NPUTF8* string;
        int32_t number;
    } value;
    bool isString;
};

NPObject* NPN_CreateScriptObject(NPP npp, v8::Handle<v8::Object>,
                                 WebCore::DOMWindow*);
NPObject* NPN_CreateNoScriptObject(void);

#endif  // NP_V8OBJECT_H__
