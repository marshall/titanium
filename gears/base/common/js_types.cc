// Copyright 2007, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <math.h>

#include "gears/base/common/js_types.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/basictypes.h"  // for isnan
#include "gears/base/common/js_dom_element.h"
#include "gears/base/common/js_runner.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#if BROWSER_FF
#include "gears/base/common/js_runner_ff_marshaling.h"
#elif BROWSER_IE
#include <dispex.h>
#include "gears/base/ie/activex_utils.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/base/ie/module_wrapper.h"
#include "genfiles/interfaces.h"
#elif BROWSER_NPAPI
#include "gears/base/npapi/browser_utils.h"
#include "gears/base/npapi/module_wrapper.h"
#include "gears/base/npapi/np_utils.h"
#include "gears/base/npapi/scoped_npapi_handles.h"
#endif
#if BROWSER_WEBKIT
#include "gears/base/safari/npapi_patches.h"
#endif

#ifdef WIN32
// For some reason Win32 thinks snprintf() needs to be marked as non-standard.
#define snprintf _snprintf
#endif

static inline bool DoubleIsInt64(double val) {
  return val >= kint64min && val <= kint64max && floor(val) == val;
}

static inline bool DoubleIsInt32(double val) {
  return val >= kint32min && val <= kint32max && floor(val) == val;
}

//------------------------------------------------------------------------------
// JsToken and friends
//------------------------------------------------------------------------------

#if BROWSER_NPAPI

JsTokenEqualTo::JsTokenEqualTo(JsRunnerInterface *js_runner)
  : js_runner_(js_runner), compare_func_(NULL) {
  // NPAPI doesn't guarantee that a Javascript object will have only one
  // NPObject representation, so we need to let the scripting engine compare
  // objects.
  const char kCompareFunc[] = "(function (a, b) { return a === b; })";

  NPObject *global;
  if (NPN_GetValue(js_runner->GetContext(), NPNVWindowNPObject,
                   &global) != NPERR_NO_ERROR)
    return;
  ScopedNPObject global_scoped(global);
  NPString np_script = { kCompareFunc, ARRAYSIZE(kCompareFunc) - 1};
  if (!NPN_Evaluate(js_runner->GetContext(), global, &np_script,
                    &compare_func_) ||
      !NPVARIANT_IS_OBJECT(compare_func_)) {
    assert(false);
  }
}

JsTokenEqualTo::~JsTokenEqualTo() {
}

JsTokenEqualTo& JsTokenEqualTo::operator=(const JsTokenEqualTo &that) {
  js_runner_ = that.js_runner_;
  compare_func_ = that.compare_func_;
  return *this;
}

bool JsTokenEqualTo::CompareObjects(const JsToken &x, const JsToken &y) const {
  assert(NPVARIANT_IS_OBJECT(compare_func_));

  NPObject *compare_func_obj = NPVARIANT_TO_OBJECT(compare_func_);
  NPVariant args[] = {x, y};

  // Invoke the method.
  ScopedNPVariant result;
  bool rv = NPN_InvokeDefault(js_runner_->GetContext(), compare_func_obj,
                              args, 2, &result);
  if (!rv) { return false; }

  if (!NPVARIANT_IS_BOOLEAN(result)) {
    assert(false);
    return false;
  }

  return result.value.boolValue;
}

#endif

//------------------------------------------------------------------------------
// JsArray
//------------------------------------------------------------------------------

#if BROWSER_FF

JsArray::JsArray() : js_context_(NULL), array_(0) {
  LEAK_COUNTER_INCREMENT(JsArray);
}

JsArray::~JsArray() {
  LEAK_COUNTER_DECREMENT(JsArray);
  if (array_ && JSVAL_IS_GCTHING(array_)) {
    JsRequest request(js_context_);
    JS_RemoveRoot(js_context_, &array_);
  }
}

bool JsArray::SetArray(JsToken value, JsContextPtr context) {
  // check that it's an array
  if (!JSVAL_IS_OBJECT(value) ||
      !JS_IsArrayObject(context, JSVAL_TO_OBJECT(value))) {
    return false;
  }

  if (array_ && JSVAL_IS_GCTHING(array_)) {
    JsRequest request(js_context_);
    JS_RemoveRoot(js_context_, &array_);
  }
  array_ = value;
  js_context_ = context;
  if (JSVAL_IS_GCTHING(array_)) {
    JsRequest request(js_context_);
    JS_AddRoot(js_context_, &array_);
  }
  return true;
}

bool JsArray::GetLength(int *length) const {
  assert(JsTokenIsArray(array_, js_context_));

  JsRequest request(js_context_);
  jsuint array_length;
  if (JS_GetArrayLength(js_context_, JSVAL_TO_OBJECT(array_), &array_length) !=
      JS_TRUE) {
    return false;
  }
  *length = static_cast<int>(array_length);
  return true;
}

bool JsArray::GetElement(int index, JsScopedToken *out) const {
  assert(JsTokenIsArray(array_, js_context_));

  // JS_GetElement sets the token to JS_VOID if we request a non-existent index.
  // This is the correct jsval for JSPARAM_UNDEFINED.
  JsRequest request(js_context_);
  return JS_GetElement(js_context_, JSVAL_TO_OBJECT(array_),
                       index, out) == JS_TRUE;
}

bool JsArray::SetElement(int index, const JsToken &value) {
  assert(JsTokenIsArray(array_, js_context_));

  JsRequest request(js_context_);
  return JS_DefineElement(js_context_,
                          JSVAL_TO_OBJECT(array_),
                          index, value,
                          nsnull, nsnull, // getter, setter
                          JSPROP_ENUMERATE) == JS_TRUE;
}

bool JsArray::SetElementBool(int index, bool value) {
  return SetElement(index, BOOLEAN_TO_JSVAL(value));
}

bool JsArray::SetElementInt(int index, int value) {
  return SetElement(index, INT_TO_JSVAL(value));
}

bool JsArray::SetElementDouble(int index, double value) {
  JsToken jsval;
  if (DoubleToJsToken(js_context_, value, &jsval)) {
    return SetElement(index, jsval);
  } else {
    return false;
  }
}

bool JsArray::SetElementString(int index, const std::string16 &value) {
  JsToken token;
  if (StringToJsToken(js_context_, value.c_str(), &token)) {
    return SetElement(index, token);
  } else {
    return false;
  }
}

#elif BROWSER_IE

JsArray::JsArray() : js_context_(NULL) {
}

JsArray::~JsArray() {
}

bool JsArray::SetArray(JsToken value, JsContextPtr context) {
  if (value.vt != VT_DISPATCH) return false;

  array_ = value;

  return true;
}

bool JsArray::GetLength(int *length) const {
  assert(JsTokenIsArray(array_, js_context_));

  CComVariant out;
  if (FAILED(ActiveXUtils::GetDispatchProperty(array_.pdispVal,
                                               STRING16(L"length"),
                                               &out))) {
    return false;
  }
  if (out.vt != VT_I4) return false;

  *length = out.lVal;

  return true;
}

bool JsArray::GetElement(int index, JsScopedToken *out) const {
  assert(JsTokenIsArray(array_, js_context_));

  std::string16 name = IntegerToString16(index);

  HRESULT hr = ActiveXUtils::GetDispatchProperty(array_.pdispVal,
                                                 name.c_str(),
                                                 out);
  if (SUCCEEDED(hr)) {
    return true;
  } else if (hr == DISP_E_UNKNOWNNAME) {
    // There's no value at this index, so this element is undefined.
    out->Clear();
    return true;
  }
  return false;
}

bool JsArray::SetElement(int index, const JsScopedToken &in) {
  assert(JsTokenIsArray(array_, js_context_));

  std::string16 name(IntegerToString16(index));
  HRESULT hr = ActiveXUtils::AddAndSetDispatchProperty(
    array_.pdispVal, name.c_str(), &in);
  return SUCCEEDED(hr);
}

bool JsArray::SetElementBool(int index, bool value) {
  return SetElement(index, CComVariant(value));
}

bool JsArray::SetElementInt(int index, int value) {
  return SetElement(index, CComVariant(value));
}

bool JsArray::SetElementDouble(int index, double value) {
  return SetElement(index, CComVariant(value));
}

bool JsArray::SetElementString(int index, const std::string16 &value) {
  return SetElement(index, CComVariant(value.c_str()));
}

#elif BROWSER_NPAPI

JsArray::JsArray() : js_context_(NULL) {
  VOID_TO_NPVARIANT(array_);
}

JsArray::~JsArray() {
}

bool JsArray::SetArray(JsToken value, JsContextPtr context) {
  if (!JsTokenIsArray(value, context)) return false;

  array_ = value;
  js_context_ = context;
  return true;
}

bool JsArray::GetLength(int *length) const {
  assert(JsTokenIsArray(array_, js_context_));

  NPObject *array = NPVARIANT_TO_OBJECT(array_);

  NPIdentifier length_id = NPN_GetStringIdentifier("length");

  NPVariant np_length;
  if (!NPN_GetProperty(js_context_, array, length_id, &np_length)) return false;

  return JsTokenToInt_NoCoerce(np_length, js_context_, length);
}

bool JsArray::GetElement(int index, JsScopedToken *out) const {
  assert(JsTokenIsArray(array_, js_context_));

  NPObject *array = NPVARIANT_TO_OBJECT(array_);

  NPIdentifier index_id = NPN_GetIntIdentifier(index);

  if (!NPN_GetProperty(js_context_, array, index_id, out)) return false;

  return true;
}

bool JsArray::SetElement(int index, const JsScopedToken &in) {
  assert(JsTokenIsArray(array_, js_context_));

  NPObject *array = NPVARIANT_TO_OBJECT(array_);
  NPIdentifier index_id = NPN_GetIntIdentifier(index);
  return NPN_SetProperty(js_context_, array, index_id, &in);
}

