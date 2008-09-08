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

#include <assert.h>
#include <map>
#include <set>
#include <gecko_sdk/include/nspr.h> // for PR_*
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsIDOMEventTarget.h>
#include <gecko_sdk/include/nsIURI.h>
#include <gecko_internal/jsapi.h>
#include <gecko_internal/nsIDOMWindowInternal.h>
#include <gecko_internal/nsIJSContextStack.h>
#include <gecko_internal/nsIPrincipal.h>
#include <gecko_internal/nsIScriptContext.h>
#include <gecko_internal/nsIScriptGlobalObject.h>
#include <gecko_internal/nsIScriptObjectPrincipal.h>
#include <gecko_internal/nsITimer.h>
#include <gecko_internal/nsIXPConnect.h>

#if BROWSER_FF3
#include <gecko_sdk/include/nsIArray.h>
#include <gecko_sdk/include/nsIMutableArray.h>
#include <gecko_internal/nsDOMJSUtils.h>
#include <gecko_internal/nsIJSContextStack.h>
#else
#include <gecko_internal/nsITimerInternal.h>
#endif
#include "gears/base/common/js_runner.h"

#include "gears/base/common/basictypes.h" // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/exception_handler.h"
#include "gears/base/common/html_event_monitor.h"
#include "gears/base/common/js_runner_ff_marshaling.h"
#include "gears/base/common/js_runner_utils.h"  // For ThrowGlobalErrorImpl()
#include "gears/base/common/leak_counter.h"
#include "gears/base/common/scoped_token.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/firefox/dom_utils.h"

static const int kGarbageCollectionIntervalMsec = 2000;

// Local helper function.
static JsObject* JsvalToNewJsObject(const jsval &val, JsContextPtr context,
                                    bool dump_on_error);

// Internal base class used to share some code between DocumentJsRunner and
// JsRunner. Do not override these methods from JsRunner or DocumentJsRunner.
// Either share the code here, or move it to those two classes if it's
// different.
class JsRunnerBase : public JsRunnerInterface {
 public:
  JsRunnerBase() : alloc_js_wrapper_(NULL), js_engine_context_(NULL) {}

  virtual ~JsRunnerBase() {
    for (int i = 0; i < MAX_JSEVENTS; i++) {
      assert(0 == event_handlers_[i].size());
    }
  }

  JsContextPtr GetContext() {
    return js_engine_context_;
  }

  virtual bool Eval(const std::string16 &full_script, jsval *return_value) = 0;

  bool Eval(const std::string16 &full_script) {
    jsval return_value;
    return Eval(full_script, &return_value);
  }

  JsObject *NewObject(bool dump_on_error = false) {
    return NewObjectWithArguments("Object", 0, NULL, dump_on_error);
  }

  JsObject *NewError(const std::string16 &message,
                     bool dump_on_error = false) {
    JsParamToSend argv[] = { {JSPARAM_STRING16, &message} };
    return NewObjectWithArguments("Error", ARRAYSIZE(argv), argv,
                                  dump_on_error);
  }

  // Using JS_CallFunction for Date (and String) do not work for the reasons
  // described in NewObjectWithArguments. The object returned when we query the
  // global context for the these properties is a function, not a constructor.
  // The return value when we call this function is therefore a string, not a
  // new object. Furthermore, arguments passed to JS_CallFunction for these
  // types are ignored.
  JsObject *NewDate(int64 milliseconds_since_epoch) {
    jsval val;
    Eval(STRING16(L"new Date(") +
         Integer64ToString16(milliseconds_since_epoch) +
         STRING16(L")"),
         &val);
    if (!js_engine_context_) {
      LOG(("Could not get JavaScript engine context."));
      return NULL;
    }
    return JsvalToNewJsObject(val, js_engine_context_, false);
  }

  JsArray* NewArray() {
    JS_BeginRequest(GetContext());
    JSObject* array_object = JS_NewArrayObject(GetContext(), 0, NULL);
    JS_EndRequest(GetContext());
    if (!array_object)
      return NULL;

    scoped_ptr<JsArray> js_array(new JsArray());
    if (!js_array.get())
      return NULL;

    jsval array = OBJECT_TO_JSVAL(array_object);
    if (!js_array->SetArray(array, GetContext()))
      return NULL;

    return js_array.release();
  }

