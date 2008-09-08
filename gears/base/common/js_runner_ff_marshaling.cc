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
// Architecture Summary:
// * To add a class, we create a 'prototype' JSObject, then attach a 'function'
//   JSObject for each class function (methods, and property getters/setters).
// * We use the same code (JsWrapperCaller) for all functions.  This code knows
//   how to convert params and return values between JavaScript and the
//   DispatcherInterface instance.
//
// Additional info:
// * 'jsval' is the wrapper the Firefox JS engine uses for all args and retvals.
// * A jsval can contain a JSObject*, an integer, or a number of other types.
// * A JSObject can represent an instance, a function, a class prototype, etc.
//
// TODO(cprince): figure out how to cleanup allocated JsWrapperDataFor* structs
// when the JSContext goes away.  Maybe can use things like Finalize properties?
//
// Similarly, where do we destroy:
// * New objects returned by Gears classes (For example, the ResultSet objects
//   returned by Database::Execute.)
// * JS_NewObject (and similar) return values, on Define*() failure
//   (Or maybe rely on JSContext cleanup -- at a higher level -- to handle it.)

// Include this first, since gecko_internal/jsapi.h needs XP_WIN or XP_UNIX
// defined.
#include <gecko_sdk/include/prtypes.h>

#include "gears/base/common/js_runner_ff_marshaling.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/leak_counter.h"
#include "gears/base/firefox/module_wrapper.h"


// The "reserved slot" in which we store our custom data for functions.
// (Non-function custom data can be stored using JS_SetPrivate.)
static const int kFunctionDataReservedSlotIndex = 0;

// Magic constants the JavaScript engine uses with the argv array.
static const int kArgvFunctionIndex = -2; // negative indices [sic]
static const int kArgvInstanceIndex = -1;


// Structs that record different custom data for the different JSObject types.
// The structs have a common header so we can differentiate generic JSObjects.
//
// These structures are saved as the private data on JSObjects.
// By sharing a common base class, we can cast the raw pointer
// stored on the JSObject to a JsWrapperDataHeader to determine the
// JsWrapperData type.
//
// There are three JsWrapperDataForXxx subclasses:
// * PROTO represents, for example, the GearsWorkerPool prototype.
// * INSTANCE represents, for example, an instance of a GearsWorkerPool.
// * FUNCTION represents, for example, the GearsWorkerPool.createWorker method.
//
// Cleaning up (i.e. resource de-allocation) is slightly different for each of
// these three types.
//
// A JsWrapperDataForInstance lives and dies with a particular instance in the
// JS engine. When a JS object that represents a Gears module is garbage
// collected, its finalizer is called, which is the FinalizeNative function
// below. In that function, we delete that JsWrapperDataForInstance.
//
// The other two subclasses (PROTO and FUNCTION) do not have a particular JS
// instance to base their lifecycle on, but have to live for at least as long
// as it is possible to create new JS objects for Gears modules. Thus, we
// create a JsRootedToken to represent the entire lifecycle of PROTOs and
// FUNCTIONs, and end that lifecycle during JsContextWrapper::CleanupRoots.
// Specifically, JsContextWrapper::CleanupRoots will unroot the js_rooted_token
// on all JsWrapperDataForProto and JsWrapperDataForFunction objects.
//
// Destroying the JsRootedToken for a PROTO object will call FinalizeNative on
// that PROTO, where we can delete the JsWrapperDataForProto. Spidermonkey does
// not provide a finalizer for FUNCTIONs, though, and instead we delete
// JsWrapperDataForFunction instances in the JsContextWrapper destructor.
//
// One last thing that we have to manage is the heap-allocated JSClass'es.
// Amongst other things, this is the structure that associates PROTOs and
// INSTANCEs with our custom finalizer: FinalizeNative. Accordingly, the
// JSClass has to live for longer than its PROTOs and INSTANCEs, and hence,
// for example, the PROTO does not have a scoped_ptr<JSClass>, since otherwise
// the JSClass might be destroyed before all its INSTANCEs are. We choose to
// hold the JSClass objects in a ref-counted SharedJsClasses object, which will
// delete the JSClass'es when there are no longer any references to them.

enum JsWrapperDataType {
  PROTO_JSOBJECT,
  INSTANCE_JSOBJECT,
  FUNCTION_JSOBJECT,
  UNKNOWN_JSOBJECT // do not use
};

