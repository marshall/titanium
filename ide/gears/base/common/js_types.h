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

#ifndef GEARS_BASE_COMMON_JS_TYPES_H__
#define GEARS_BASE_COMMON_JS_TYPES_H__

#include <assert.h>
#include <functional>
#include <vector>
#include "gears/base/common/basictypes.h"  // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/common.h"
#include "gears/base/common/string16.h"  // for string16

class JsObject;
class JsRootedToken;
class JsRunnerInterface;
class MarshaledJsToken;
class ModuleImplBaseClass;
typedef JsRootedToken JsRootedCallback;

// TODO(michaeln): Split into multiple browser specific files.

//------------------------------------------------------------------------------
// JsToken and friends
//------------------------------------------------------------------------------

#if BROWSER_FF

#include <gecko_internal/jsapi.h>

// Abstracted types for values used with JavaScript engines.
typedef jsval JsToken;
typedef jsval JsScopedToken;  // unneeded in FF, see comment on JsArray
typedef JSContext* JsContextPtr;

// TODO(michaeln): compare string values rather than pointers?
struct JsTokenEqualTo : public std::equal_to<JsToken> {
  JsTokenEqualTo(JsRunnerInterface *js_runner) {}
};

#elif BROWSER_IE

#include <windows.h>
// no "base_interface_ie.h" because IE doesn't require a COM base interface

// Abstracted types for values used with JavaScript engines.
typedef VARIANT JsToken;
typedef CComVariant JsScopedToken;
typedef void* JsContextPtr;  // unused in IE

// On Firefox, a JsToken is a jsval, which is an int, and the natural
// operator== is fine.  On IE, JsToken is a VARIANT, which does not have a
// natural operator==, so we have to make one here.
struct JsTokenEqualTo : public std::binary_function<JsToken, JsToken, bool> {
  JsTokenEqualTo(JsRunnerInterface *js_runner) {}
  bool operator()(const JsToken &x, const JsToken &y) const {
    // All we are looking for in this comparator is that different VARIANTs
    // will compare differently, but that the same IDispatch* (wrapped as a
    // VARIANT) will compare the same.  A non-goal is that the VARIANT
    // representing the integer 3 is "equal to" one representing 3.0.
    if (x.vt != y.vt) {
      return false;
    }
    switch (x.vt) {
      case VT_EMPTY:
        return true;
        break;
      case VT_NULL:
        return true;
        break;
      case VT_I4:
        return x.lVal == y.lVal;
        break;
      case VT_R8:
        return x.dblVal == y.dblVal;
        break;
      case VT_BSTR:
        // TODO(michaeln): compare string values rather than pointers?
        return x.bstrVal == y.bstrVal;
        break;
      case VT_DISPATCH:
        return x.pdispVal == y.pdispVal;
        break;
      case VT_BOOL:
        return x.boolVal == y.boolVal;
        break;
      default:
        // do nothing
        break;
    }
    return false;
  }
};

#elif BROWSER_WEBKIT

#include <WebKit/npapi.h>
#include <WebKit/npfunctions.h>
#include <WebKit/npruntime.h>

#elif BROWSER_NPAPI

#include "third_party/npapi/nphostapi.h"

#endif

#if BROWSER_NPAPI || BROWSER_WEBKIT
// An NPVariant that takes ownership of its value and releases it when it goes
// out of scope.
class ScopedNPVariant : public NPVariant {
 public:
  ScopedNPVariant() { VOID_TO_NPVARIANT(*this); }
  // Must not be 'explicit' so we can use it as a JsScopedToken transparently.
  template<class T>
  ScopedNPVariant(T value) { VOID_TO_NPVARIANT(*this); Reset(value); }

  ~ScopedNPVariant() { Reset(); }

  // This is necessary so we can use it as a JsScopedToken transparently.
  ScopedNPVariant& operator=(const NPVariant &value) {
    Reset(value);
    return *this;
  }
  ScopedNPVariant& operator=(const ScopedNPVariant &value) {
    Reset(value);
    return *this;
  }

  // Frees the old value and replaces it with the new value.  Strings are
  // copied, and objects are retained.
  void Reset();
  void ResetToNull();
  void Reset(bool value);
  void Reset(int value);
  void Reset(double value);
  void Reset(const char *value);
  void Reset(const char16 *value);
  void Reset(NPObject *value);
  void Reset(const NPVariant &value);