bool JsArray::SetElementBool(int index, bool value) {
  return SetElement(index, JsScopedToken(value));
}

bool JsArray::SetElementInt(int index, int value) {
  return SetElement(index, JsScopedToken(value));
}

bool JsArray::SetElementDouble(int index, double value) {
  return SetElement(index, JsScopedToken(value));
}

bool JsArray::SetElementString(int index, const std::string16 &value) {
  return SetElement(index, JsScopedToken(value.c_str()));
}

#endif

bool JsArray::GetElementAsBool(int index, bool *out) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return false;

  return JsTokenToBool_NoCoerce(token, js_context_, out);
}

bool JsArray::GetElementAsInt(int index, int *out) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return false;

  return JsTokenToInt_NoCoerce(token, js_context_, out);
}

bool JsArray::GetElementAsDouble(int index, double *out) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return false;

  return JsTokenToDouble_NoCoerce(token, js_context_, out);
}

bool JsArray::GetElementAsString(int index, std::string16 *out) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return false;

  return JsTokenToString_NoCoerce(token, js_context_, out);
}

bool JsArray::GetElementAsArray(int index, JsArray *out) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return false;

  return out->SetArray(token, js_context_);
}

bool JsArray::GetElementAsObject(int index, JsObject *out) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return false;

  return out->SetObject(token, js_context_);
}

bool JsArray::GetElementAsFunction(int index, JsRootedCallback **out) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return false;

  return JsTokenToNewCallback_NoCoerce(token, js_context_, out);
}

JsParamType JsArray::GetElementType(int index) const {
  JsScopedToken token;
  if (!GetElement(index, &token)) return JSPARAM_UNKNOWN;

  return JsTokenGetType(token, js_context_);
}

bool JsArray::SetElementArray(int index, JsArray *value) {
  return SetElement(index, value->array_);
}

bool JsArray::SetElementObject(int index, JsObject *value) {
  return SetElement(index, value->token());
}

bool JsArray::SetElementFunction(int index, JsRootedCallback *value) {
  return SetElement(index, value->token());
}

bool JsArray::SetElementModule(int index, ModuleImplBaseClass *value) {
  return SetElement(index, value->GetWrapperToken());
}

//------------------------------------------------------------------------------
// JsObject
//------------------------------------------------------------------------------

#if BROWSER_FF

JsObject::JsObject() : js_context_(NULL), js_object_(0) {
  LEAK_COUNTER_INCREMENT(JsObject);
}

JsObject::~JsObject() {
  LEAK_COUNTER_DECREMENT(JsObject);
  if (js_object_ && JSVAL_IS_GCTHING(js_object_)) {
    JsRequest request(js_context_);
    JS_RemoveRoot(js_context_, &js_object_);
  }
}

bool JsObject::SetObject(JsToken value, JsContextPtr context) {
  if (JSVAL_IS_OBJECT(value)) {
    if (js_object_ && JSVAL_IS_GCTHING(js_object_))
      JS_RemoveRoot(js_context_, &js_object_);
    js_object_ = value;
    js_context_ = context;
    if (JSVAL_IS_GCTHING(js_object_)) {
      JsRequest request(js_context_);
      JS_AddRoot(js_context_, &js_object_);
    }
    return true;
  }

  return false;
}

bool JsObject::GetPropertyNames(std::vector<std::string16> *out) const {
  assert(JsTokenIsObject(js_object_));

  JsRequest request(js_context_);
  JSIdArray *ids = JS_Enumerate(js_context_, JSVAL_TO_OBJECT(js_object_));
  for (int i = 0; i < ids->length; i++) {
    jsval property_key;
    if (!JS_IdToValue(js_context_, ids->vector[i], &property_key)) {
      // JS_IdToValue is implemented as a typecast, and should never fail.
      assert(false);
    }
    std::string16 property_name;
    if (!JsTokenToString_Coerce(property_key, js_context_, &property_name)) {
      continue;
    }
    out->push_back(property_name);
  }
  JS_DestroyIdArray(js_context_, ids);
  return true;
}

bool JsObject::GetProperty(const std::string16 &name,
                           JsScopedToken *out) const {
  assert(JsTokenIsObject(js_object_));

  JsRequest request(js_context_);
  return JS_GetUCProperty(js_context_, JSVAL_TO_OBJECT(js_object_),
                          reinterpret_cast<const jschar*>(name.c_str()),
                          name.length(), out) == JS_TRUE;
}

bool JsObject::SetProperty(const std::string16 &name, const JsToken &value) {
  assert(JsTokenIsObject(js_object_));

  std::string name_utf8;
  if (!String16ToUTF8(name.c_str(), &name_utf8)) {
    LOG(("Could not convert property name to utf8."));
    return false;
  }

  JsRequest request(js_context_);
  JSBool result = JS_DefineProperty(js_context_,
                                    JSVAL_TO_OBJECT(js_object_),
                                    name_utf8.c_str(), value,
                                    nsnull, nsnull, // getter, setter
                                    JSPROP_ENUMERATE);
  if (!result) {
    LOG(("Could not define property."));
    return false;
  }

  return true;
}

bool JsObject::SetPropertyBool(const std::string16& name, bool value) {
  return SetProperty(name, BOOLEAN_TO_JSVAL(value));
}

bool JsObject::SetPropertyInt(const std::string16 &name, int value) {
  return SetProperty(name, INT_TO_JSVAL(value));
}

bool JsObject::SetPropertyDouble(const std::string16& name, double value) {
  JsToken jsval;
  if (DoubleToJsToken(js_context_, value, &jsval)) {
    return SetProperty(name, jsval);
  } else {
    return false;
  }
}

bool JsObject::SetPropertyString(const std::string16 &name,
                                 const std::string16 &value) {
  JsToken token;
  if (StringToJsToken(js_context_, value.c_str(), &token)) {
    return SetProperty(name, token);
  } else {
    return false;
  }
}

#elif BROWSER_IE

JsObject::JsObject() : js_context_(NULL) {
}

JsObject::~JsObject() {
}

bool JsObject::SetObject(JsToken value, JsContextPtr context) {
  if (value.vt != VT_DISPATCH) return false;

  js_object_ = value;

  return true;
}

bool JsObject::GetPropertyNames(std::vector<std::string16> *out) const {
  assert(JsTokenIsObject(js_object_));

  return SUCCEEDED(
      ActiveXUtils::GetDispatchPropertyNames(js_object_.pdispVal, out));
}

bool JsObject::GetProperty(const std::string16 &name,
                           JsScopedToken *out) const {
  assert(JsTokenIsObject(js_object_));

  // If the property name is unknown, GetDispatchProperty will return
  // DISP_E_UNKNOWNNAME and out will be unchanged.
  HRESULT hr = ActiveXUtils::GetDispatchProperty(js_object_.pdispVal,
                                                 name.c_str(),
                                                 out);
  if (DISP_E_UNKNOWNNAME == hr) {
    // Set the token to the equivalent of JSPARAM_UNDEFINED.
    out->Clear();
    return true;
  }
  return SUCCEEDED(hr);
}

bool JsObject::SetProperty(const std::string16 &name, const JsToken &value) {
  assert(JsTokenIsObject(js_object_));

  HRESULT hr = ActiveXUtils::AddAndSetDispatchProperty(
    js_object_.pdispVal, name.c_str(), &value);
  return SUCCEEDED(hr);
}

bool JsObject::SetPropertyBool(const std::string16& name, bool value) {
  return SetProperty(name, CComVariant(value));
}

bool JsObject::SetPropertyInt(const std::string16 &name, int value) {
  return SetProperty(name, CComVariant(value));
}

bool JsObject::SetPropertyDouble(const std::string16& name, double value) {
  return SetProperty(name, CComVariant(value));
}

bool JsObject::SetPropertyString(const std::string16 &name,
                                 const std::string16 &value) {
  return SetProperty(name, CComVariant(value.c_str()));
}

#elif BROWSER_NPAPI

JsObject::JsObject() : js_context_(NULL) {
  VOID_TO_NPVARIANT(js_object_);
}

JsObject::~JsObject() {
}

bool JsObject::SetObject(JsToken value, JsContextPtr context) {
  if (NPVARIANT_IS_OBJECT(value)) {
    js_object_ = value;
    js_context_ = context;
    return true;
  }

  return false;
}

bool JsObject::GetPropertyNames(std::vector<std::string16> *out) const {
  assert(JsTokenIsObject(js_object_));

  NPIdentifier *identifiers;
  uint32 count;

  NPObject *object = NPVARIANT_TO_OBJECT(js_object_);
  if (!NPN_Enumerate(js_context_, object, &identifiers, (uint32_t *)&count)) {
    return false;
  }

  for (unsigned int i = 0; i < count; ++i) {
    if (!NPN_IdentifierIsString(identifiers[i])) {
      out->push_back(IntegerToString16(NPN_IntFromIdentifier(identifiers[i])));
      continue;
    }
    NPUTF8 *utf8_name = NPN_UTF8FromIdentifier(identifiers[i]);
    std::string16 name;

    bool success = UTF8ToString16(utf8_name, &name);

    NPN_MemFree(utf8_name);

    if (!success) {
      return false;
    }
    out->push_back(name);
  }
  return true;
}

bool JsObject::GetProperty(const std::string16 &name,
                           JsScopedToken *out) const {
  assert(JsTokenIsObject(js_object_));

  std::string name_utf8;
  if (!String16ToUTF8(name.c_str(), &name_utf8)) return false;

  NPObject *object = NPVARIANT_TO_OBJECT(js_object_);
  NPIdentifier name_id = NPN_GetStringIdentifier(name_utf8.c_str());

  return NPN_GetProperty(js_context_, object, name_id, out);
}