struct JsWrapperDataHeader {
  JsWrapperDataType type;

  explicit JsWrapperDataHeader(JsWrapperDataType t) : type(t) {}
};

struct JsWrapperData {
  const JsWrapperDataHeader  header;
  JsWrapperData(JsWrapperDataType t) : header(t) {};
};

struct JsWrapperDataForProto : public JsWrapperData {
  JsContextWrapper                *js_context_wrapper;
  JSClass                         *js_class;
  scoped_ptr<JsRootedToken>       js_rooted_token;
  scoped_ptr<std::string>         alloc_name;
  scoped_refptr<SharedJsClasses>  shared_js_classes_;

  JsWrapperDataForProto() : JsWrapperData(PROTO_JSOBJECT) {
    LEAK_COUNTER_INCREMENT(JsWrapperDataForProto);
  }
  ~JsWrapperDataForProto() {
    LEAK_COUNTER_DECREMENT(JsWrapperDataForProto);
  }
};

struct JsWrapperDataForInstance : public JsWrapperData {
  JsContextWrapper                *js_context_wrapper;
  ModuleImplBaseClass             *module;
  scoped_refptr<SharedJsClasses>  shared_js_classes_;

  JsWrapperDataForInstance() : JsWrapperData(INSTANCE_JSOBJECT) {
    LEAK_COUNTER_INCREMENT(JsWrapperDataForInstance);
  }
  ~JsWrapperDataForInstance() {
    LEAK_COUNTER_DECREMENT(JsWrapperDataForInstance);
    if (module) {
      static_cast<ModuleWrapper *>(module->GetWrapper())->Destroy();
    }
  }
};

struct JsWrapperDataForFunction : public JsWrapperData {
  scoped_ptr<JsRootedToken>  js_rooted_token;
  DispatchId                 dispatch_id;
  int                        flags;

  JsWrapperDataForFunction() : JsWrapperData(FUNCTION_JSOBJECT) {
    LEAK_COUNTER_INCREMENT(JsWrapperDataForFunction);
  }
  ~JsWrapperDataForFunction() {
    LEAK_COUNTER_DECREMENT(JsWrapperDataForFunction);
  }
};


// Called when a JS object representing a Gears module prototype or a Gears
// module instance is cleaned up.
void FinalizeNative(JSContext *cx, JSObject *obj) {
  JsWrapperData *p = reinterpret_cast<JsWrapperData *>(JS_GetPrivate(cx, obj));
  if (!p)
    return;
  switch(p->header.type) {
    case PROTO_JSOBJECT:
      delete static_cast<JsWrapperDataForProto *>(p);
      break;
    case INSTANCE_JSOBJECT:
      delete static_cast<JsWrapperDataForInstance *>(p);
      break;
    default:
      assert(false);  // Should never reach this line.
      break;
  }
}


SharedJsClasses::SharedJsClasses() {
  LEAK_COUNTER_INCREMENT(SharedJsClasses);
}


SharedJsClasses::~SharedJsClasses() {
  LEAK_COUNTER_DECREMENT(SharedJsClasses);
  std::set<JSClass*>::iterator c;
  for (c = js_classes_.begin(); c != js_classes_.end(); ++c) {
    delete *c;
  }
}


bool SharedJsClasses::Contains(JSClass *js_class) {
  return js_classes_.find(js_class) != js_classes_.end();
}


void SharedJsClasses::Insert(JSClass *js_class) {
  js_classes_.insert(js_class);
}


JsContextWrapper::JsContextWrapper(JSContext *cx, JSObject *global_obj)
    : cx_(cx), global_obj_(global_obj),
      shared_js_classes_(new SharedJsClasses),
      cleanup_roots_has_been_called_(false) {
  LEAK_COUNTER_INCREMENT(JsContextWrapper);
}


JsContextWrapper::~JsContextWrapper() {
  LEAK_COUNTER_DECREMENT(JsContextWrapper);
  assert(cleanup_roots_has_been_called_);

  std::vector<JsWrapperDataForFunction *>::iterator f;
  for (f = function_wrappers_.begin(); f != function_wrappers_.end(); ++f) {
    delete *f;
  }
}