  // Gives up ownership of this variant, without freeing or releasing the
  // underyling object.  The variant will be VOID after this call.
  void Release();
};

// Abstracted types for values used with JavaScript engines.
typedef NPVariant JsToken;
typedef ScopedNPVariant JsScopedToken;
typedef NPP JsContextPtr;

struct JsTokenEqualTo : public std::binary_function<JsToken, JsToken, bool>  {
  JsTokenEqualTo(JsRunnerInterface *js_runner);
  JsTokenEqualTo(const JsTokenEqualTo& that) { *this = that; }
  ~JsTokenEqualTo();

  JsTokenEqualTo& operator=(const JsTokenEqualTo &that);

  bool operator()(const JsToken &x, const JsToken &y) const  {
    // All we are looking for in this comparator is that different NPVariants
    // will compare differently, but that the same NPObject* (wrapped as a
    // NPVariant) will compare the same.  A non-goal is that the NPVariant
    // representing the integer 3 is "equal to" one representing 3.0.
    if (x.type != y.type) {
      return false;
    }
    switch (x.type) {
      case NPVariantType_Void:
        return true;
        break;
      case NPVariantType_Null:
        return true;
        break;
      case NPVariantType_Bool:
        return x.value.boolValue == y.value.boolValue;
        break;
      case NPVariantType_Int32:
        return x.value.intValue == y.value.intValue;
        break;
      case NPVariantType_Double:
        return x.value.doubleValue == y.value.doubleValue;
        break;
      case NPVariantType_String:
        // TODO(michaeln): compare string values rather than pointers?
        return x.value.stringValue.UTF8Characters ==
               y.value.stringValue.UTF8Characters;
        break;
      case NPVariantType_Object:
        return CompareObjects(x, y);
        break;
      default:
        // do nothing
        break;
    }
    return false;
  }
 private:
  bool CompareObjects(const JsToken &x, const JsToken &y) const;

  JsRunnerInterface *js_runner_;
  ScopedNPVariant compare_func_;
};

#endif  // BROWSER_NPAPI || BROWSER_WEBKIT

// The JsParam* types define values for sending and receiving JS parameters.
enum JsParamType {
  JSPARAM_BOOL,
  JSPARAM_INT,
  JSPARAM_INT64,
  JSPARAM_DOUBLE,
  JSPARAM_STRING16,
  JSPARAM_OBJECT,
  JSPARAM_ARRAY,
  JSPARAM_FUNCTION,
  JSPARAM_MODULE,
  // JSPARAM_DOM_ELEMENT should only be used from the main JavaScript thread,
  // not from worker threads.
  JSPARAM_DOM_ELEMENT,
  JSPARAM_NULL,
  JSPARAM_UNDEFINED,
  JSPARAM_UNKNOWN,
  JSPARAM_TOKEN
};

enum JsParamRequirement {
  JSPARAM_OPTIONAL,
  JSPARAM_REQUIRED,
};

struct JsParamToSend {
  JsParamType type;
  const void *value_ptr;
};

struct JsArgument {
  JsParamRequirement requirement;
  JsParamType type;
  void* value_ptr;
  bool was_specified;
};

//------------------------------------------------------------------------------
// JsTokenToXxx, XxxToJsToken
//------------------------------------------------------------------------------

// Utility functions to convert JsToken to various C++ types without coercion.
// If the type of the underlying JavaScript variable does not exactly match the
// requested type, these functions return false.
bool JsTokenToBool_NoCoerce(JsToken t, JsContextPtr cx, bool *out);
bool JsTokenToInt_NoCoerce(JsToken t, JsContextPtr cx, int *out);
bool JsTokenToInt64_NoCoerce(JsToken t, JsContextPtr cx, int64 *out);
bool JsTokenToDouble_NoCoerce(JsToken t, JsContextPtr cx, double *out);
bool JsTokenToString_NoCoerce(JsToken t, JsContextPtr cx, std::string16 *out);
bool JsTokenToNewCallback_NoCoerce(JsToken t, JsContextPtr cx,
                                   JsRootedCallback **out);
bool JsTokenToModule(JsRunnerInterface *js_runner,
                     JsContextPtr context,
                     const JsToken in,
                     ModuleImplBaseClass **out);