  bool ConvertJsObjectToDate(JsObject *obj,
                             int64 *milliseconds_since_epoch) {
    assert(obj);
    assert(milliseconds_since_epoch);

    JSObject *js_object = JSVAL_TO_OBJECT(obj->token());
    jsval func_val = JSVAL_VOID;
    JSObject *objp = NULL;
    JSBool result = JS_GetMethod(js_engine_context_, js_object,
                                 "getTime", &objp, &func_val);
    if (!result || JSVAL_IS_VOID(func_val)) {
      return false;
    }

    jsval ret_val = JSVAL_VOID;
    result = JS_CallFunctionValue(js_engine_context_, js_object, func_val,
                                  0, NULL, &ret_val);
    if (!result) {
      return false;
    }

    jsdouble td;
    if (!JS_ValueToNumber(js_engine_context_, ret_val, &td)) {
      return false;
    }
    LL_D2L(*((int64*)milliseconds_since_epoch), td);

    return true;
  }

  bool CreateJsTokenForModule(ModuleImplBaseClass *module, JsToken *token_out) {
    return alloc_js_wrapper_->CreateJsTokenForModule(module, token_out);
  }

  bool GetModuleFromJsToken(JsToken token, ModuleImplBaseClass **module_out) {
    ModuleImplBaseClass *module =
        alloc_js_wrapper_->GetModuleFromJsToken(token);
    if (module) {
      *module_out = module;
      return true;
    }
    return false;
  }

  virtual bool InvokeCallbackSpecialized(
                   const JsRootedCallback *callback, int argc, jsval *argv,
                   JsRootedToken **optional_alloc_retval) = 0;

  bool InvokeCallback(const JsRootedCallback *callback,
                      int argc, JsParamToSend *argv,
                      JsRootedToken **optional_alloc_retval) {
    assert(callback && (!argc || argv));

    if (JsTokenIsNullOrUndefined(callback->token())) { return false; }

    // Setup argument array.
    scoped_array<jsval> js_engine_argv(new jsval[argc]);
    for (int i = 0; i < argc; ++i)
      ConvertJsParamToToken(argv[i], callback->context(), &js_engine_argv[i]);

    // Invoke the method.
    return InvokeCallbackSpecialized(callback, argc, js_engine_argv.get(),
                                     optional_alloc_retval);
  }

  // Add the provided handler to the notification list for the specified event.
  virtual bool AddEventHandler(JsEventType event_type,
                               JsEventHandlerInterface *handler) {
    assert(event_type >= 0 && event_type < MAX_JSEVENTS);

    event_handlers_[event_type].insert(handler);
    return true;
  }

  // Remove the provided handler from the notification list for the specified
  // event.
  virtual bool RemoveEventHandler(JsEventType event_type,
                                  JsEventHandlerInterface *handler) {
    assert(event_type >= 0 && event_type < MAX_JSEVENTS);

    event_handlers_[event_type].erase(handler);
    return true;
  }

#ifdef DEBUG
  void ForceGC() {
    if (js_engine_context_) {
      JS_GC(js_engine_context_);
    }
  }
#endif

  // This function and others (AddEventHandler, RemoveEventHandler etc) do not
  // conatin any browser-specific code. They should be implemented in a new
  // class 'JsRunnerCommon', which inherits from JsRunnerInterface.
  virtual void ThrowGlobalError(const std::string16 &message) {
    ThrowGlobalErrorImpl(this, message);
  }

 protected:
  // Alert all monitors that an event has occured.
  void SendEvent(JsEventType event_type) {
    assert(event_type >= 0 && event_type < MAX_JSEVENTS);

    // Make a copy of the list of listeners, in case they change during the
    // alert phase.
    std::vector<JsEventHandlerInterface *> monitors;
    monitors.insert(monitors.end(),
                    event_handlers_[event_type].begin(),
                    event_handlers_[event_type].end());

    std::vector<JsEventHandlerInterface *>::iterator monitor;
    for (monitor = monitors.begin();
         monitor != monitors.end();
         ++monitor) {
      // Check that the listener hasn't been removed.  This can occur if a
      // listener removes another listener from the list.
      if (event_handlers_[event_type].find(*monitor) !=
                                     event_handlers_[event_type].end()) {
        (*monitor)->HandleEvent(event_type);
      }
    }
  }

