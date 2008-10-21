// Copyright 2008, Google Inc.
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
//
// A JsToken is a platform-independent abstraction for whatever a JavaScript
// variable can hold, e.g. an integer, an array, a JavaScript object, or even
// a function.  However, a JsToken is tied to a specific instance of a
// JavaScript execution engine (i.e. an instance of JsRunnerInterface).
//
// A MarshaledJsToken is a "saved JsToken" that is not tied to a specific
// JsRunnerInterface, but can be "restored" to provide a JsToken for a given
// JsRunnerInterface, and importantly, be "restored" to a different
// JsRunnerInterface than the one it was originally "saved" from.
//
// An original JsToken and a "restored" JsToken (in the same or possibly a
// different JsRunnerInterface) is equivalent in the sense that one is a
// copy of the other, but modifying one JsToken (either in C++ or via a
// Worker's JavaScript) will not modify the other.  The implementation might
// share objects for efficiency (if such objects are thread-safe), but
// semantically, the "restored" JsToken is effectively a totally separate
// copy whose life cycle and behavior are independent of the original JsToken.
//
// Not all JsTokens can be marshaled.  For example, functions can not be
// marshaled, and JavaScript objects that represent DOM elements can not be
// meaningfully marshaled (although practically, this will most likely not
// fail, but instead produce a partial reflection of the original JavaScript
// object).  In fact, only basic JavaScript objects with String property keys
// and marshalable property values should be marshaled.  Behavior for any other
// sort of object is unspecified and likely to be different across browsers
// and platforms, and consequently should not be relied upon.
//
// An example of a "basic JavaScript object" is given by this bit of js:
// var marshalable = {
//   "foo": "bar",
//   "baz": [1, 2, 3]
// };
//
// A simple rule of thumb is that any valid JSON object is marshalable.
// However, some marshalable objects are not representable as pure JSON.
// Specifically, some Gears modules may be marshalable.
//
// Objects that (directly or indirectly) repeatedly refer to child objects
// will not maintain their structure when marshaled and unmarshaled.
// For example:
// var alsoMarshalable = [3.14159, marshalable, "hello", marshalable];
// will not, when marshaled and unmarshaled, have its 2nd and 4th elements
// be the same object (in the == sense), but instead be two separate copies.
//
// Furthermore, attempting to marshal a cyclic graph of JsTokens, such as the
// variable aa in the code snippet below, will fail - calling Marshal on such
// an object will return NULL.
// var aa = [1, 2];
// var bb = [7, aa];
// aa.push(bb);

#ifndef GEARS_BASE_COMMON_JS_MARSHAL_H__
#define GEARS_BASE_COMMON_JS_MARSHAL_H__

#include <map>
#include <vector>
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_types.h"

struct ModuleEnvironment;
class MarshaledModule;

// Marshaling is implemented by having a 1-1 correspondance between a JsToken
// (e.g. representing an integer or an object) and a MarshaledJsToken.
// Some JsToken's (and hence MarshaledJsToken's) can have children, e.g.
// Arrays and Objects (which are basically treated as key-value maps).
class MarshaledJsToken {
public:
  ~MarshaledJsToken();

  // This method returns NULL if the marshal fails (e.g. trying to marshal a
  // JavaScript function, or trying to marshal an object graph that contains
  // a cycle).  On failure, error_message_out will contain the error message.
  static MarshaledJsToken *Marshal(const JsToken &token,
                                   JsRunnerInterface *js_runner,
                                   JsContextPtr js_context,
                                   std::string16 *error_message_out);

  // Returns whether or not this MarshaledJsToken was successfully restored to
  // be a JsScopedToken (*out) inside the given ModuleEnvironment.
  bool Unmarshal(ModuleEnvironment *module_environment, JsScopedToken *out);

private:
  typedef std::vector<JsToken> JsTokenVector;

  MarshaledJsToken();

  static bool CausesCycle(JsRunnerInterface *js_runner,
                          const JsToken &token,
                          JsTokenVector *object_stack,
                          std::string16 *error_message_out);

  static MarshaledJsToken *Marshal(const JsToken &token,
                                   JsRunnerInterface *js_runner,
                                   JsContextPtr js_context,
                                   std::string16 *error_message_out,
                                   JsTokenVector *object_stack);

  bool InitializeFromObject(JsObject &object,
                            JsRunnerInterface *js_runner,
                            JsContextPtr js_context,
                            std::string16 *error_message_out,
                            JsTokenVector *object_stack);

  bool InitializeFromArray(JsArray &array,
                           JsRunnerInterface *js_runner,
                           JsContextPtr js_context,
                           std::string16 *error_message_out,
                           JsTokenVector *object_stack);

  JsParamType type_;
  union {
    bool bool_value;
    int int_value;
    double double_value;
    std::string16 *string_value;
    std::map<std::string16, MarshaledJsToken*> *object_value;
    std::vector<MarshaledJsToken*> *array_value;
    MarshaledModule *marshaled_module_value;
  } value_;

  DISALLOW_EVIL_CONSTRUCTORS(MarshaledJsToken);
};

#endif  // GEARS_BASE_COMMON_JS_MARSHAL_H__