// Utility functions to coerce JsTokens to various C++ types. These functions
// implement coercion as defined by ECMA 262-3 Section 9:
// http://bclary.com/2004/11/07/#a-9
// They return false if the coercion could not be performed.
bool JsTokenToBool_Coerce(JsToken t, JsContextPtr cx, bool *out);
bool JsTokenToInt_Coerce(JsToken t, JsContextPtr cx, int *out);
bool JsTokenToString_Coerce(JsToken t, JsContextPtr cx, std::string16 *out);
bool JsTokenToDouble_Coerce(JsToken t, JsContextPtr cx, double *out);

// Utility function to determine the type of a JsToken.
JsParamType JsTokenGetType(JsToken t, JsContextPtr cx);

// Utility function to tell if a given token is a JavaScript function.
bool JsTokenIsCallback(JsToken t, JsContextPtr cx);

// Utility function to tell if a given token is a JavaScript array.
bool JsTokenIsArray(JsToken t, JsContextPtr cx);

// Utility function to tell if a given token is a JavaScript object. This
// function returns true for all JavaScript objects, including things like
// functions and Date objects. It returns false for all primitive values
// including null and undefined.
bool JsTokenIsObject(JsToken t);

// Utility function to check for the JavaScript values null and undefined. We
// usually treat these two identically to prevent confusion.
bool JsTokenIsNullOrUndefined(JsToken t);

bool BoolToJsToken(JsContextPtr context, bool value, JsScopedToken *out);
bool IntToJsToken(JsContextPtr context, int value, JsScopedToken *out);
bool StringToJsToken(JsContextPtr context, const char16 *value,
                     JsScopedToken *out);
bool DoubleToJsToken(JsContextPtr context, double value, JsScopedToken *out);
bool NullToJsToken(JsContextPtr context, JsScopedToken *out);
bool UndefinedToJsToken(JsContextPtr context, JsScopedToken *out);

//------------------------------------------------------------------------------
// JsRootedToken
//------------------------------------------------------------------------------

#if BROWSER_FF

// A JsToken that won't get GC'd out from under you.
class JsRootedToken {
 public:
  JsRootedToken(JsContextPtr context, JsToken token);
  ~JsRootedToken();

  JsToken token() const { return token_; }
  JsContextPtr context() const { return context_; }

 private:
  JsContextPtr context_;
  JsToken token_;
  DISALLOW_EVIL_CONSTRUCTORS(JsRootedToken);
};

#elif BROWSER_IE

// A JsToken that won't get GC'd out from under you.
// TODO(aa): This leaks for things like strings and arrays. We need to correctly
// handle lifetime. Also need to think of different requirements for when token
// is an input parameter vs a return value.
class JsRootedToken {
 public:
  JsRootedToken(JsContextPtr context, JsToken token);
  ~JsRootedToken();

  JsToken token() const { return token_; }
  JsContextPtr context() const { return NULL; }

 private:
  JsToken token_;
  DISALLOW_EVIL_CONSTRUCTORS(JsRootedToken);
};

#elif BROWSER_NPAPI

// A JsToken that won't get GC'd out from under you.
class JsRootedToken {
 public:
  JsRootedToken(JsContextPtr context, JsToken token);
  ~JsRootedToken();

  const JsScopedToken& token() const { return token_; }
  JsContextPtr context() const { return context_; }

 private:
  JsContextPtr context_;  // TODO(mpcomplete): figure out if this is necessary
  JsScopedToken token_;
  DISALLOW_EVIL_CONSTRUCTORS(JsRootedToken);
};

#endif

//------------------------------------------------------------------------------
// JsArray
//------------------------------------------------------------------------------

// JsArray and JsObject make use of a JsScopedToken, which is a token that
// releases any references it has when it goes out of scope.  This is necessary
// in IE and NPAPI, because getting an array element or object property
// increases the reference count, and we need a way to release that reference
// *after* we're done using the object.  In Firefox, JsToken == JsScopedToken
// because Firefox only gives us a weak pointer to the value.

class JsArray {
 public:
  JsArray();
  ~JsArray();

  bool SetArray(JsToken value, JsContextPtr context);

  bool GetLength(int *length) const;