  // Not using scoped_ptr even though it is possible because the cleanup code
  // for JsRunner is tricky and would rather be explicit about the order things
  // get torn down.
  JsContextWrapper *alloc_js_wrapper_;
  JSContext *js_engine_context_;
  
 private:
  JsObject *NewObjectWithArguments(const std::string &ctor_string,
                                   int argc, JsParamToSend *argv,
                                   bool dump_on_error) {
    assert(!argc || argv);
    if (!js_engine_context_) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      LOG(("Could not get JavaScript engine context."));
      return NULL;
    }

    JSObject *global_object = JS_GetGlobalObject(js_engine_context_);
    if (!global_object) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      LOG(("Could not get global object from script engine."));
      return NULL;
    }

    JsRequest request(js_engine_context_);

    jsval val = INT_TO_JSVAL(0);
    JSBool result = JS_GetProperty(js_engine_context_, global_object,
                                   ctor_string.c_str(), &val);
    if (!result) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      LOG(("Could not get constructor property from global object."));
      return NULL;
    }

    JSFunction *ctor = JS_ValueToFunction(js_engine_context_, val);
    if (!ctor) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      LOG(("Could not convert constructor property to function."));
      return NULL;
    }

    // Form the argument array.
    scoped_array<jsval> js_engine_argv(new jsval[argc]);
    for (int i = 0; i < argc; ++i) {
      ConvertJsParamToToken(argv[i], js_engine_context_, &js_engine_argv[i]);
    }

    // NOTE: We are calling the specified function here as a regular function,
    // not as a constructor. I could not find a way to call a function as a
    // constructor using JSAPI other than JS_ConstructObject which takes
    // arguments I don't know how to provide. Ideally, there would be something
    // like DISPATCH_CONSTRUCT in IE.
    //
    // This is OK for the built-in constructors that we want to call (such as
    // "Error", "Object", etc) because those objects are specified to behave as
    // constructors even without the 'new' keyword.
    //
    // For more information, see:
    // * ECMAScript spec section 15.2.1, 15.3.1, 15.4.1, etc.
    // * DISPATCH_CONSTRUCT:
    //     http://msdn2.microsoft.com/en-us/library/asd22sd4.aspx
    result = JS_CallFunction(js_engine_context_, global_object, ctor, argc,
                             js_engine_argv.get(), &val);
    if (!result) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      LOG(("Could not call constructor function."));
      return NULL;
    }

    return JsvalToNewJsObject(val, js_engine_context_, dump_on_error);
  }

  std::set<JsEventHandlerInterface *> event_handlers_[MAX_JSEVENTS];

  DISALLOW_EVIL_CONSTRUCTORS(JsRunnerBase);
};


class JsRunner : public JsRunnerBase {
 public:
  JsRunner(JSRuntime *js_runtime) : error_handler_(NULL), global_obj_(NULL),
               js_runtime_(js_runtime), js_script_(NULL) {
    LEAK_COUNTER_INCREMENT(JsRunner);
    // TODO(aa): Consider moving initialization of JsRunners out since there is
    // no way to detect errors in ctors.
    if (!InitJavaScriptEngine())
      return;

    // This creates a timer to run the garbage collector on a repeating
    // interval, which is what Firefox does in
    // source/dom/src/base/nsJSEnvironment.cpp.
    nsresult result;
    gc_timer_ = do_CreateInstance("@mozilla.org/timer;1", &result);

    if (NS_SUCCEEDED(result)) {
#if BROWSER_FF2
      // Turning off idle causes the callback to be invoked in this thread,
      // instead of in the Timer idle thread.
      nsCOMPtr<nsITimerInternal> timer_internal(do_QueryInterface(gc_timer_));
      timer_internal->SetIdle(false);
#endif

      // Start the timer
      gc_timer_->InitWithFuncCallback(GarbageCollectionCallback,
                                      js_engine_context_,
                                      kGarbageCollectionIntervalMsec,
                                      nsITimer::TYPE_REPEATING_SLACK);
    }
  }

  ~JsRunner();