bool JsObject::SetProperty(const std::string16 &name, const JsToken &value) {
  assert(JsTokenIsObject(js_object_));

  std::string name_utf8;
  if (!String16ToUTF8(name.c_str(), &name_utf8)) { return false; }

  NPObject *np_object = NPVARIANT_TO_OBJECT(js_object_);
  NPIdentifier np_name = NPN_GetStringIdentifier(name_utf8.c_str());
  return NPN_SetProperty(js_context_, np_object, np_name, &value);
}

bool JsObject::SetPropertyString(const std::string16 &name,
                                 const std::string16 &value) {
  return SetProperty(name, ScopedNPVariant(value.c_str()));
}

bool JsObject::SetPropertyInt(const std::string16 &name, int value) {
  return SetProperty(name, ScopedNPVariant(value));
}

bool JsObject::SetPropertyBool(const std::string16 &name, bool value) {
  return SetProperty(name, ScopedNPVariant(value));
}

bool JsObject::SetPropertyDouble(const std::string16 &name, double value) {
  return SetProperty(name, ScopedNPVariant(value));
}

#endif

bool JsObject::GetPropertyAsBool(const std::string16 &name, bool *out) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return false;

  return JsTokenToBool_NoCoerce(token, js_context_, out);
}

bool JsObject::GetPropertyAsInt(const std::string16 &name, int *out) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return false;

  return JsTokenToInt_NoCoerce(token, js_context_, out);
}

bool JsObject::GetPropertyAsDouble(const std::string16 &name,
                                   double *out) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return false;

  return JsTokenToDouble_NoCoerce(token, js_context_, out);
}

bool JsObject::GetPropertyAsString(const std::string16 &name,
                                   std::string16 *out) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return false;

  return JsTokenToString_NoCoerce(token, js_context_, out);
}

bool JsObject::GetPropertyAsArray(const std::string16 &name,
                                  JsArray *out) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return false;

  return out->SetArray(token, js_context_);
}

bool JsObject::GetPropertyAsObject(const std::string16 &name,
                                   JsObject *out) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return false;

  return out->SetObject(token, js_context_);
}

bool JsObject::GetPropertyAsFunction(const std::string16 &name,
                                     JsRootedCallback **out) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return false;

  return JsTokenToNewCallback_NoCoerce(token, js_context_, out);
}

JsParamType JsObject::GetPropertyType(const std::string16 &name) const {
  JsScopedToken token;
  if (!GetProperty(name, &token)) return JSPARAM_UNKNOWN;

  return JsTokenGetType(token, js_context_);
}

bool JsObject::SetPropertyArray(const std::string16& name, JsArray* value) {
  return SetProperty(name, value->token());
}

bool JsObject::SetPropertyObject(const std::string16& name, JsObject* value) {
  return SetProperty(name, value->token());
}

bool JsObject::SetPropertyFunction(const std::string16& name,
                                   JsRootedCallback* value) {
  return SetProperty(name, value->token());
}

bool JsObject::SetPropertyModule(const std::string16 &name,
                                 ModuleImplBaseClass *value) {
  return SetProperty(name, value->GetWrapperToken());
}

//------------------------------------------------------------------------------
// JsTokenToXxx, XxxToJsToken
//------------------------------------------------------------------------------

// Given a JsToken, extract it into a JsArgument.  Object pointers are weak
// references (ref count is not increased).
static bool ConvertTokenToArgument(JsCallContext *context,
                                   const JsToken &variant,
                                   JsArgument *param) {
  switch (param->type) {
    case JSPARAM_BOOL: {
      bool *value = static_cast<bool *>(param->value_ptr);
      if (!JsTokenToBool_NoCoerce(variant, context->js_context(), value)) {
        return false;
      }
      break;
    }
    case JSPARAM_INT: {
      int *value = static_cast<int *>(param->value_ptr);
      if (!JsTokenToInt_NoCoerce(variant, context->js_context(), value)) {
        return false;
      }
      break;
    }
    case JSPARAM_INT64: {
      int64 *value = static_cast<int64 *>(param->value_ptr);
      if (!JsTokenToInt64_NoCoerce(variant, context->js_context(), value)) {
        return false;
      }
      break;
    }
    case JSPARAM_DOUBLE: {
      double *value = static_cast<double *>(param->value_ptr);
      if (!JsTokenToDouble_NoCoerce(variant, context->js_context(), value)) {
        return false;
      }
      break;
    }
    case JSPARAM_OBJECT: {
      JsObject *value = static_cast<JsObject *>(param->value_ptr);
      if (!value->SetObject(variant, context->js_context())) {
        return false;
      }
      break;
    }
    case JSPARAM_ARRAY: {
      JsArray *value = static_cast<JsArray *>(param->value_ptr);
      if (!value->SetArray(variant, context->js_context())) {
        return false;
      }
      break;
    }
    case JSPARAM_FUNCTION: {
      // TODO(nigeltao): make this a JsRootedCallback* instead of a pointer-
      // to-a-pointer JsRootedCallback**, and add a JsRootedCallback::SetToken
      // method just like JsObject::SetObject and JsArray::SetArray is called
      // above, in order to simplify memory management issues around using
      // JSPARAM_FUNCTION and JsCallContext::GetArguments, especially if
      // GetArguments partially succeeds (on the first few arguments) but
      // ultimately fails.
      JsRootedCallback **value =
          static_cast<JsRootedCallback **>(param->value_ptr);
      if (!JsTokenToNewCallback_NoCoerce(variant, context->js_context(),
                                         value)) {
        return false;
      }
      break;
    }
    case JSPARAM_STRING16: {
      std::string16 *value = static_cast<std::string16 *>(param->value_ptr);
      if (!JsTokenToString_NoCoerce(variant, context->js_context(), value)) {
        return false;
      }
      break;
    }
    case JSPARAM_TOKEN: {
      JsToken *value = static_cast<JsToken *>(param->value_ptr);
      *value = variant;
      return true;
    }
    case JSPARAM_MODULE: {
      ModuleImplBaseClass **value =
          static_cast<ModuleImplBaseClass **>(param->value_ptr);
      if (!JsTokenToModule(
#if BROWSER_FF
          context->js_runner(),
#else
          NULL,
#endif
          context->js_context(), variant, value)) {
        context->SetException(
            STRING16(L"Invalid argument type: expected module."));
        return false;
      }
      break;
    }
    case JSPARAM_DOM_ELEMENT: {
      JsDomElement *value = static_cast<JsDomElement *>(param->value_ptr);
      if (!value->InitJsDomElement(context->js_context(), variant)) {
        context->SetException(
            STRING16(L"Invalid argument type: expected DOM element."));
        return false;
      }
      break;
    }
    default:
      assert(false);
      return false;
  }

  return true;
}

bool JsTokenToNewCallback_NoCoerce(JsToken t, JsContextPtr cx,
                                   JsRootedCallback **out) {
  if (!JsTokenIsCallback(t, cx)) { return false; }
  *out = new JsRootedCallback(cx, t);
  return true;
}

bool JsTokenToInt64_NoCoerce(JsToken t, JsContextPtr cx, int64 *out) {
  double dbl;
  if (!JsTokenToDouble_NoCoerce(t, cx, &dbl)) { return false; }
  if (dbl < JS_INT_MIN || dbl > JS_INT_MAX) { return false; }
  if (!DoubleIsInt64(dbl)) { return false; }
  *out = static_cast<int64>(dbl);
  return true;
}


#if BROWSER_FF

bool JsTokenToBool_NoCoerce(JsToken t, JsContextPtr cx, bool *out) {
  if (!JSVAL_IS_BOOLEAN(t)) { return false; }
  *out = (JSVAL_TO_BOOLEAN(t) == JS_TRUE);
  return true;
}

bool JsTokenToDouble_NoCoerce(JsToken t, JsContextPtr cx, double *out) {
  if (JSVAL_IS_DOUBLE(t)) {
    *out = *(JSVAL_TO_DOUBLE(t));
    return true;
  } else if (JSVAL_IS_INT(t)) {
    *out = JSVAL_TO_INT(t);
    return true;
  }

  return false;
}

bool JsTokenToInt_NoCoerce(JsToken t, JsContextPtr cx, int *out) {
  if (!JSVAL_IS_INT(t)) { return false; }
  *out = JSVAL_TO_INT(t);
  return true;
}

bool JsTokenToString_NoCoerce(JsToken t, JsContextPtr cx, std::string16 *out) {
  if (!JSVAL_IS_STRING(t)) { return false; }
  JSString *js_str = JSVAL_TO_STRING(t);
  out->assign(reinterpret_cast<const char16 *>(JS_GetStringChars(js_str)),
              JS_GetStringLength(js_str));
  return true;
}


bool JsTokenToBool_Coerce(JsToken t, JsContextPtr cx, bool *out) {
  JSBool bool_value;
  JsRequest request(cx);
  if (!JS_ValueToBoolean(cx, t, &bool_value)) { return false; }
  *out = (bool_value == JS_TRUE);
  return true;
}

bool JsTokenToInt_Coerce(JsToken t, JsContextPtr cx, int *out) {
  // JS_ValueToECMAInt32 has edge-cases with non-numeric types, for example
  // non-numeric strings, NaN and {} are coerced to 0 rather than returning
  // failure. Compare this behaviour to JS_ValueToNumber (in
  // JsTokenToDouble_Coerce) which returns failure for these cases. It seems
  // best to be consistent in our handling of numbers. Coercing non-numbers to 0
  // could make Gears applications harder to debug, e.g. a user passes a non-
  // numeric type into a Gears API call that expects an integer and rather than
  // being notified of the problem (as they would be for calls that expect a
  // double) Gears instead treats it as 0 and carries on its way.
  // Because of this we use JS_ValueToNumber to coerce to a double and cast
  // to an int, which bypasses these problems. To avoid code duplication,
  // we just call JsTokenToDouble_Coerce and cast the result.
  // Note also that in IE we have to do a double coercion and an int cast too
  // due to the way VariantChangeType() handles integer coercions (rounding
  // instead of truncating).
  double output;
  if (!JsTokenToDouble_Coerce(t, cx, &output)) { return false; }
  // Make sure the double will fit into an int
  if (output < kint32min || output > kint32max) { return false; }
  *out = static_cast<int>(output);
  return true;
}