  // use the same syntax as JsRootedToken
  const JsScopedToken &token() const { return array_; }
  const JsContextPtr &context() const { return js_context_; }

  // These methods return false on failure. They return true, and set out to
  // JSPARAM_UNDEFINED if the requested element does not exist.
  bool GetElement(int index, JsScopedToken *out) const;
  
  bool GetElementAsBool(int index, bool *out) const;
  bool GetElementAsInt(int index, int *out) const;
  bool GetElementAsDouble(int index, double *out) const;
  bool GetElementAsString(int index, std::string16 *out) const;
  bool GetElementAsArray(int index, JsArray *out) const;
  bool GetElementAsObject(int index, JsObject *out) const;
  bool GetElementAsFunction(int index, JsRootedCallback **out) const;

  // Method to get the type of an element. Returns JSPARAM_UNDEFINED if the
  // requested element does not exist.
  JsParamType GetElementType(int index) const;

  bool SetElement(int index, const JsScopedToken &value);
  bool SetElementBool(int index, bool value);
  bool SetElementInt(int index, int value);
  bool SetElementDouble(int index, double value);
  bool SetElementString(int index, const std::string16 &value);
  bool SetElementArray(int index, JsArray *value);
  bool SetElementObject(int index, JsObject *value);
  bool SetElementFunction(int index, JsRootedCallback *value);
  bool SetElementModule(int index, ModuleImplBaseClass *value);

 private:
  JsContextPtr js_context_;
  JsScopedToken array_;
  DISALLOW_EVIL_CONSTRUCTORS(JsArray);
};

//------------------------------------------------------------------------------
// JsObject
//------------------------------------------------------------------------------

class JsObject {
 public:
  JsObject();
  ~JsObject();

  bool SetObject(JsToken value, JsContextPtr context);

  // These methods return false on failure. They return true, and set out to
  // JSPARAM_UNDEFINED if the requested property does not exist.
  bool GetProperty(const std::string16 &name, JsScopedToken *value) const;

  bool GetPropertyAsBool(const std::string16 &name, bool *out) const;
  bool GetPropertyAsInt(const std::string16 &name, int *out) const;
  bool GetPropertyAsDouble(const std::string16 &name, double *out) const;
  bool GetPropertyAsString(const std::string16 &name, std::string16 *out) const;
  bool GetPropertyAsArray(const std::string16 &name, JsArray *out) const;
  bool GetPropertyAsObject(const std::string16 &name, JsObject *out) const;
  bool GetPropertyAsFunction(const std::string16 &name,
                             JsRootedCallback **out) const;

  // Method to get the type of a property. Returns JSPARAM_UNDEFINED if the
  // requested property does not exist.
  JsParamType GetPropertyType(const std::string16 &name) const;
  bool HasProperty(const std::string16 &name) const {
    return JSPARAM_UNDEFINED != GetPropertyType(name);
  }
  
  // GetPropertyNames fills the given vector with the (string) names of this
  // JsObject's properties.  There is no guarantee that it retrieves (or does
  // not retrieve) property names from the object's prototype, nor does it
  // rule anything in or out about property keys that aren't strings.  Also,
  // this only *appends* property names to the vector out.  In particular, it
  // does not reset the vector to be empty.
  bool GetPropertyNames(std::vector<std::string16> *out) const;

  // SetProperty*() overwrites the existing named property or adds a new one if
  // none exists.
  bool SetPropertyBool(const std::string16& name, bool value);
  bool SetPropertyInt(const std::string16 &name, int value);
  bool SetPropertyDouble(const std::string16& name, double value);
  bool SetPropertyString(const std::string16 &name, const std::string16 &value);
  bool SetPropertyArray(const std::string16& name, JsArray* value);
  bool SetPropertyObject(const std::string16& name, JsObject* value);
  bool SetPropertyFunction(const std::string16& name, JsRootedCallback* value);
  bool SetPropertyModule(const std::string16& name,
                                   ModuleImplBaseClass* value);
  bool SetProperty(const std::string16 &name, const JsToken &value);

  const JsScopedToken &token() const { return js_object_; }
  const JsContextPtr &context() const { return js_context_; }

 private:
  JsContextPtr js_context_;
  JsScopedToken js_object_;
  DISALLOW_EVIL_CONSTRUCTORS(JsObject);
};