  void OnModuleEnvironmentAttach() {
    // No-op. A JsRunner is not self-deleting, unlike a DocumentJsRunner.
  }

  void OnModuleEnvironmentDetach() {
    // No-op. A JsRunner is not self-deleting, unlike a DocumentJsRunner.
  }

  bool AddGlobal(const std::string16 &name, ModuleImplBaseClass *object);
  bool Start(const std::string16 &full_script);
  bool Stop();
  virtual bool Eval(const std::string16 &full_script, jsval *return_value);
  void SetErrorHandler(JsErrorHandlerInterface *handler) {
    error_handler_ = handler;
  }
  bool InvokeCallbackSpecialized(const JsRootedCallback *callback,
                                 int argc, jsval *argv,
                                 JsRootedCallback **optional_alloc_retval);

 private:
  bool InitJavaScriptEngine();

  static void GarbageCollectionCallback(nsITimer *timer, void *context);
  static void JS_DLL_CALLBACK JsErrorHandler(JSContext *cx, const char *message,
                                             JSErrorReport *report);

  JsErrorHandlerInterface *error_handler_;
  JSObject *global_obj_;
  JSRuntime *js_runtime_;
  JSScript *js_script_;
  scoped_ptr<JsRootedToken> js_script_root_;
  nsCOMPtr<nsITimer> gc_timer_;

  DISALLOW_EVIL_CONSTRUCTORS(JsRunner);
};

void JsRunner::GarbageCollectionCallback(nsITimer *timer,
                                                 void *context) {
  JSContext *cx = reinterpret_cast<JSContext *>(context);
  JS_GC(cx);
}

void JS_DLL_CALLBACK JsRunner::JsErrorHandler(JSContext *cx,
                                              const char *message,
                                              JSErrorReport *report) {
  JsRunner *js_runner = static_cast<JsRunner*>(JS_GetContextPrivate(cx));
  if (js_runner && js_runner->error_handler_ && report) {
    JsErrorInfo error_info;
    error_info.line = report->lineno + 1; // Reported lines start at zero.

    // The error message can either be in the separate *message param or in
    // *report->ucmessage. For example, running the following JS in a worker
    // causes the separate message param to get used:
    //   throw new Error("foo")
    // Other errors cause the report->ucmessage property to get used.
    //
    // Mozilla also does this, see:
    // http://lxr.mozilla.org/mozilla1.8.0/source/dom/src/base/nsJSEnvironment.cpp#163
    if (report->ucmessage) {
      error_info.message = reinterpret_cast<const char16 *>(report->ucmessage);
    } else if (message) {
      std::string16 message_str;
      if (UTF8ToString16(message, &message_str)) {
        error_info.message = message_str;
      }
    }

    js_runner->error_handler_->HandleError(error_info);
  }
}

JsRunner::~JsRunner() {
  LEAK_COUNTER_DECREMENT(JsRunner);
  // Alert modules that the engine is unloading.
  SendEvent(JSEVENT_UNLOAD);

  if (gc_timer_) {
    // Stop garbage collection now.
    gc_timer_->Cancel();
    gc_timer_ = NULL;
  }

  // We need to remove the roots now, because they will be referencing an
  // invalid context if we wait for the destructor.
  if (alloc_js_wrapper_)
    alloc_js_wrapper_->CleanupRoots();

  // Reset the scoped_ptr to unroot the script.  This needs to be done before
  // we destroy the context and runtime, so we can't wait for the destructor.
  js_script_root_.reset(NULL);

  if (js_engine_context_) {
    JS_DestroyContext(js_engine_context_);
  }

  // This has to occur after the context has been destroyed,
  // because it maintains data structures that the JS engine requires.
  // Specifically, any of the JSObjects stored in the JsWrapperData and the
  // global_obj_ need to exist as long as the object in the JS Engine which
  // they are linked to.
  delete alloc_js_wrapper_;

  // We do not clean up the JSRuntime because it is owned by the creator of the
  // JsRunner.
}

typedef DECLARE_SCOPED_TRAITS(JSContext*, JS_DestroyContext, NULL)
    JSContextTraits;
typedef scoped_token<JSContext*, JSContextTraits> scoped_jscontext_ptr;