bool JsTokenToDouble_Coerce(JsToken t, JsContextPtr cx, double *out) {
  jsdouble dbl_value;
  JsRequest request(cx);
  if (!JS_ValueToNumber(cx, t, &dbl_value)) { return false; }
  // Edge-case: NaN should return failure
  if (isnan(dbl_value)) { return false; }
  *out = dbl_value;
  return true;
}

bool JsTokenToString_Coerce(JsToken t, JsContextPtr cx, std::string16 *out) {
  JsRequest request(cx);
  JSString *js_str = JS_ValueToString(cx, t);
  if (!js_str) { return false; }
  out->assign(reinterpret_cast<const char16 *>(JS_GetStringChars(js_str)),
              JS_GetStringLength(js_str));
  return true;
}

bool JsTokenToModule(JsRunnerInterface *js_runner,
                     JsContextPtr context,
                     const JsToken in,
                     ModuleImplBaseClass **out) {
  return js_runner->GetModuleFromJsToken(in, out);
}

JsParamType JsTokenGetType(JsToken t, JsContextPtr cx) {
  if (JSVAL_IS_BOOLEAN(t)) {
    return JSPARAM_BOOL;
  } else if (JSVAL_IS_INT(t)) {
    return JSPARAM_INT;
  } else if (JSVAL_IS_DOUBLE(t)) {
    return JSPARAM_DOUBLE;
  } else if (JSVAL_IS_STRING(t)) {
    return JSPARAM_STRING16;
  } else if (JSVAL_IS_NULL(t)) {
    return JSPARAM_NULL;
  } else if (JSVAL_IS_VOID(t)) {
    return JSPARAM_UNDEFINED;
  } else if (JsTokenIsArray(t, cx)) {
    return JSPARAM_ARRAY;
  } else if (JsTokenIsCallback(t, cx)) {
    return JSPARAM_FUNCTION;
  } else if (JsTokenIsObject(t)) {
    return JSPARAM_OBJECT;
  } else {
    return JSPARAM_UNKNOWN;  // Unsupported type
  }
}

bool JsTokenIsCallback(JsToken t, JsContextPtr cx) {
  return JsTokenIsObject(t) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(t));
}

bool JsTokenIsArray(JsToken t, JsContextPtr cx) {
  return JsTokenIsObject(t) && JS_IsArrayObject(cx, JSVAL_TO_OBJECT(t));
}

bool JsTokenIsObject(JsToken t) {
  return JSVAL_IS_OBJECT(t) && !JSVAL_IS_NULL(t); // JSVAL_IS_OBJECT returns
                                                  // true for <null>.
}

bool JsTokenIsNullOrUndefined(JsToken t) {
  return JSVAL_IS_NULL(t) || JSVAL_IS_VOID(t); // null or undefined
}

bool BoolToJsToken(JsContextPtr context, bool value, JsScopedToken *out) {
  *out = BOOLEAN_TO_JSVAL(value);
  return true;
}

bool IntToJsToken(JsContextPtr context, int value, JsScopedToken *out) {
  *out = INT_TO_JSVAL(value);
  return true;
}

// TODO(nigeltao): We are creating an unrooted jsval here (and saving it as
// *out), which is vulnerable to garbage collection.  Strictly speaking,
// callers should immediately root the token to avoid holding a dangling
// pointer.  The TODO is to fix up the API so that the callee does the
// right thing.
// Similarly for FooToJsToken in general, for Foo in {Bool, Int, Double}.
bool StringToJsToken(JsContextPtr context, const char16 *value,
                     JsScopedToken *out) {
  JsRequest request(context);
  JSString *jstr = JS_NewUCStringCopyZ(context,
                                       reinterpret_cast<const jschar *>(value));
  if (jstr) {
    *out = STRING_TO_JSVAL(jstr);
    return true;
  } else {
    return false;
  }
}

bool DoubleToJsToken(JsContextPtr context, double value, JsScopedToken *out) {
  JsRequest request(context);
  jsdouble* dp = JS_NewDouble(context, value);
  if (!dp)
    return false;

  *out = DOUBLE_TO_JSVAL(dp);
  return true;
}

bool NullToJsToken(JsContextPtr context, JsScopedToken *out) {
  *out = JSVAL_NULL;
  return true;
}

bool UndefinedToJsToken(JsContextPtr context, JsScopedToken *out) {
  *out = JSVAL_VOID;
  return true;
}

#elif BROWSER_IE

bool JsTokenToBool_NoCoerce(JsToken t, JsContextPtr cx, bool *out) {
  if (t.vt != VT_BOOL) { return false; }
  *out = (t.boolVal == VARIANT_TRUE);
  return true;
}

bool JsTokenToInt_NoCoerce(JsToken t, JsContextPtr cx, int *out) {
  if (t.vt != VT_I4) { return false; }
  *out = t.lVal;
  return true;
}

bool JsTokenToString_NoCoerce(JsToken t, JsContextPtr cx, std::string16 *out) {
  if (t.vt != VT_BSTR) { return false; }
  // NOTE: BSTR semantics demand that null BSTRs be interpreted as empty
  // string.
  if (!t.bstrVal) {
    out->clear();
  } else {
    out->assign(t.bstrVal);
  }
  return true;
}

bool JsTokenToDouble_NoCoerce(JsToken t, JsContextPtr cx, double *out) {
  if (t.vt == VT_R8) {
    *out = t.dblVal;
    return true;
  } else if (t.vt == VT_I4) {
    *out = static_cast<double>(t.lVal);
    return true;
  }

  return false;
}

bool JsTokenToBool_Coerce(JsToken t, JsContextPtr cx, bool *out) {
  // JsToken is a bool
  if (t.vt == VT_BOOL) {
    *out = (t.boolVal == VARIANT_TRUE);
  // Try to coerce the JsToken to a bool
  // There are many edge-cases with to-boolean coercions in IE using
  // VariantChangeType() (NaN is true, {} is false, "test" is false, etc.)
  // so we may as well do this coercion manually. The only values that should
  // be treated as false in JavaScript are false, null, NaN, 0, undefined and
  // an empty string. false is already covered by the case above.
  } else if (JsTokenIsNullOrUndefined(t) ||       // null, undefined
             t.vt == VT_R8 && isnan(t.dblVal) ||  // NaN
             t.vt == VT_I4 && t.lVal == 0 ||      // 0
             t.vt == VT_BSTR && !t.bstrVal[0]) {  // empty string
    *out = false;
  } else {
    *out = true;  // Anything else is true
  }
  return true;
}

bool JsTokenToInt_Coerce(JsToken t, JsContextPtr cx, int *out) {
  if (t.vt == VT_I4) {
    *out = t.lVal;
  // Try to coerce the JsToken to an int.
  } else {
    // Note that doubles are rounded to ints by VariantChangeType() when they
    // should be truncated, so we must force it to truncate by converting to
    // a double and then casting to an int manually.
    double output;
    if (!JsTokenToDouble_Coerce(t, cx, &output)) { return false; }
    // Make sure the double will fit into an int
    if (output < kint32min || output > kint32max) {
      return false;
    }
    *out = static_cast<int>(output);
  }
  return true;
}

bool JsTokenToDouble_Coerce(JsToken t, JsContextPtr cx, double *out) {
  // This coercion is very tricky to get just-right. See test cases in
  // test/testcases/internal_tests.js before making any changes. The expected
  // outputs are based on the result of doing a Number(testval) in JavaScript.
  // JsToken is a double
  if (t.vt == VT_R8) {
    // Edge-case: NaN should return failure
    if (isnan(t.dblVal)) { return false; }
    *out = t.dblVal;
  // Edge-case: boolean true should coerce to 1, not -1.
  } else if (t.vt == VT_BOOL) {
    *out = (t.boolVal == VARIANT_TRUE) ? 1 : 0;
  // Edge-case: null should coerce to 0
  } else if (t.vt == VT_NULL) {
    *out = 0;
  // Edge-case: undefined should return failure
  // Note: this case must come _after_ the previous case because VT_NULL is
  // also VT_EMPTY, but the converse is not true.
  } else if (t.vt == VT_EMPTY) {
    return false;
  // All other coercions
  } else {
    CComVariant variant;
    HRESULT hr;
    hr = VariantChangeType(&variant, &t, 0, VT_R8);
    if (hr != S_OK) {
      // Edge-case: empty strings should coerce to 0
      if (t.vt == VT_BSTR && !t.bstrVal[0]) {
        *out = 0;
        return true;
      } else {
        return false;
      }
    }
    // Edge-case: NaN should return failure
    if (isnan(variant.dblVal)) { return false; }
    *out = variant.dblVal;
  }
  return true;
}

bool JsTokenToString_Coerce(JsToken t, JsContextPtr cx, std::string16 *out) {
  // JsToken is a string
  if (t.vt == VT_BSTR) {
    // A BSTR must have identical semantics for NULL and for "".
    out->assign(t.bstrVal ? t.bstrVal : STRING16(L""));
  // Edge-case: booleans should coerce to "true" and "false".
  // Note that VariantChangeType() converts booleans to "1" and "0",
  // or "True" and "False" (if VARIANT_ALPHABOOL is specified).
  } else if (t.vt == VT_BOOL) {
    if (t.boolVal == VARIANT_TRUE) {
      out->assign(STRING16(L"true"));
    } else {
      out->assign(STRING16(L"false"));
    }
  // Edge-case: null should coerce to "null"
  } else if (t.vt == VT_NULL) {
    out->assign(STRING16(L"null"));
  // Edge-case: undefined should coerce to "undefined"
  // Note: see comment in JsTokenToDouble_Coerce.
  } else if (t.vt == VT_EMPTY) {
    out->assign(STRING16(L"undefined"));
  // Edge-case: NaN should coerce to "NaN"
  } else if (t.vt == VT_R8 && isnan(t.dblVal)) {
    out->assign(STRING16(L"NaN"));
  // All other coercions
  } else {
    CComVariant variant;
    HRESULT hr;
    hr = VariantChangeType(&variant, &t, 0, VT_BSTR);
    if (hr != S_OK) { return false; }
    out->assign(variant.bstrVal);
    // We are responsible for calling VariantClear() on the variant we
    // created if it contains a BSTR
    VariantClear(&variant);
  }
  return true;
}