//------------------------------------------------------------------------------
// ConvertJsParamToToken, JsCallContext
//------------------------------------------------------------------------------

// Given a JsParamToSend, extract it into a JsScopedToken.  Resulting token
// increases the reference count for objects.
// Returns true if successful. On failure, token is not modified.
// Failure could happen if the caller tried to return a numerical value that
// is outside the range of numbers representable by the particular JavaScript
// engine. For example, if that engine internally only uses int32s or doubles,
// then some int64 values are not representable.
bool ConvertJsParamToToken(const JsParamToSend &param,
                           JsContextPtr context, JsScopedToken *token);

// This class provides an interface for a property or method access on a native
// object from JavaScript.  It allows consumers to retrieve what arguments were
// passed in, and return a value or exception back to the caller.  Any native
// property or method handler should take an instance of this object as a
// parameter.
class JsCallContext {
 public:
  // Only browser-specific wrapper code should need to instantiate this object.
#if BROWSER_NPAPI
  JsCallContext(JsContextPtr js_context, NPObject *object,
                int argc, const JsToken *argv, JsToken *retval);
#elif BROWSER_IE
  JsCallContext(DISPPARAMS FAR *disp_params, VARIANT FAR *retval,
                EXCEPINFO FAR *excep_info);
#elif BROWSER_FF
  JsCallContext(JsContextPtr cx, JsRunnerInterface *js_runner,
                int argc, JsToken *argv, JsToken *retval);
#endif

  ~JsCallContext();

  // Get the arguments a JavaScript caller has passed into a scriptable method
  // of a native object.  Returns the number of arguments successfully read
  // (will bail at the first invalid argument).
  int GetArguments(int argc, JsArgument *argv);

  // As above, except that this function tries to extract all given parameters
  // and accepts null for optional arguments. Returns false if an exception
  // occurs and true otherwise.
  // TODO(andreip): post 0.4, replace the above JsCallContext::GetArguments
  // with this implementation and fix all the call sites.
  bool GetArguments2(int argc, JsArgument *argv);

  // Get the type of an argument that was passed in.
  JsParamType GetArgumentType(int i);

  // Sets the value to be returned to the calling JavaScript.
  //
  // The ModuleImplBaseClass* version should only be used when returning a
  // JSPARAM_COM_MODULE.  It exists because passing a derived class through a
  // void* and then casting to the base class is not safe - the compiler won't
  // be able to correctly adjust the pointer offset.
  //
  // The int version is for use with JSPARAM_NULL, to avoid conflicting with the
  // ModuleImplBaseClass version (works because NULL is defined as 0).
  void SetReturnValue(JsParamType type, const void *value_ptr);
  void SetReturnValue(JsParamType type, const ModuleImplBaseClass *value_ptr) {
    assert(type == JSPARAM_MODULE);
    SetReturnValue(type, reinterpret_cast<const void*>(value_ptr));
  }
  void SetReturnValue(JsParamType type, int) {
    assert(type == JSPARAM_NULL);
    SetReturnValue(type, reinterpret_cast<const void*>(NULL));
  }

  // Sets an exception to be thrown to the calling JavaScript.  Setting an
  // exception overrides any previous exception and any return values.
  void SetException(const std::string16 &message);

  JsContextPtr js_context() { return js_context_; }
  bool is_exception_set() { return is_exception_set_; }
  bool is_return_value_set() { return is_return_value_set_; }
#if BROWSER_FF
  JsRunnerInterface *js_runner() { return js_runner_; }
#endif

 private:
  int GetArgumentCount();
  const JsToken &GetArgument(int index);

  JsContextPtr js_context_;
  bool is_exception_set_;
  bool is_return_value_set_;
#if BROWSER_NPAPI
  NPObject *object_;
  int argc_;
  const JsToken *argv_;
  JsToken *retval_;
#elif BROWSER_IE
  DISPPARAMS FAR *disp_params_;
  VARIANT FAR *retval_;
  EXCEPINFO FAR *exception_info_;
#elif BROWSER_FF
  int argc_;
  JsToken *argv_;
  JsToken *retval_;
  JsRunnerInterface *js_runner_;
#endif

  DISALLOW_EVIL_CONSTRUCTORS(JsCallContext);
};

#endif  // GEARS_BASE_COMMON_JS_TYPES_H__