bool JsRunner::InitJavaScriptEngine() {
  JSBool js_ok;

  // To cleanup after failures we use scoped objects to manage everything that
  // should be destroyed.  On success we take ownership to avoid cleanup.

  // These structs are static because they must live for duration of JS engine.
  // SpiderMonkey README also suggests using static for one-off objects.
  static JSClass global_class = {
      "Global", 0, // name, flags
      JS_PropertyStub, JS_PropertyStub,  // defineProperty, deleteProperty
      JS_PropertyStub, JS_PropertyStub, // getProperty, setProperty
      JS_EnumerateStub, JS_ResolveStub, // enum, resolve
      JS_ConvertStub, JS_FinalizeStub // convert, finalize
  };


  //
  // Instantiate a JavaScript engine
  //

  if (!js_runtime_) {
    ExceptionManager::ReportAndContinue();
    LOG(("Maximum thread count reached."));
    return false;
  }

  const int kContextStackChunkSize = 1024; // Firefox often uses 1024;
                                           // also see js/src/readme.html

  scoped_jscontext_ptr cx(JS_NewContext(js_runtime_, kContextStackChunkSize));
  if (!cx.get()) { return false; }
  // VAROBJFIX is recommended in /mozilla/js/src/jsapi.h
  JS_SetOptions(cx.get(), JS_GetOptions(cx.get()) | JSOPTION_VAROBJFIX);

  // JS_SetErrorReporter takes a static callback, so we need
  // JS_SetContextPrivate to later save the error in a per-worker location
  JS_SetErrorReporter(cx.get(), JsErrorHandler);
  JS_SetContextPrivate(cx.get(), static_cast<void*>(this));
#ifdef DEBUG
  // must set this here to allow workerPool.forceGC() during child init
  js_engine_context_ = cx.get();
#endif

  JsRequest request(cx.get());
  global_obj_ = JS_NewObject(cx.get(), &global_class, 0, 0);

  if (!global_obj_) { return false; }
  js_ok = JS_InitStandardClasses(cx.get(), global_obj_);
  if (!js_ok) { return false; }
  // Note: an alternative is to lazily define the "standard classes" (which
  // include things like eval).  To do that, change JS_InitStandardClasses
  // to JS_SetGlobalObject, and add handlers for Enumerate and Resolve in
  // global_class.  See /mozilla/js/src/js.c for sample code.

  //
  // Define classes in the JSContext
  //

  // first need to create a JsWrapperManager for this thread
  scoped_ptr<JsContextWrapper> js_wrapper(new JsContextWrapper(cx.get(),
                                                               global_obj_));

  js_engine_context_ = cx.release();
  alloc_js_wrapper_ = js_wrapper.release();

#ifdef DEBUG
  // Do it here to trigger potential GC bugs in our code.
  JS_GC(js_engine_context_);
#endif

  return true;
}

bool JsRunner::AddGlobal(const std::string16 &name,
                         ModuleImplBaseClass *object) {
  JsRequest request(js_engine_context_);
  return JS_TRUE == JS_DefineUCProperty(
      js_engine_context_, global_obj_,
      reinterpret_cast<const jschar *>(name.c_str()),
      name.length(),
      object->GetWrapperToken(),
      NULL, NULL, // getter, setter
      JSPROP_ENUMERATE);
}

bool JsRunner::Start(const std::string16 &full_script) {
  //
  // Add script code to the engine instance
  //

  JsRequest request(js_engine_context_);

  uintN line_number_start = 0;
  js_script_ = JS_CompileUCScript(
                       js_engine_context_, global_obj_,
                       reinterpret_cast<const jschar *>(full_script.c_str()),
                       full_script.length(),
                       "script", line_number_start);
  if (!js_script_) { return false; }

  // we must root any script returned by JS_Compile* (see jsapi.h)
  JSObject *compiled_script_obj = JS_NewScriptObject(js_engine_context_,
                                                     js_script_);
  if (!compiled_script_obj) { return false; }
  js_script_root_.reset(new JsRootedToken(
                                js_engine_context_,
                                OBJECT_TO_JSVAL(compiled_script_obj)));


  //
  // Start the engine running
  //

  jsval return_string;
  JSBool js_ok = JS_ExecuteScript(js_engine_context_, global_obj_,
                                  js_script_, &return_string);
  if (!js_ok) { return false; }

  return true;
}

