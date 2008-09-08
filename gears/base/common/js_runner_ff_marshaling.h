// Copyright 2006, Google Inc.
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
// Handles interaction between JavaScript and C++ in Firefox. You can think
// of this as the equivalent of XPConnect, except that Gears C++ modules are
// not XPCOM objects, but instead have a DispatcherInterface instance.

#ifndef GEARS_BASE_COMMON_JS_RUNNER_FF_MARSHALING_H__
#define GEARS_BASE_COMMON_JS_RUNNER_FF_MARSHALING_H__

#include <set>
#include <vector>

#include <gecko_internal/jsapi.h>
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/scoped_refptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

typedef std::map<const std::string, JSObject*> NameToProtoMap;
struct JsWrapperDataForProto;
struct JsWrapperDataForFunction;


// In Spidermonkey, each JSObject has a JSClass, and Gears allocates its own
// JSClass objects, since we don't use the thread-unsafe XPConnect. Thus, we
// need something to be responsible for deleting those JSClass objects, but
// we can't delete them until all three criteria below are met:
// (1) there are no more instances of that class,
// (2) the prototype for that class is deleted, and
// (3) there is no possibility of creating further instances of that class.
//
// Accordingly, this SharedJsClasses, declared below, is a ref-counted object,
// and references are held by (1) each instance (via JsWrapperDataForInstance,
// defined in the .cc file), (2) each prototype (via JsWrapperDataForProto,
// similarly), and (3) the JsContextWrapper.
//
// This is not a global object, but a per-JSContext object. For example, for
// a page with a main thread and three worker threads, there will be four
// SharedJsClasses objects. Or, if there are two Gears-enabled web pages, but
// neither of them have spawned any worker threads, then there will be two
// SharedJsClasses objects.
class SharedJsClasses : public RefCounted {
 public:
  SharedJsClasses();

  bool Contains(JSClass *js_class);

  // Takes ownership of the given JSClass*.
  // Insert has the same semantics as a std::set, in that Insert'ing something
  // twice is idempotent.
  void Insert(JSClass *js_class);

 protected:
  virtual ~SharedJsClasses();

 private:
  std::set<JSClass*> js_classes_;
  DISALLOW_EVIL_CONSTRUCTORS(SharedJsClasses);
};


// TODO(nigeltao): pick a better name than JsContextWrapper.
class JsContextWrapper {
 public:
  JsContextWrapper(JSContext *cx, JSObject *global_obj);
  ~JsContextWrapper();

  // CleanupRoots should be called before destroying this JsContextWrapper.
  void CleanupRoots();

  // Creates a new JavaScript object to represent a Gears module in the JS
  // engine.
  bool CreateJsTokenForModule(ModuleImplBaseClass *module, JsToken *token_out);

  ModuleImplBaseClass *GetModuleFromJsToken(const JsToken token);

 private:
  // Initializes a new class with the given name. If successful, returns a
  // JSObject (i.e. a GC'able thing) that represents the glue between a
  // C++ object and a JS object. The returned value is placed inside a
  // JsRootedToken by the callee, so the caller does not need to manage its
  // lifetime. On failure, returns NULL.
  JSObject *InitClass(const char *class_name,
                      JsWrapperDataForProto *proto_data,
                      JSClass *js_class);

  // Populate a JS prototype based on all the methods in a C++ class.
  bool AddAllFunctionsToPrototype(JSObject *proto_obj,
                                  DispatcherInterface *dispatcher);
  bool AddFunctionToPrototype(JSObject *proto_obj, const char *name,
                              bool is_getter, bool is_setter,
                              DispatchId dispatch_id);

  static JSBool JsWrapperCaller(JSContext *cx, JSObject *obj,
                                uintN argc, jsval *argv, jsval *retval);

  JSContext *cx_;
  JSObject  *global_obj_;

  // Map from module name to prototype. We use this so that we create only one
  // prototype for each module type.
  NameToProtoMap name_to_proto_map_;

  // The elements in these two vectors are "owned", meaning that the
  // JsContextWrapper is responsible for resetting their js_rooted_token
  // during CleanupRoots().
  std::vector<JsWrapperDataForProto *> proto_wrappers_;
  std::vector<JsWrapperDataForFunction *> function_wrappers_;

  scoped_refptr<SharedJsClasses> shared_js_classes_;
  bool cleanup_roots_has_been_called_;

  DISALLOW_EVIL_CONSTRUCTORS(JsContextWrapper);
};


#endif // GEARS_BASE_COMMON_JS_RUNNER_FF_MARSHALING_H__
