// Copyright 2007 Google Inc. All Rights Reserved.

// Forked with few modifications from:
// https://svn/chrome/trunk/webkit/port/bindings/v8/v8_np_utils.h@16743

#ifndef GEARS_V8_BINDINGS_V8_NP_UTILS_H__
#define GEARS_V8_BINDINGS_V8_NP_UTILS_H__

#include "gears/base/common/common.h"
#include "third_party/v8/public/v8.h"
#include "v8_npruntime.h"

// Convert a V8 Value of any type (string, bool, object, etc) to a NPVariant.
void ConvertV8ObjectToNPVariant(v8::Local<v8::Value> object, NPObject *owner,
                                NPVariant* result);

// Convert a NPVariant (string, bool, object, etc) back to a V8 Value. The owner
// object is the NPObject which relates to the object, if the variant to be
// converted is an NPObject.  In that case, the created NPObject will be tied to
// the lifetime of the owner.  If owner is NULL, then the new NPObject is
// treated as a "root" object that must be manually Released by the caller.
// This should only happen for global objects.
v8::Handle<v8::Value> ConvertNPVariantToV8Object(const NPVariant* value,
                                                 NPObject* owner);

// Helper function to create an NPN String Identifier from a v8 string.
NPIdentifier GetStringIdentifier(v8::Handle<v8::String> str);

#endif  // V8_NP_UTILS_H__