bool JsRunner::Stop() {
  // TODO(zork): Implement
  return false;
}

bool JsRunner::Eval(const std::string16 &script, jsval *return_value) {
  assert(return_value);
  JSObject *object = JS_GetGlobalObject(js_engine_context_);

  JsRequest request(js_engine_context_);
  uintN line_number_start = 0;
  JSBool js_ok = JS_EvaluateUCScript(
                       js_engine_context_,
                       object,
                       reinterpret_cast<const jschar *>(script.c_str()),
                       script.length(),
                       "script", line_number_start,
                       return_value);
  if (!js_ok) { return false; }
  return true;
}

bool JsRunner::InvokeCallbackSpecialized(
                   const JsRootedCallback *callback, int argc, jsval *argv,
                   JsRootedToken **optional_alloc_retval) {
  JsRequest request(js_engine_context_);

  jsval retval;
  JSBool result = JS_CallFunctionValue(
                      callback->context(),
                      JS_GetGlobalObject(callback->context()),
                      callback->token(), argc, argv, &retval);

  if (result == JS_FALSE) { return false; }

  if (optional_alloc_retval) {
    // Note: A valid jsval is returned no matter what the javascript function
    // returns. If the javascript function returns nothing, or explicitly
    // returns <undefined>, the the jsval will be JSVAL_IS_VOID. If the
    // javascript function returns <null>, then the jsval will be JSVAL_IS_NULL.
    // Always returning a JsRootedToken should allow us to coerce these values
    // to other types correctly in the future.
    *optional_alloc_retval = new JsRootedToken(js_engine_context_, retval);
  }

  return true;
}

// Provides the same interface as JsRunner, but for the normal JavaScript engine
// that runs in HTML pages.
class DocumentJsRunner : public JsRunnerBase {
 public:
  DocumentJsRunner(JsContextPtr context)
      : is_module_environment_detached_(true),
        is_unloaded_(true) {
    LEAK_COUNTER_INCREMENT(DocumentJsRunner);
    js_engine_context_ = context;
    alloc_js_wrapper_ = new JsContextWrapper(context,
                                             JS_GetGlobalObject(context));
  }

  ~DocumentJsRunner() {
    LEAK_COUNTER_DECREMENT(DocumentJsRunner);
    assert(is_module_environment_detached_ && is_unloaded_);
    if (alloc_js_wrapper_) {
      alloc_js_wrapper_->CleanupRoots();
      delete alloc_js_wrapper_;
    }
  }

  void OnModuleEnvironmentAttach() {
    is_module_environment_detached_ = false;
  }

  void OnModuleEnvironmentDetach() {
    is_module_environment_detached_ = true;
    DeleteIfNoLongerRelevant();
  }

  bool AddGlobal(const std::string16 &name, ModuleImplBaseClass *object) {
    // TODO(zork): Add this functionality to DocumentJsRunner.
    return false;
  }
  void SetErrorHandler(JsErrorHandlerInterface *handler) {
    assert(false); // This should not be called on DocumentJsRunner.
  }
  bool Start(const std::string16 &full_script) {
    assert(false); // This should not be called on DocumentJsRunner.
    return false;
  }
  bool Stop() {
    assert(false); // This should not be called on DocumentJsRunner.
    return false;
  }
  virtual bool Eval(const std::string16 &full_script, jsval *return_value);
  bool InvokeCallbackSpecialized(const JsRootedCallback *callback,
                                 int argc, jsval *argv,
                                 JsRootedToken **optional_alloc_retval);
  bool ListenForUnloadEvent();

 private:
  static void HandleEventUnload(void *user_param);  // Callback for 'onunload'

  void DeleteIfNoLongerRelevant() {
    if (is_module_environment_detached_ && is_unloaded_) {
      delete this;
    }
  }

  scoped_ptr<HtmlEventMonitor> unload_monitor_;  // For 'onunload' notifications
  bool is_module_environment_detached_;
  bool is_unloaded_;
  DISALLOW_EVIL_CONSTRUCTORS(DocumentJsRunner);
};