void JsContextWrapper::CleanupRoots() {
  assert(!cleanup_roots_has_been_called_);
  cleanup_roots_has_been_called_ = true;

  std::vector<JsWrapperDataForProto *>::iterator p;
  for (p = proto_wrappers_.begin(); p != proto_wrappers_.end(); ++p) {
    (*p)->js_rooted_token.reset();
  }

  std::vector<JsWrapperDataForFunction *>::iterator f;
  for (f = function_wrappers_.begin(); f != function_wrappers_.end(); ++f) {
    (*f)->js_rooted_token.reset();
  }
}


bool JsContextWrapper::CreateJsTokenForModule(ModuleImplBaseClass *module,
                                              JsToken *token_out) {
  // We require the name property to be set since we use it as the key for
  // caching created prototype objects.
  const std::string &module_name = module->get_module_name();

  JSObject *proto = NULL;
  JSClass *js_class = NULL;

  // Get the JSClass for this type of Module, or else create one if we've
  // never seen this class before.
  NameToProtoMap::iterator iter = name_to_proto_map_.find(module_name);
  if (iter != name_to_proto_map_.end()) {
    proto = iter->second;
    js_class = static_cast<JsWrapperDataForProto*>(JS_GetPrivate(cx_, proto))->
        js_class;

  } else {
    scoped_ptr<JsWrapperDataForProto> proto_data(new JsWrapperDataForProto);
    scoped_ptr<JSClass> alloc_js_class(new JSClass);

    proto = InitClass(module_name.c_str(), proto_data.get(),
                      alloc_js_class.get());
    js_class = alloc_js_class.get();

    if (!proto ||
        !AddAllFunctionsToPrototype(proto,
                                    module->GetWrapper()->GetDispatcher())) {
      return false;
    }

    // Save values, and transfer ownership.
    name_to_proto_map_[module_name] = proto;
    proto_wrappers_.push_back(proto_data.get());
    shared_js_classes_->Insert(alloc_js_class.release());
    JS_SetPrivate(cx_, proto, proto_data.release());
  }

  JS_BeginRequest(cx_);
  JSObject *instance_obj = JS_NewObject(cx_, js_class, proto, global_obj_);
  JS_EndRequest(cx_);
  if (!instance_obj) return false;

  scoped_ptr<JsWrapperDataForInstance> instance_data(
      new JsWrapperDataForInstance);
  instance_data->js_context_wrapper = this;
  instance_data->module = module;
  instance_data->shared_js_classes_.reset(shared_js_classes_.get());
  JS_SetPrivate(cx_, instance_obj, instance_data.release());

  *token_out = OBJECT_TO_JSVAL(instance_obj);
  return true;
}


JSObject *JsContextWrapper::InitClass(const char *class_name,
                                      JsWrapperDataForProto *proto_data,
                                      JSClass *js_class) {
  assert(class_name);
  // TODO(nigeltao): Do we really need class_name_copy (and hence alloc_name)?
  scoped_ptr<std::string> class_name_copy(new std::string(class_name));

  // Initialize js_class. The interesting fields are: name, flags and finalize.
  // TODO(cprince): do we need the JSCLASS_NEW_RESOLVE flag?
  js_class->name         = class_name_copy->c_str();
  js_class->flags        = JSCLASS_NEW_RESOLVE | JSCLASS_HAS_PRIVATE;
  js_class->addProperty  = JS_PropertyStub;
  js_class->delProperty  = JS_PropertyStub;
  js_class->getProperty  = JS_PropertyStub;
  js_class->setProperty  = JS_PropertyStub;
  js_class->enumerate    = JS_EnumerateStub;
  js_class->resolve      = JS_ResolveStub;
  js_class->convert      = JS_ConvertStub;
  js_class->finalize     = FinalizeNative;
  js_class->getObjectOps = 0;
  js_class->checkAccess  = 0;
  js_class->call         = 0;
  js_class->construct    = 0;
  js_class->xdrObject    = 0;
  js_class->hasInstance  = 0;
  js_class->mark         = 0;
  js_class->reserveSlots = 0;

  // add the class to the JSContext
  JS_BeginRequest(cx_);
  JSObject *result = JS_InitClass(cx_, global_obj_,
                                  NULL, // parent_proto
                                  js_class, // JSClass *
                                  NULL, // constructor
                                  0, // ctor_num_args
                                  NULL, NULL,  //   prototype   props/funcs
                                  NULL, NULL); // "constructor" props/funcs
  JS_EndRequest(cx_);
  if (!result) { return NULL; }

  // setup the JsWrapperDataForProto struct
  proto_data->js_context_wrapper = this;
  proto_data->js_class = js_class;
  proto_data->alloc_name.swap(class_name_copy); // take ownership
  proto_data->js_rooted_token.reset(
      new JsRootedToken(cx_, OBJECT_TO_JSVAL(result)));
  proto_data->shared_js_classes_.reset(shared_js_classes_.get());

  return result;
}