bool JsTokenToModule(JsRunnerInterface *js_runner,
                     JsContextPtr context,
                     const JsToken in,
                     ModuleImplBaseClass **out) {
  if (in.vt == VT_DISPATCH) {
    CComQIPtr<GearsModuleProviderInterface> module_provider(in.pdispVal);
    if (!module_provider) {
      return false;
    }
    VARIANT var;
    HRESULT hr = module_provider->get_moduleWrapper(&var);
    if (FAILED(hr)) {
      return false;
    }
    ModuleWrapper *module_wrapper =
        reinterpret_cast<ModuleWrapper*>(var.byref);
    *out = module_wrapper->GetModuleImplBaseClass();
    return true;
  }
  return false;
}

JsParamType JsTokenGetType(JsToken t, JsContextPtr cx) {
  if (t.vt == VT_BOOL) {
    return JSPARAM_BOOL;
  } else if (t.vt == VT_I4) {
    return JSPARAM_INT;
  } else if (t.vt == VT_R8) {
    return JSPARAM_DOUBLE;
  } else if (t.vt == VT_BSTR) {
    return JSPARAM_STRING16;
  } else if (t.vt == VT_NULL) {
    return JSPARAM_NULL;
  } else if (t.vt == VT_EMPTY) {  // Must come after the previous test
    return JSPARAM_UNDEFINED;
  } else if (JsTokenIsArray(t, cx)) { 
    return JSPARAM_ARRAY;
  } else if (JsTokenIsCallback(t, cx)) {
    return JSPARAM_FUNCTION;
  } else if (JsTokenIsObject(t)) {
    return JSPARAM_OBJECT;
  } else {
    return JSPARAM_UNKNOWN;  // Unsupported type
  }
}

bool JsTokenIsCallback(JsToken t, JsContextPtr cx) {
  if (t.vt != VT_DISPATCH || !t.pdispVal) return false; 
  // Check for call() method which all Function objects have.
  // Note: User defined objects that have a call() method will be incorrectly
  // flagged as callbacks but this check is better than nothing and probably
  // the best we are going to get.
  CComVariant out;
  if (FAILED(ActiveXUtils::GetDispatchProperty(t.pdispVal,
                                               STRING16(L"call"),
                                               &out))) {
    return false;
  }
  return true;
}

bool JsTokenIsArray(JsToken t, JsContextPtr cx) {
  if (t.vt != VT_DISPATCH || !t.pdispVal) return false;
  // Check for join() method which all Array objects have.
  // Note: Function objects also have a length property so it's not safe
  // to assume that an object with a length property is an Array!
  CComVariant out;
  if (FAILED(ActiveXUtils::GetDispatchProperty(t.pdispVal,
                                               STRING16(L"join"),
                                               &out))) {
    return false;
  }
  return true;
}

bool JsTokenIsObject(JsToken t) {
  return t.vt == VT_DISPATCH && t.pdispVal;
}

bool JsTokenIsNullOrUndefined(JsToken t) {
  return t.vt == VT_NULL || t.vt == VT_EMPTY;  // null or undefined
}

bool BoolToJsToken(JsContextPtr context, bool value, JsScopedToken *out) {
  *out = value;
  return true;
}

bool IntToJsToken(JsContextPtr context, int value, JsScopedToken *out) {
  *out = value;
  return true;
}

bool StringToJsToken(JsContextPtr context, const char16 *value,
                     JsScopedToken *out) {
  *out = value;
  return true;
}

bool DoubleToJsToken(JsContextPtr context, double value, JsScopedToken *out) {
  *out = value;
  return true;
}

bool NullToJsToken(JsContextPtr context, JsScopedToken *out) {
  VARIANT null_variant;
  null_variant.vt = VT_NULL;
  *out = null_variant;
  return true;
}

bool UndefinedToJsToken(JsContextPtr context, JsScopedToken *out) {
  VARIANT undefined_variant;
  undefined_variant.vt = VT_EMPTY;
  *out = undefined_variant;
  return true;
}

#elif BROWSER_NPAPI

bool JsTokenToBool_NoCoerce(JsToken t, JsContextPtr cx, bool *out) {
  if (!NPVARIANT_IS_BOOLEAN(t)) { return false; }
  *out = NPVARIANT_TO_BOOLEAN(t);
  return true;
}

bool JsTokenToInt_NoCoerce(JsToken t, JsContextPtr cx, int *out) {
  // NOTE: WebKit's implementation of NPVARIANT always returns JavaScript
  // numbers as doubles, even if they are integral and small enough to fit in
  // int32. Therefore this first branch never gets hit in WebKit today. However,
  // it should get hit in other NPAPI implementations.
  if (NPVARIANT_IS_INT32(t)) {
    *out = NPVARIANT_TO_INT32(t);
    return true;
  } else if (NPVARIANT_IS_DOUBLE(t)) {
    double d = NPVARIANT_TO_DOUBLE(t);
    if (DoubleIsInt32(d)) {
      *out = static_cast<int>(d);
      return true;
    }
  }
  return false;
}

bool JsTokenToDouble_NoCoerce(JsToken t, JsContextPtr cx, double *out) {
  if (NPVARIANT_IS_DOUBLE(t)) {
    *out = NPVARIANT_TO_DOUBLE(t);
    return true;
  } else if (NPVARIANT_IS_INT32(t)) {
    *out = static_cast<double>(NPVARIANT_TO_INT32(t));
    return true;
  }

  return false;
}

bool JsTokenToString_NoCoerce(JsToken t, JsContextPtr cx, std::string16 *out) {
  if (!NPVARIANT_IS_STRING(t)) { return false; }
  NPString str = NPVARIANT_TO_STRING(t);
  if (GetNPStringUTF8Length(str) == 0) {
    // TODO(mpcomplete): find out if UTF8ToString16 can be changed to return
    // true in this case.
    out->clear();
    return true;
  }
  return UTF8ToString16(GetNPStringUTF8Characters(str),
                        GetNPStringUTF8Length(str), out);
}

// TODO: Implement coercion for NPAPI. Since NPAPI does not have built-in
// coercion this will have to be done manually in terms of C++ types. It might
// be worth considering reusing that code across all browsers, for consistency.
bool JsTokenToBool_Coerce(JsToken t, JsContextPtr cx, bool *out) {
  // JsToken is a bool
  if (JsTokenGetType(t, cx) == JSPARAM_BOOL) {
    return JsTokenToBool_NoCoerce(t, cx, out);
  // Try to coerce the JsToken to a bool
  // The only values that should be treated as false in JavaScript are false,
  // null, NaN, 0, undefined and an empty string. false is already covered by
  // the case above.
  } else if (JsTokenIsNullOrUndefined(t) ||  // null, undefined
             NPVARIANT_IS_DOUBLE(t) && // 0 or Nan
                 (NPVARIANT_TO_DOUBLE(t) == 0 ||
                  isnan(NPVARIANT_TO_DOUBLE(t))) || 
             NPVARIANT_IS_INT32(t) && NPVARIANT_TO_INT32(t) == 0 ||  // 0
             NPVARIANT_IS_STRING(t) &&  // empty string
                 GetNPStringUTF8Length(NPVARIANT_TO_STRING(t)) == 0) {
    *out = false;
  } else {
    *out = true;  // Anything else is true
  }
  return true;
}

bool JsTokenToInt_Coerce(JsToken t, JsContextPtr cx, int *out) {
  if (JsTokenGetType(t, cx) == JSPARAM_INT) {
    return JsTokenToInt_NoCoerce(t, cx, out);
  // Try to coerce the JsToken to an int.
  } else {
    double output;
    if (!JsTokenToDouble_Coerce(t, cx, &output)) { return false; }
    // Make sure the double will fit into an int
    if (output < kint32min || output > kint32max) {
      return false;
    }
    *out = static_cast<int>(output);
  }
  return true;
}

bool JsTokenToDouble_Coerce(JsToken t, JsContextPtr cx, double *out) {
  // This coercion is very tricky to get just-right. See test cases in
  // test/testcases/internal_tests.js before making any changes. The expected
  // outputs are based on the result of doing a Number(testval) in JavaScript.
  JsParamType type = JsTokenGetType(t, cx);
  // JsToken is a double
  if (type == JSPARAM_DOUBLE) {
    // Edge-case: NaN should return failure
    if (isnan(NPVARIANT_TO_DOUBLE(t))) { return false; }
    *out = NPVARIANT_TO_DOUBLE(t);
  // JsToken is an integer (or a double that can be converted to an integer).
  } else if (type == JSPARAM_INT) {
    int out_int;
    if (!JsTokenToInt_NoCoerce(t, cx, &out_int))
      return false;
    *out = out_int;
    return true;
  // Edge-case: boolean true should coerce to 1, not -1.
  } else if (type == JSPARAM_BOOL) {
    *out = NPVARIANT_TO_BOOLEAN(t) ? 1 : 0;
  // Edge-case: null should coerce to 0
  } else if (NPVARIANT_IS_NULL(t)) {
    *out = 0;
  // Edge-case: undefined should return failure
  } else if (NPVARIANT_IS_VOID(t)) {
    return false;
  // Strings
  } else if (NPVARIANT_IS_STRING(t)) {
    // TODO(mpcomplete): I'm not sure if NPStrings are guaranteed to be null
    // terminated.  Do we need the str temporary?
    NPString s = NPVARIANT_TO_STRING(t);
    std::string str(GetNPStringUTF8Characters(s), GetNPStringUTF8Length(s));

    // Edge-case: NaN should return failure
    if (StringCompareIgnoreCase(str.c_str(), "nan") == 0) {
      return false;
    }

    char *end;
    *out = strtod(str.c_str(), &end);
    if (*end)
      return false;  // not every char was part of the number

    return true;
  // All other coercions
  } else {
    return false;
  }
  return true;
}