bool DocumentJsRunner::Eval(const std::string16 &script, jsval *return_value) {
  JSObject *object = JS_GetGlobalObject(js_engine_context_);
  if (!object) { return false; }

  // To eval the script, we need the JSPrincipals to be acquired through
  // nsIPrincipal.  nsIPrincipal can be queried through the
  // nsIScriptObjectPrincipal interface on the Script Global Object.  In order
  // to get the Script Global Object, we need to request the private data
  // associated with the global JSObject on the current context.
  nsCOMPtr<nsIScriptGlobalObject> sgo;
  nsISupports *priv = reinterpret_cast<nsISupports *>(JS_GetPrivate(
                                                          js_engine_context_,
                                                          object));
  nsCOMPtr<nsIXPConnectWrappedNative> wrapped_native = do_QueryInterface(priv);

  if (wrapped_native) {
    // The global object is a XPConnect wrapped native, the native in
    // the wrapper might be the nsIScriptGlobalObject.
    sgo = do_QueryWrappedNative(wrapped_native);
  } else {
    sgo = do_QueryInterface(priv);
  }

  JSPrincipals *jsprin;
  nsresult nr;

  nsCOMPtr<nsIScriptObjectPrincipal> obj_prin = do_QueryInterface(sgo, &nr);
  if (NS_FAILED(nr)) { return false; }

  nsIPrincipal *principal = obj_prin->GetPrincipal();
  if (!principal) { return false; }

  // Get the script scheme and host from the principal.  This is the URI that
  // Firefox treats this script as running from.
  nsCOMPtr<nsIURI> codebase;
  principal->GetURI(getter_AddRefs(codebase));
  if (!codebase) { return false; }
  nsCString scheme;
  nsCString host;
  if (NS_FAILED(codebase->GetScheme(scheme)) ||
      NS_FAILED(codebase->GetHostPort(host))) {
    return false;
  }

  // Build a virtual filename that we'll run as.  This is to workaround
  // http://lxr.mozilla.org/seamonkey/source/dom/src/base/nsJSEnvironment.cpp#500
  // Bug: https://bugzilla.mozilla.org/show_bug.cgi?id=387477
  // The filename is being used as the security origin instead of the principal.
  // TODO(zork): Remove this section if this bug is resolved.
  std::string virtual_filename(scheme.BeginReading());
  virtual_filename += "://";
  virtual_filename += host.BeginReading();

  principal->GetJSPrincipals(js_engine_context_, &jsprin);

  // Set up the JS stack so that our context is on top.  This is needed to
  // play nicely with plugins that access the context stack, such as Firebug.
  nsCOMPtr<nsIJSContextStack> stack =
      do_GetService("@mozilla.org/js/xpc/ContextStack;1");
  if (!stack) { return false; }

  stack->Push(js_engine_context_);

  uintN line_number_start = 0;
  JS_BeginRequest(js_engine_context_);
  JSBool js_ok = JS_EvaluateUCScriptForPrincipals(
      js_engine_context_, object, jsprin,
      reinterpret_cast<const jschar *>(script.c_str()), script.length(),
      virtual_filename.c_str(), line_number_start, return_value);
  JS_EndRequest(js_engine_context_);

  // Restore the context stack.
  JSContext *cx;
  stack->Pop(&cx);

  // Decrements ref count on jsprin (Was added in GetJSPrincipals()).
  (void)JSPRINCIPALS_DROP(js_engine_context_, jsprin);
  if (!js_ok) { return false; }
  return true;
}

bool DocumentJsRunner::ListenForUnloadEvent() {
  unload_monitor_.reset(new HtmlEventMonitor(kEventUnload,
                                             HandleEventUnload,
                                             this));
  nsCOMPtr<nsIDOMWindowInternal> dom_window_internal;
  DOMUtils::GetDOMWindowInternal(js_engine_context_,
                                 getter_AddRefs(dom_window_internal));
  nsCOMPtr<nsIDOMEventTarget> dom_event_target;
  nsIDOMEventTarget **target = getter_AddRefs(dom_event_target);
  if (NS_SUCCEEDED(CallQueryInterface(dom_window_internal, target))) {
    unload_monitor_->Start(dom_event_target);
    is_unloaded_ = false;
    return true;
  }
  return false;
}