bool JsContextWrapper::AddAllFunctionsToPrototype(
                           JSObject *proto_obj,
                           DispatcherInterface *dispatcher) {
  DispatcherNameList::const_iterator members =
      dispatcher->GetMemberNames().begin();
  for (; members != dispatcher->GetMemberNames().end(); ++members) {
    DispatchId dispatch_id = members->second;
    bool has_getter = dispatcher->HasPropertyGetter(dispatch_id);
    bool has_setter = dispatcher->HasPropertySetter(dispatch_id);

    if (has_getter) {
      if (!AddFunctionToPrototype(proto_obj,
                                  members->first.c_str(), // member name
                                  true, false, // is_getter, is_setter
                                  dispatch_id))
        return false;
    }

    if (has_setter) {
      if (!AddFunctionToPrototype(proto_obj,
                                  members->first.c_str(), // member name
                                  false, true, // is_getter, is_setter
                                  dispatch_id))
        return false;
    }

    // If there is no getter or setter, this member must be a method.
    if (!has_getter && !has_setter) {
      if (!AddFunctionToPrototype(proto_obj,
                                  members->first.c_str(), // member name
                                  false, false, // is_getter, is_setter
                                  dispatch_id))
        return false;
    }
  }

  return true;
}


bool JsContextWrapper::AddFunctionToPrototype(
                           JSObject *proto_obj, const char *name,
                           bool is_getter, bool is_setter,
                           DispatchId dispatch_id) {
  // Create a JSFunction object for the property getter/setter or method.
  int newfunction_flags = 0;
  if (is_getter) {
    newfunction_flags = JSFUN_GETTER;
  } else if (is_setter) {
    newfunction_flags = JSFUN_SETTER;
  }
  
  JS_BeginRequest(cx_);
  JSFunction *function = JS_NewFunction(cx_,
                                        JsContextWrapper::JsWrapperCaller,
                                        0,
                                        newfunction_flags,
                                        proto_obj, // parent
                                        name);
  JS_EndRequest(cx_);
  JSObject *function_obj = JS_GetFunctionObject(function);

  // Save info about the function.
  scoped_ptr<JsWrapperDataForFunction> function_data(
                                           new JsWrapperDataForFunction);
  function_data->dispatch_id = dispatch_id;
  function_data->flags = newfunction_flags;
  function_data->js_rooted_token.reset(
      new JsRootedToken(cx_, OBJECT_TO_JSVAL(function_obj)));

  // Assume function is a method, and revise if it's a getter or setter.
  // FYI: xptinfo reports getters/setters separately (but with same name).
  JSPropertyOp getter = NULL;
  JSPropertyOp setter = NULL;
  jsval        method = OBJECT_TO_JSVAL(function_obj);
  uintN        function_flags = 0;

  if (is_getter) {
    getter = (JSPropertyOp) function_obj;
    method = OBJECT_TO_JSVAL(NULL);
    function_flags = (JSPROP_GETTER | JSPROP_SHARED);
    // TODO(cprince): need JSPROP_READONLY for no-setter attributes?
  } else if (is_setter) {
    setter = (JSPropertyOp) function_obj;
    method = OBJECT_TO_JSVAL(NULL);
    function_flags = (JSPROP_SETTER | JSPROP_SHARED);
  }

  // Note: JS_DefineProperty is written to handle adding a setter to a
  // previously defined getter with the same name.
  JS_BeginRequest(cx_);
  JSBool js_ok = JS_DefineProperty(cx_, proto_obj, name,
                                   method, // method
                                   getter, setter, // getter, setter
                                   function_flags);
  JS_EndRequest(cx_);
  if (!js_ok) { return false; }

  // succeeded; prevent scoped cleanup of allocations, and save the pointer
  //
  // We cannot use JS_SetPrivate() here because a function JSObject stores
  // its JSFunction pointer there (see js_LinkFunctionObject in jsfun.c).
  //
  // Instead, use reserved slots, which we DO have.  From js_FunctionClass in
  // jsfun.c: "Reserve two slots in all function objects for XPConnect."
  //
  // We must use PRIVATE_TO_JSVAL (only works on pointers!) to prevent the
  // garbage collector from touching any private data stored in JS 'slots'.
  assert(0 == (0x01 & reinterpret_cast<int>(function_data.get())));
  jsval pointer_jsval = PRIVATE_TO_JSVAL((jsval)function_data.get());
  function_wrappers_.push_back(function_data.release());
  assert(!JSVAL_IS_GCTHING(pointer_jsval));
  JS_BeginRequest(cx_);
  JS_SetReservedSlot(cx_, function_obj, kFunctionDataReservedSlotIndex,
                     pointer_jsval);
  JS_EndRequest(cx_);

  return true;
}