bool JsTokenToString_Coerce(JsToken t, JsContextPtr cx, std::string16 *out) {
  JsParamType param_type = JsTokenGetType(t, cx);
  bool converted_succesfully = false;
  
  switch(param_type) {
    case JSPARAM_UNDEFINED: {
      *out = STRING16(L"undefined");
      converted_succesfully = true;
      break;
    }
    case JSPARAM_NULL: {
      *out = STRING16(L"null");
      converted_succesfully = true;
      break;
    }
    case JSPARAM_BOOL: {
      bool bool_val;
      converted_succesfully = JsTokenToBool_NoCoerce(t, cx, &bool_val);
      if (converted_succesfully) {
        *out = bool_val ? STRING16(L"true") : STRING16(L"false");
      }
      break;
    }
    case JSPARAM_STRING16: {
      converted_succesfully = JsTokenToString_NoCoerce(t, cx, out);
      break;
    }
    case JSPARAM_INT: {
      int int_val;
      converted_succesfully = JsTokenToInt_NoCoerce(t, cx, &int_val);
      if (converted_succesfully) {
        *out = IntegerToString16(int_val);
      }
      break;
    }
    case JSPARAM_DOUBLE: {
      double double_val;
      converted_succesfully = JsTokenToDouble_NoCoerce(t, cx, &double_val);
      if (converted_succesfully) {
        // Edge case: NaN
        if (isnan(double_val)) {
          *out = STRING16(L"NaN");
          break;
        }

        char buf[1024];
        int len = snprintf(buf, sizeof(buf), "%f", double_val);

        // Trim trailing 0s.  This comes up, for example, with 3.14159, which
        // is inaccurately represented, so sprintf prints it as "3.141590".
        // (This is pretty bogus... maybe we should just change the test.)
        while (len > 1 && buf[len-1] == '0')
          --len;

        converted_succesfully = UTF8ToString16(buf, len, out);
      }
      break;
    }
    case JSPARAM_ARRAY:
    case JSPARAM_OBJECT: {
      // Call object.toString().
      NPObject *object = NPVARIANT_TO_OBJECT(t);
      ScopedNPVariant result;
      NPIdentifier tostring_id = NPN_GetStringIdentifier("toString");
      if (NPN_Invoke(cx, object, tostring_id, NULL, 0, &result) &&
          NPVARIANT_IS_STRING(result)) {
        NPString str = NPVARIANT_TO_STRING(result);
        converted_succesfully =
            UTF8ToString16(str.UTF8Characters, str.UTF8Length, out);
      }

      break;
    }
    default: {
      break;
    }
  }
  return converted_succesfully;
}

bool JsTokenToModule(JsRunnerInterface *js_runner,
                     JsContextPtr context,
                     const JsToken in,
                     ModuleImplBaseClass **out) {
  if (!NPVARIANT_IS_OBJECT(in)) {
    return false;
  }
  NPObject *object = NPVARIANT_TO_OBJECT(in);
  if (object->_class != ModuleWrapper::GetNPClass()) {
    return false;
  }
  *out = static_cast<ModuleWrapper*>(object)->GetModuleImplBaseClass();
  return true;
}

JsParamType JsTokenGetType(JsToken t, JsContextPtr cx) {
  if (NPVARIANT_IS_BOOLEAN(t)) {
    return JSPARAM_BOOL;
  } else if (NPVARIANT_IS_INT32(t)) {
    return JSPARAM_INT;
  } else if (NPVARIANT_IS_DOUBLE(t)) {
    // Patch for WebKit, which reports both ints and doubles as both being of
    // type JSPARAM_DOUBLE.
    double double_val = NPVARIANT_TO_DOUBLE(t);
    if (DoubleIsInt32(double_val)) {
      return JSPARAM_INT;
    }
    return JSPARAM_DOUBLE;
  } else if (NPVARIANT_IS_STRING(t)) {
    return JSPARAM_STRING16;
  } else if (NPVARIANT_IS_NULL(t)) {
    return JSPARAM_NULL;
  } else if (NPVARIANT_IS_VOID(t)) {
    return JSPARAM_UNDEFINED;
  } else if (JsTokenIsArray(t, cx)) {
    return JSPARAM_ARRAY;
  } else if (JsTokenIsCallback(t, cx)) {
    return JSPARAM_FUNCTION;
  } else if (JsTokenIsObject(t)) {
    return JSPARAM_OBJECT;
  } else {
    return JSPARAM_UNKNOWN;  // Unsupported type
  }
}

bool JsTokenIsCallback(JsToken t, JsContextPtr cx) {
  if (!NPVARIANT_IS_OBJECT(t))
    return false;

  // Check for call() method which all Function objects have.
  // Note: User defined objects that have a call() method will be incorrectly
  // flagged as callbacks but this check is better than nothing and probably
  // the best we are going to get.
  NPIdentifier call_id = NPN_GetStringIdentifier("call");
  ScopedNPVariant out;
  if (!NPN_GetProperty(cx, NPVARIANT_TO_OBJECT(t), call_id, &out) ||
      NPVARIANT_IS_VOID(out)) {
    return false;
  }
  return true;
}

bool JsTokenIsArray(JsToken t, JsContextPtr cx) {
  if (!NPVARIANT_IS_OBJECT(t))
    return false;

  // Check for join() method which all Array objects have.
  // Note: Function objects also have a length property so it's not safe
  // to assume that an object with a length property is an Array!
  NPIdentifier join_id = NPN_GetStringIdentifier("join");
  ScopedNPVariant out;
  if (!NPN_GetProperty(cx, NPVARIANT_TO_OBJECT(t), join_id, &out) ||
      NPVARIANT_IS_VOID(out)) {
    return false;
  }
  return true;
}

bool JsTokenIsObject(JsToken t) {
  return NPVARIANT_IS_OBJECT(t);
}

bool JsTokenIsNullOrUndefined(JsToken t) {
  return NPVARIANT_IS_NULL(t) || NPVARIANT_IS_VOID(t);
}

bool BoolToJsToken(JsContextPtr context, bool value, JsScopedToken *out) {
  out->Reset(value);
  return true;
}

bool IntToJsToken(JsContextPtr context, int value, JsScopedToken *out) {
  out->Reset(value);
  return true;
}

bool StringToJsToken(JsContextPtr context, const char16 *value,
                     JsScopedToken *out) {
  out->Reset(value);
  return true;
}

bool DoubleToJsToken(JsContextPtr context, double value, JsScopedToken *out) {
  out->Reset(value);
  return true;
}

bool NullToJsToken(JsContextPtr context, JsScopedToken *out) {
  out->ResetToNull();
  return true;
}

bool UndefinedToJsToken(JsContextPtr context, JsScopedToken *out) {
  out->Reset();
  return true;
}

// ScopedNPVariant functions.
void ScopedNPVariant::Reset() {
  NPN_ReleaseVariantValue(this);
  VOID_TO_NPVARIANT(*this);
}

void ScopedNPVariant::ResetToNull() {
  Reset();
  NULL_TO_NPVARIANT(*this);
}

void ScopedNPVariant::Reset(int value) {
  Reset();
  INT32_TO_NPVARIANT(value, *this);
}

void ScopedNPVariant::Reset(bool value) {
  Reset();
  BOOLEAN_TO_NPVARIANT(value, *this);
}

void ScopedNPVariant::Reset(double value) {
  Reset();
  DOUBLE_TO_NPVARIANT(value, *this);
}

void ScopedNPVariant::Reset(const char *value) {
  Reset();
  NPString npstr = NPN_StringDup(value, strlen(value));
  NPSTRING_TO_NPVARIANT(npstr, *this);
}

void ScopedNPVariant::Reset(const char16 *value) {
  Reset();
  NPString npstr = NPN_StringDup(value,
                                 std::char_traits<char16>::length(value));
  NPSTRING_TO_NPVARIANT(npstr, *this);
}

void ScopedNPVariant::Reset(NPObject *value) {
  Reset();
  OBJECT_TO_NPVARIANT(value, *this);
  NPN_RetainObject(value);
}

void ScopedNPVariant::Reset(const NPVariant &value) {
  if (static_cast<NPVariant*>(this) == &value) return;

  Reset();
  memcpy(this, &value, sizeof(value));
  if (NPVARIANT_IS_OBJECT(value)) {
    NPN_RetainObject(NPVARIANT_TO_OBJECT(*this));
  } else if (NPVARIANT_IS_STRING(value)) {
    NPString npstr = NPN_StringDup(NPVARIANT_TO_STRING(*this));
    NPSTRING_TO_NPVARIANT(npstr, *this);
  }
}

void ScopedNPVariant::Release() {
  VOID_TO_NPVARIANT(*this);
}

#endif

//------------------------------------------------------------------------------
// JsRootedToken
//------------------------------------------------------------------------------

#if BROWSER_FF

JsRootedToken::JsRootedToken(JsContextPtr context, JsToken token)
    : context_(context), token_(token) {
  LEAK_COUNTER_INCREMENT(JsRootedToken);
  if (JSVAL_IS_GCTHING(token_)) {
    JS_BeginRequest(context_);
    JS_AddRoot(context_, &token_);
    JS_EndRequest(context_);
  }
}