void DocumentJsRunner::HandleEventUnload(void *user_param) {
  DocumentJsRunner *document_js_runner =
      static_cast<DocumentJsRunner*>(user_param);
  document_js_runner->SendEvent(JSEVENT_UNLOAD);
  document_js_runner->is_unloaded_ = true;
  document_js_runner->DeleteIfNoLongerRelevant();
}

bool DocumentJsRunner::InvokeCallbackSpecialized(
                           const JsRootedCallback *callback,
                           int argc, jsval *argv,
                           JsRootedToken **optional_alloc_retval) {
  // When invoking a callback on the document context, we must go through
  // nsIScriptContext->CallEventHandler because it sets up certain state that
  // the browser error handler expects to find if there is an error. Without
  // this, crashes happen. For more information, see:
  // http://code.google.com/p/google-gears/issues/detail?id=32
  nsCOMPtr<nsIScriptContext> sc;
  sc = GetScriptContextFromJSContext(callback->context());
  if (!sc) { return false; }

  nsresult result = NS_OK;
  jsval retval = 0;

#if BROWSER_FF3
  JSContext* cx = reinterpret_cast<JSContext*>(sc->GetNativeContext());

  nsCOMPtr<nsIXPConnect> xpc = do_GetService("@mozilla.org/js/xpc/XPConnect;1",
                                             &result);
  if (NS_FAILED(result)) {
    return false;
  }

  nsCOMPtr<nsIMutableArray> argarray = do_CreateInstance("@mozilla.org/array;1",
                                                         &result);
  if (NS_FAILED(result)) {
    return false;
  }

  for (int i = 0; i < argc; ++i) {
    nsCOMPtr<nsIVariant> arg;
    if (NS_FAILED(xpc->JSToVariant(cx, argv[i], getter_AddRefs(arg)))) {
      return false;
    }
    argarray->AppendElement(arg, false);
  }

  JSObject* globalObject = JS_GetGlobalObject(callback->context());
  nsCOMPtr<nsIVariant> target;
  result = xpc->JSToVariant(cx, OBJECT_TO_JSVAL(globalObject),
                            getter_AddRefs(target));
  if (NS_FAILED(result)) {
    return false;
  }

  nsCOMPtr<nsIVariant> var_retval;
  result = sc->CallEventHandler(target,
                                globalObject, // scope
                                JSVAL_TO_OBJECT(callback->token()), // function
                                argarray,
                                getter_AddRefs(var_retval));

  if (NS_FAILED(result)) { return false; }

  result = xpc->VariantToJS(cx, globalObject, var_retval, &retval);

#else
  result = sc->CallEventHandler(JS_GetGlobalObject(callback->context()),
                                JSVAL_TO_OBJECT(callback->token()),
                                argc, argv, &retval);
#endif

  if (NS_FAILED(result)) { return false; }

  if (optional_alloc_retval) {
    // See note in JsRunner::InvokeCallbackSpecialized about return values of
    // javascript functions.
    *optional_alloc_retval = new JsRootedToken(js_engine_context_, retval);
  }

  return true;
}


JsRunnerInterface* NewJsRunner(JSRuntime *js_runtime) {
  return static_cast<JsRunnerInterface*>(new JsRunner(js_runtime));
}

JsRunnerInterface* NewDocumentJsRunner(void *, JsContextPtr context) {
  scoped_ptr<DocumentJsRunner> document_js_runner(
      new DocumentJsRunner(context));
  if (!document_js_runner->ListenForUnloadEvent()) {
    return NULL;
  }
  // A DocumentJsRunner who has had ListenForUnloadEvent called successfully
  // is self-deleting, so we can release it from our scoped_ptr.
  return document_js_runner.release();
}

// Local helper function.
static JsObject* JsvalToNewJsObject(const jsval &val, JsContextPtr context,
                                    bool dump_on_error) {
  if (JSVAL_IS_OBJECT(val)) {
    scoped_ptr<JsObject> retval(new JsObject);

    if (!retval->SetObject(val, context)) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      LOG(("Could not assign to JsObject."));
      return NULL;
    }
    return retval.release();
  } else {
    if (dump_on_error) ExceptionManager::ReportAndContinue();
    LOG(("Constructor did not return an object"));
    return NULL;
  }
}