ModuleImplBaseClass *JsContextWrapper::GetModuleFromJsToken(
    const JsToken token) {
  // First, check that the JsToken represents a (JavaScript) object, and not
  // a JS int, for example.
  if (!JSVAL_IS_OBJECT(token)) {
    return NULL;
  }
  JSObject *obj = JSVAL_TO_OBJECT(token);

  // Next, check that the JSObject is for a Dispatcher-based module, and not
  // any other type of JSObject.  To do that, we get its JSClass, and check
  // that against those JSClasses we have previously seen (in this
  // JsContextWrapper) for Dispatcher-based modules.
  JSClass *js_class = JS_GET_CLASS(cx_, obj);
  if (!shared_js_classes_->Contains(js_class)) {
    return NULL;
  }

  // Now that we know that we have a Dispatcher-based module, we know that
  // the JS private data is in fact a JsWrapperDataForInstance*, which we can
  // crack open for its ModuleImplBaseClass*.
  JsWrapperDataForInstance *instance_data =
      reinterpret_cast<JsWrapperDataForInstance*>(JS_GetPrivate(cx_, obj));
  return instance_data->module;
}


// General-purpose wrapper to invoke any class function (method, or
// property getter/setter).
JSBool JsContextWrapper::JsWrapperCaller(JSContext *cx, JSObject *obj,
                                         uintN argc, jsval *argv,
                                         jsval *js_retval) {
  // Gather data regarding the function and instance being called.
  JSObject *function_obj = JSVAL_TO_OBJECT(argv[kArgvFunctionIndex]);
  assert(function_obj);

  JSObject *instance_obj = JSVAL_TO_OBJECT(argv[kArgvInstanceIndex]);
  assert(instance_obj);


  JsWrapperDataForFunction *function_data;
  jsval function_data_jsval;
  JS_BeginRequest(cx);
  JS_GetReservedSlot(cx, function_obj,
                     kFunctionDataReservedSlotIndex,
                     &function_data_jsval);
  JS_EndRequest(cx);
  function_data = static_cast<JsWrapperDataForFunction *>(
                      JSVAL_TO_PRIVATE(function_data_jsval));
  assert(function_data);
  assert(function_data->header.type == FUNCTION_JSOBJECT);

  JsWrapperDataForInstance *instance_data =
      static_cast<JsWrapperDataForInstance*>(JS_GetPrivate(cx, instance_obj));
  assert(instance_data);
  assert(instance_data->header.type == INSTANCE_JSOBJECT);
  assert(instance_data->module);
  assert(function_data->dispatch_id);

  ModuleWrapperBaseClass *module_wrapper =
      instance_data->module->GetWrapper();
  JsCallContext call_context(cx, instance_data->module->GetJsRunner(),
                             argc, argv, js_retval);

  if (function_data->flags == JSFUN_GETTER) {
    if (!module_wrapper->GetDispatcher()->GetProperty(
                                  function_data->dispatch_id,
                                  &call_context)) {
      call_context.SetException(
          STRING16(L"Property not found or not getter."));
      return JS_FALSE;
    }
  } else if (function_data->flags == JSFUN_SETTER) {
    if (!module_wrapper->GetDispatcher()->SetProperty(
                                  function_data->dispatch_id,
                                  &call_context)) {
      call_context.SetException(
          STRING16(L"Property not found or not setter."));
      return JS_FALSE;
    }
  } else {
    if (!module_wrapper->GetDispatcher()->CallMethod(
                                  function_data->dispatch_id,
                                  &call_context)) {
      call_context.SetException(
          STRING16(L"Method not found."));
      return JS_FALSE;
    }
  }

  return !call_context.is_exception_set() ? JS_TRUE : JS_FALSE;
}