JsRootedToken::~JsRootedToken() {
  LEAK_COUNTER_DECREMENT(JsRootedToken);
  if (JSVAL_IS_GCTHING(token_)) {
    JS_BeginRequest(context_);
    JS_RemoveRoot(context_, &token_);
    JS_EndRequest(context_);
  }
}

#elif BROWSER_IE

JsRootedToken::JsRootedToken(JsContextPtr context, JsToken token)
    : token_(token) { // IE doesn't use JsContextPtr
  LEAK_COUNTER_INCREMENT(JsRootedToken);
  if (token_.vt == VT_DISPATCH) {
    token_.pdispVal->AddRef();
  }
}

JsRootedToken::~JsRootedToken() {
  LEAK_COUNTER_DECREMENT(JsRootedToken);
  if (token_.vt == VT_DISPATCH) {
    token_.pdispVal->Release();
  }
}

#elif BROWSER_NPAPI

JsRootedToken::JsRootedToken(JsContextPtr context, JsToken token)
    : context_(context), token_(token) {
  LEAK_COUNTER_INCREMENT(JsRootedToken);
}

JsRootedToken::~JsRootedToken() {
  LEAK_COUNTER_DECREMENT(JsRootedToken);
}

#endif

//------------------------------------------------------------------------------
// ConvertJsParamToToken, JsCallContext
//------------------------------------------------------------------------------

#if BROWSER_FF

bool ConvertJsParamToToken(const JsParamToSend &param,
                           JsContextPtr context, JsScopedToken *token) {
  JsRequest request(context);
  switch (param.type) {
    case JSPARAM_BOOL: {
      const bool *value = static_cast<const bool *>(param.value_ptr);
      *token = *value ? JSVAL_TRUE : JSVAL_FALSE;
      break;
    }
    case JSPARAM_INT: {
      const int *value = static_cast<const int *>(param.value_ptr);
      *token = INT_TO_JSVAL(*value);
      break;
    }
    case JSPARAM_INT64: {
      const int64 *value = static_cast<const int64 *>(param.value_ptr);
      // If the value fits inside a 31-bit signed int (i.e. 30 regular bits
      // and 1 sign bit), then using an int is more efficient, on SpiderMonkey,
      // than a heap-allocated double.
      if ((*value >= JSVAL_INT_MIN) && (*value <= JSVAL_INT_MAX)) {
        int value_as_int32 = static_cast<int32>(*value);
        *token = INT_TO_JSVAL(value_as_int32);
        break;
      }
      // Otherwise, fall back to a double, although if the value can't be
      // represented by a double, without loss of precision, then we will fail.
      if ((*value < JS_INT_MIN) || (*value > JS_INT_MAX)) {
        return false;
      }
      const double dvalue = static_cast<double>(*value);
      jsdouble *js_double = JS_NewDouble(context, dvalue);
      *token = DOUBLE_TO_JSVAL(js_double);
      break;
    }
    case JSPARAM_DOUBLE: {
      const double *value = static_cast<const double *>(param.value_ptr);
      jsdouble *js_double = JS_NewDouble(context, *value);
      *token = DOUBLE_TO_JSVAL(js_double);
      break;
    }
    case JSPARAM_STRING16: {
      const std::string16 *value = static_cast<const std::string16 *>(
                                                   param.value_ptr);
      JSString *js_string = JS_NewUCStringCopyZ(
          context,
          reinterpret_cast<const jschar *>(value->c_str()));
      *token = STRING_TO_JSVAL(js_string);
      break;
    }
    case JSPARAM_OBJECT: {
      const JsObject *value = static_cast<const JsObject *>(param.value_ptr);
      *token = value->token();
      break;
    }
    case JSPARAM_ARRAY: {
      const JsArray *value = static_cast<const JsArray *>(param.value_ptr);
      *token = value->token();
      break;
    }
    case JSPARAM_FUNCTION: {
      const JsRootedCallback *value = static_cast<const JsRootedCallback *>(
                                                   param.value_ptr);
      *token = value->token();
      break;
    }
    case JSPARAM_MODULE: {
      const ModuleImplBaseClass *value =
          static_cast<const ModuleImplBaseClass *>(param.value_ptr);
      *token = value->GetWrapperToken();
      break;
    }
    case JSPARAM_NULL:
      *token = JSVAL_NULL;
      break;
    case JSPARAM_UNDEFINED:
      *token = JSVAL_VOID;
      break;
    default:
      assert(false);
  }
  return true;
}

JsCallContext::JsCallContext(JsContextPtr cx, JsRunnerInterface *js_runner,
                             int argc, JsToken *argv, JsToken *retval)
    : js_context_(cx), is_exception_set_(false), is_return_value_set_(false),
      argc_(argc), argv_(argv), retval_(retval),
      js_runner_(js_runner) {
  LEAK_COUNTER_INCREMENT(JsCallContext);
}

void JsCallContext::SetReturnValue(JsParamType type, const void *value_ptr) {
  // There is only a valid retval_ if the JS caller is expecting a return value.
  if (retval_) {
    JsParamToSend retval = { type, value_ptr };
    if (ConvertJsParamToToken(retval, js_context(), retval_)) {
      is_return_value_set_ = true;
    } else {
      SetException(STRING16(L"Return value is out of range."));
    }
  }
}

void JsCallContext::SetException(const std::string16 &message) {
  assert(!message.empty());
  is_exception_set_ = true;

  // First set the exception to any value, in case we fail to create the full
  // exception object below.  Setting any jsval will satisfy the JS engine,
  // we just won't get e.message.  We use INT_TO_JSVAL(1) here for simplicity.
  JS_BeginRequest(js_context_);
  JS_SetPendingException(js_context_, INT_TO_JSVAL(1));
  JS_EndRequest(js_context_);

  // Create a JS Error object with a '.message' property. The other fields
  // like "lineNumber" and "fileName" are filled in automatically by Firefox
  // based on the top frame of the JS stack. It's important to use an actual
  // Error object so that some tools work correctly. See:
  // http://code.google.com/p/google-gears/issues/detail?id=5
  //
  // Note: JS_ThrowReportedError and JS_ReportError look promising, but they
  // don't quite do what we need.

  scoped_ptr<JsObject> error_object(js_runner_->NewError(message.c_str()));
  if (!error_object.get()) { return; }

  // Note: need JS_SetPendingException to bubble 'catch' in workers.
  JS_BeginRequest(js_context_);
  JS_SetPendingException(js_context_, error_object->token());
  JS_EndRequest(js_context_);
}

#elif BROWSER_IE

bool ConvertJsParamToToken(const JsParamToSend &param,
                           JsContextPtr context, CComVariant *token) {
  switch (param.type) {
    case JSPARAM_BOOL: {
      const bool *value = static_cast<const bool *>(param.value_ptr);
      *token = *value;  // CComVariant understands 'bool'
      break;
    }
    case JSPARAM_INT: {
      const int *value = static_cast<const int *>(param.value_ptr);
      *token = *value;  // CComVariant understands 'int'
      break;
    }
    case JSPARAM_INT64: {
      const int64 *value = static_cast<const int64 *>(param.value_ptr);
      // If the value can't be represented by a double, without loss of
      // precision, then we will fail.
      if ((*value < JS_INT_MIN) || (*value > JS_INT_MAX)) {
        return false;
      }
      const double dvalue = static_cast<double>(*value);
      *token = dvalue;  // CComVariant understands 'double'
      break;
    }
    case JSPARAM_DOUBLE: {
      const double *value = static_cast<const double *>(param.value_ptr);
      *token = *value;  // CComVariant understands 'double'
      break;
    }
    case JSPARAM_STRING16: {
      const std::string16 *value = static_cast<const std::string16 *>(
                                                   param.value_ptr);
      *token = value->c_str();  // copies 'wchar*' for us
      break;
    }
    case JSPARAM_OBJECT: {
      const JsObject *value = static_cast<const JsObject *>(param.value_ptr);
      *token = value->token();
      break;
    }
    case JSPARAM_ARRAY: {
      const JsArray *value = static_cast<const JsArray *>(param.value_ptr);
      *token = value->token();
      break;
    }
    case JSPARAM_FUNCTION: {
      const JsRootedCallback *value = static_cast<const JsRootedCallback *>(
                                                   param.value_ptr);
      *token = value->token();
      break;
    }
    case JSPARAM_MODULE: {
      const ModuleImplBaseClass *value =
          static_cast<const ModuleImplBaseClass *>(param.value_ptr);
      *token = value->GetWrapperToken();
      break;
    }
    case JSPARAM_NULL:
      VARIANT null_variant;
      null_variant.vt = VT_NULL;
      *token = null_variant;
      break;
    case JSPARAM_UNDEFINED:
      // Setting *token = VT_EMPTY doesn't seem to work.
      token->Clear();
      break;
    default:
      assert(false);
  }
  return true;
}

JsCallContext::JsCallContext(DISPPARAMS FAR *disp_params, VARIANT FAR *retval,
                             EXCEPINFO FAR *excep_info)
    : disp_params_(disp_params), retval_(retval), exception_info_(excep_info),
      is_exception_set_(false), is_return_value_set_(false) {
  LEAK_COUNTER_INCREMENT(JsCallContext);
}

void JsCallContext::SetReturnValue(JsParamType type, const void *value_ptr) {
  // There is only a valid retval_ if the javascript caller is expecting a
  // return value.
  if (retval_) {
    JsParamToSend retval = { type, value_ptr };
    JsScopedToken scoped_retval;
    if (ConvertJsParamToToken(retval, js_context(), &scoped_retval)) {
      is_return_value_set_ = true;
      // In COM, return values are released by the caller.
      scoped_retval.Detach(retval_);
    } else {
      SetException(STRING16(L"Return value is out of range."));
    }
  }
}

void JsCallContext::SetException(const std::string16 &message) {
  assert(!message.empty());
  if (!exception_info_) {
#if DEBUG
    // MSDN says exception_info_ can be null, which seems very unfortunate.
    // Asserting to see if we can find out under what conditions that's true.
    assert(false);
#endif
    return;
  }

  exception_info_->wCode = 1001; // Not used, MSDN says must be > 1000.
  exception_info_->wReserved = 0;
  exception_info_->bstrSource = SysAllocString(PRODUCT_FRIENDLY_NAME);
  exception_info_->bstrDescription = SysAllocString(message.c_str());
  exception_info_->bstrHelpFile = NULL;
  exception_info_->dwHelpContext = 0;
  exception_info_->pvReserved = NULL;
  exception_info_->pfnDeferredFillIn = NULL;
  exception_info_->scode = 0;

  is_exception_set_ = true;
}

#elif BROWSER_NPAPI

bool ConvertJsParamToToken(const JsParamToSend &param,
                           JsContextPtr context, JsScopedToken *variant) {
  switch (param.type) {
    case JSPARAM_BOOL: {
      const bool *value = static_cast<const bool *>(param.value_ptr);
      variant->Reset(*value);
      break;
    }
    case JSPARAM_INT: {
      const int *value = static_cast<const int *>(param.value_ptr);
      variant->Reset(*value);
      break;
    }
    case JSPARAM_INT64: {
      const int64 *value = static_cast<const int64 *>(param.value_ptr);
      // If the value can't be represented by a double, without loss of
      // precision, then we will fail.
      if ((*value < JS_INT_MIN) || (*value > JS_INT_MAX)) {
        return false;
      }
      const double dvalue = static_cast<double>(*value);
      variant->Reset(dvalue);
      break;
    }
    case JSPARAM_DOUBLE: {
      const double *value = static_cast<const double *>(param.value_ptr);
      variant->Reset(*value);
      break;
    }
    case JSPARAM_MODULE: {
      const ModuleImplBaseClass *value =
          static_cast<const ModuleImplBaseClass *>(param.value_ptr);
      variant->Reset(NPVARIANT_TO_OBJECT(value->GetWrapperToken()));
      break;
    }
    case JSPARAM_OBJECT: {
      const JsObject *value = static_cast<const JsObject *>(param.value_ptr);
      variant->Reset(NPVARIANT_TO_OBJECT(value->token()));
      break;
    }
    case JSPARAM_ARRAY: {
      const JsArray *value = static_cast<const JsArray *>(param.value_ptr);
      variant->Reset(NPVARIANT_TO_OBJECT(value->token()));
      break;
    }
    case JSPARAM_FUNCTION: {
      const JsRootedCallback *value =
          static_cast<const JsRootedCallback *>(param.value_ptr);
      variant->Reset(NPVARIANT_TO_OBJECT(value->token()));
      break;
    }
    case JSPARAM_STRING16: {
      const std::string16 *value = static_cast<const std::string16 *>(
                                                   param.value_ptr);
      variant->Reset(value->c_str());
      break;
    }
    case JSPARAM_NULL: {
      variant->Reset();  // makes it VOID (undefined).
      NULL_TO_NPVARIANT(*variant);
      break;
    }
    case JSPARAM_UNDEFINED: {
      variant->Reset();  // makes it VOID (undefined).
      VOID_TO_NPVARIANT(*variant);  // TODO(steveblock): Is this needed?
      break;
    }
    default:
      assert(false);
  }
  return true;
}

JsCallContext::JsCallContext(JsContextPtr js_context, NPObject *object,
                             int argc, const JsToken *argv, JsToken *retval)
    : js_context_(js_context), is_exception_set_(false),
      is_return_value_set_(false), object_(object),
      argc_(argc), argv_(argv), retval_(retval) {
  LEAK_COUNTER_INCREMENT(JsCallContext);
}

void JsCallContext::SetReturnValue(JsParamType type, const void *value_ptr) {
  assert(value_ptr != NULL || type == JSPARAM_NULL);

  JsParamToSend retval = { type, value_ptr };
  ScopedNPVariant np_retval;
  if (ConvertJsParamToToken(retval, js_context(), &np_retval)) {
    *retval_ = np_retval;
    is_return_value_set_ = true;
    // In NPAPI, return values from callbacks are released by the browser.
    // Therefore, we give up ownership of this variant without releasing it.
    np_retval.Release();
  } else {
    SetException(STRING16(L"Return value is out of range."));
  }
}

void JsCallContext::SetException(const std::string16 &message) {
  assert(!message.empty());
  // TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string message_ascii;
  String16ToUTF8(message.c_str(), message.length(), &message_ascii);
  LOG(("SetException: %s\n", message_ascii.c_str()));
#endif

  is_exception_set_ = true;
  
  std::string message_utf8;
  if (!String16ToUTF8(message.data(), message.length(), &message_utf8))
    message_utf8 = "Unknown Gears Error";  // better to throw *something*

  NPObject *window;
  if (NPN_GetValue(js_context(), NPNVWindowNPObject, &window) != NPERR_NO_ERROR)
    return;
  ScopedNPObject window_scoped(window);
  NPN_SetException(window, message_utf8.c_str());
}

#endif

#if defined(BROWSER_FF) || defined(BROWSER_NPAPI)

int JsCallContext::GetArgumentCount() {
  return argc_;
}

const JsToken &JsCallContext::GetArgument(int index) {
  return argv_[index];
}

#elif BROWSER_IE

int JsCallContext::GetArgumentCount() {
  return disp_params_->cArgs;
}

const JsToken &JsCallContext::GetArgument(int index) {
  int arg_index = disp_params_->cArgs - index - 1;
  return disp_params_->rgvarg[arg_index];
}

#endif

int JsCallContext::GetArguments(int output_argc, JsArgument *output_argv) {
  bool has_optional = false;

  if (GetArgumentCount() > output_argc) {
    SetException(STRING16(L"Too many parameters."));
    return 0;
  }

  for (int i = 0; i < output_argc; ++i) {
    assert(output_argv[i].value_ptr);

    has_optional |= output_argv[i].requirement == JSPARAM_OPTIONAL;
    if (output_argv[i].requirement == JSPARAM_REQUIRED)
      assert(!has_optional);  // should not have required arg after optional

    // TODO(mpcomplete): We need to handle this more generally.  We should
    // continue processing arguments for the case where a developer does
    // something like 'method(null, foo)' if the first argument is optional.
    if (i >= GetArgumentCount() ||
        GetArgumentType(i) == JSPARAM_NULL || 
        GetArgumentType(i) == JSPARAM_UNDEFINED) {
      // Out of arguments
      if (output_argv[i].requirement == JSPARAM_REQUIRED) {
        std::string16 msg;
        msg += STRING16(L"Required argument ");
        msg += IntegerToString16(i + 1);
        msg += STRING16(L" is missing.");
        SetException(msg.c_str());
      }

      // If failed on index [N], then N args succeeded
      return i;
    }

    if (!ConvertTokenToArgument(this, GetArgument(i), &output_argv[i])) {
      std::string16 msg(STRING16(L"Argument "));
      msg += IntegerToString16(i + 1);
      msg += STRING16(L" has invalid type or is outside allowed range.");
      SetException(msg);
      return i;
    }
  }

  return output_argc;
}

bool JsCallContext::GetArguments2(int output_argc, JsArgument *output_argv) {

  if (GetArgumentCount() > output_argc) {
    SetException(STRING16(L"Too many parameters."));
    return false;
  }

  for (int i = 0; i < output_argc; ++i) {
    assert(output_argv[i].value_ptr);
    // Do we have more arguments to look at?
    if (i >= GetArgumentCount()) {
      // Out of arguments.
      // Scan the rest of the expected arguments and see if there are
      // any JSPARAM_REQUIRED. If so, set an exception.
      for (; i < output_argc; ++i) {
        if (output_argv[i].requirement == JSPARAM_REQUIRED) {
          std::string16 msg;
          msg += STRING16(L"Required argument ");
          msg += IntegerToString16(i + 1);
          msg += STRING16(L" is missing.");
          SetException(msg.c_str());    
          return false;
        }
      }
      // No JSPARAM_REQUIRED left to fill. The caller actually read
      // the documentation. Reward with a true.
      return true;
    }

    // We have arguments left to inspect. Inspect.
    // Is it null? Is it undefined?
    if (GetArgumentType(i) == JSPARAM_NULL ||
        GetArgumentType(i) == JSPARAM_UNDEFINED) {
      if (output_argv[i].requirement == JSPARAM_REQUIRED) {
        // This argument is required, but what was passed was null or
        // undefinded, so set an exception.
        std::string16 msg;
        msg += STRING16(L"Null or undefined passed for required argument ");
        msg += IntegerToString16(i + 1);
        msg += STRING16(L".");
        SetException(msg.c_str());
        return false;
      } else {
        // This argument is optional so null or undefined values are ok.
        output_argv[i].was_specified = false;
      }
    } else {
      // What was passed is not null or undefined. Attempt to
      // convert to the expected type.
      if (!ConvertTokenToArgument(this, GetArgument(i), &output_argv[i])) {
          std::string16 msg(STRING16(L"Argument "));
          msg += IntegerToString16(i + 1);
          msg += STRING16(L" has invalid type or is outside allowed range.");
          SetException(msg);
          return false;
      }
      // Conversion completed without fault.
      output_argv[i].was_specified = true;
    }
  }

  return true;
}


JsParamType JsCallContext::GetArgumentType(int i) {
  if (i >= GetArgumentCount()) return JSPARAM_UNKNOWN;
  return JsTokenGetType(GetArgument(i), js_context());
}

JsCallContext::~JsCallContext() {
  LEAK_COUNTER_DECREMENT(JsCallContext);
}
