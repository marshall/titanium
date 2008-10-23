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

#include "gears/base/common/js_runner.h"

#include <assert.h>
#include <set>
#include <map>
#include <stdio.h>

#include "third_party/npapi/nphostapi.h"

#include "third_party/scoped_ptr/scoped_ptr.h"

#include "gears/base/chrome/module_cr.h"
#include "gears/base/common/async_router.h"
#include "gears/base/common/common.h" // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/html_event_monitor.h"
#include "gears/base/common/js_runner_utils.h"  // For ThrowGlobalErrorImpl()
#include "gears/base/common/scoped_token.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/common/timed_call.h"
#include "gears/base/npapi/browser_utils.h"
#include "gears/base/npapi/module.h"
#include "gears/base/npapi/np_utils.h"
#include "gears/base/npapi/scoped_npapi_handles.h"

#include "third_party/v8/public/v8.h"
#include "np_v8object.h"
#include "v8_npruntime.h"
#include "v8_helpers.h"

static const ThreadLocals::Slot kJsErrorHandlerKey = ThreadLocals::Alloc();
static const int kGarbageCollectionIntervalMsec = 30*1000;

// We keep a map of active DocumentJsRunners so we can notify them when their
// NP instance is destroyed.
class DocumentJsRunner;
typedef std::map<NPP, DocumentJsRunner*> DocumentJsRunnerList;
static DocumentJsRunnerList *g_document_js_runners = NULL;

static void RegisterDocumentJsRunner(NPP instance, DocumentJsRunner* runner) {
  if (!g_document_js_runners)
    g_document_js_runners = new DocumentJsRunnerList;
  // Right now there is a 1:1 mapping among NPPs, GearsFactorys, and
  // DocumentJsRunners.
  assert((*g_document_js_runners)[instance] == NULL);
  (*g_document_js_runners)[instance] = runner;
}

static void UnregisterDocumentJsRunner(NPP instance) {
  if (!g_document_js_runners)
    return;

  g_document_js_runners->erase(instance);
  if (g_document_js_runners->empty()) {
    delete g_document_js_runners;
    g_document_js_runners = NULL;
  }
}

// This helper class simply periodically pokes V8 to do some garbage collection.
// Note that this only affects background workers.
class TimedGC {
 public:
  // Create the timer on the main thread so it lives for the life of the
  // process.  V8 has a shared heap, so one GC cleans up for every worker.
  static void InitOnMainThread() {
    AsyncRouter::GetInstance()->CallAsync(
        CP::plugin_thread_id(), new CreateFunctor());
  }

 private:
  class CreateFunctor : public AsyncFunctor {
    void Run() {
      static bool initialized = false;
      if (initialized) return;
      initialized = true;
      new TimedGC();
    }
  };

  TimedGC() {
    timed_call_.reset(
        new TimedCall(kGarbageCollectionIntervalMsec, true, &TimerFired, this));
    V8Locker locker;
    v8_context_ = v8::Context::New();
  }
  ~TimedGC() {
    V8Locker locker;
    v8_context_.Dispose();
  }

  static void TimerFired(void *arg) {
    TimedGC *self = static_cast<TimedGC*>(arg);
    self->ForceGC();
  }

  void ForceGC() {
    V8Locker locker;

    v8::HandleScope handle_scope;
    v8::Context::Scope scope(v8_context_);

    v8::Local<v8::Script> script = v8::Script::Compile(
        v8::String::New("void(gc())"));
    if (script.IsEmpty()) return;
    script->Run();
  }

  scoped_ptr<TimedCall> timed_call_;
  v8::Persistent<v8::Context> v8_context_;
};

// Internal base class used to share some code between DocumentJsRunner and
// JsRunner. Do not override these methods from JsRunner or DocumentJsRunner.
// Either share the code here, or move it to those two classes if it's
// different.
class JsRunnerBase : public JsRunnerInterface {
 public:
  JsRunnerBase() : evaluator_(NULL) {
  }

  void OnModuleEnvironmentAttach() {
    // TODO(nigeltao): implement on NPAPI, i.e. plug the DocumentJsRunner leak.
  }

  void OnModuleEnvironmentDetach() {
    // TODO(nigeltao): implement on NPAPI, i.e. plug the DocumentJsRunner leak.
  }

  virtual NPObject *GetGlobalObject() = 0;

  bool AddGlobal(const std::string16 &name, ModuleImplBaseClass *object) {
    NPVariant np_window;
    OBJECT_TO_NPVARIANT(GetGlobalObject(), np_window);
    return SetProperty(np_window, name.c_str(), object->GetWrapperToken());
  }

  virtual JsObject *NewObject(bool dump_on_error = false) {
    return NewObjectImpl("Object()");
  }

  virtual JsObject *NewError(const std::string16 &message,
                             bool dump_on_error = false) {
    // We must manually escape special characters before evaluating the
    // JavaScript string.
    std::string16 escaped_message = EscapeMessage(message);
    std::string escaped_message_utf8;
    if (!String16ToUTF8(escaped_message.c_str(), escaped_message.size(),
                        &escaped_message_utf8)) {
      LOG(("Could not convert message."));
      return NULL;
    }
    return NewObjectImpl("Error('" + escaped_message_utf8 + "')");
  }

  virtual JsObject *NewDate(int64 milliseconds_since_epoch) {
    return NewObjectImpl("new Date(" +
                         Integer64ToString(milliseconds_since_epoch) +
                         ")");
  }

  virtual JsArray* NewArray() {
    scoped_ptr<JsObject> js_object(NewObjectImpl("Array()"));
    if (!js_object.get())
      return NULL;

    scoped_ptr<JsArray> js_array(new JsArray());
    if (!js_array.get())
      return NULL;

    if (!js_array->SetArray(js_object->token(), js_object->context()))
      return NULL;

    return js_array.release();
  }
  
  virtual bool ConvertJsObjectToDate(JsObject *obj,
                                      int64 *milliseconds_since_epoch) {
    assert(obj);
    assert(milliseconds_since_epoch);

    NPObject *np_obj = NPVARIANT_TO_OBJECT(obj->token());

    NPIdentifier get_time_id = NPN_GetStringIdentifier("getTime");

    ScopedNPVariant result;
    bool rv = NPN_Invoke(GetContext(), np_obj, get_time_id, NULL, 0, &result);
    if (!rv) {
      return false;
    }

    if (!NPVARIANT_IS_DOUBLE(result)) {
      return false;
    }

    *milliseconds_since_epoch = static_cast<int64>(NPVARIANT_TO_DOUBLE(result));

    return true;
  }

  virtual bool InvokeCallback(const JsRootedCallback *callback,
                              int argc, JsParamToSend *argv,
                              JsRootedToken **optional_alloc_retval) {
    assert(callback && (!argc || argv));
    if (!NPVARIANT_IS_OBJECT(callback->token())) { return false; }

    NPObject *evaluator = GetEvaluator();
    if (!evaluator) { return false; }

    // Setup argument array to pass to the evaluator.  The first argument is
    // the callback, followed by the argv array.  We invoke the function like:
    //   evaluator(callback, arg1, arg2, ..., argn)
    int evaluator_argc = argc + 1;  // extra arg for callback
    scoped_array<ScopedNPVariant> evaluator_args(
        new ScopedNPVariant[evaluator_argc]);
    evaluator_args[0].Reset(callback->token());
    for (int i = 0; i < argc; ++i)
      ConvertJsParamToToken(argv[i], GetContext(), &evaluator_args[i + 1]);

    // Invoke the method.
    ScopedNPVariant result;
    bool rv = NPN_InvokeDefault(GetContext(), evaluator,
                                evaluator_args.get(), evaluator_argc, &result);
    if (!rv) { return false; }

    if (!NPVARIANT_IS_OBJECT(result)) {
      assert(false);
      return false;
    }

    JsObject obj;
    obj.SetObject(result, GetContext());

    std::string16 exception;
    if (obj.GetPropertyAsString(STRING16(L"exception"), &exception)) {
      SetException(exception);
      return false;
    }

    ScopedNPVariant retval;
    if (!obj.GetProperty(STRING16(L"retval"), &retval)) {
      retval.Reset();  // Reset to "undefined"
    }

    if (optional_alloc_retval) {
      // Note: A valid NPVariant is returned no matter what the js function
      // returns. If it returns nothing, or explicitly returns <undefined>, the
      // variant will contain VOID. If it returns <null>, the variant will
      // contain NULL. Always returning a JsRootedToken should allow us to
      // coerce these values to other types correctly in the future.
      *optional_alloc_retval = new JsRootedToken(GetContext(), retval);
    }

    return true;
  }

  virtual bool Eval(const std::string16 &script) {
    return EvalImpl(script, true);
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

  // This function and others (AddEventHandler, RemoveEventHandler etc) do not
  // contain any browser-specific code. They should be implemented in a new
  // class 'JsRunnerCommon', which inherits from JsRunnerInterface.
  virtual void ThrowGlobalError(const std::string16 &message) {
    std::string16 string_to_eval =
        std::string16(STRING16(L"window.onerror('")) +
        EscapeMessage(message) +
        std::string16(STRING16(L"')"));
    EvalImpl(string_to_eval, false);
  }

#ifdef DEBUG
  void ForceGC() {
    // TODO(mpcomplete): implement me
  }
#endif

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

  // Throws an exception, handling the special case that we are not in
  // JavaScript context (in which case we throw the errow globally).
  void SetException(const std::string16& exception) {
    JsCallContext *context = BrowserUtils::GetCurrentJsCallContext();
    if (context) {
      context->SetException(exception);
    } else {
      ThrowGlobalError(exception);
    }
  }

 private:
  bool SetProperty(JsToken object, const char16 *name, const NPVariant &value) {
    if (!NPVARIANT_IS_OBJECT(object)) { return false; }

    std::string name_utf8;
    if (!String16ToUTF8(name, &name_utf8)) { return false; }

    NPObject *np_object = NPVARIANT_TO_OBJECT(object);
    NPIdentifier np_name = NPN_GetStringIdentifier(name_utf8.c_str());
    return NPN_SetProperty(GetContext(), np_object, np_name, &value);
  }

  // This function returns a lazily-initialized helper function for use in
  // calling callbacks.  This is a workaround for browsers without support for
  // window.onerror.  In the case of an asynchronous callback, called when no
  // JavaScript context is active, this helper lets us catch the exception
  // ourselves and manually call window.onerror, or bubble it up to a parent
  // worker as appropriate.
  NPObject *GetEvaluator() {
    if (evaluator_.get())
      return evaluator_.get();

    const char kEvaluatorScript[] =
      "(function () {"  // variable number of arguments
      "  var fn = arguments[0];"
      "  var args = Array.prototype.slice.call(arguments, 1);"
      "  var result = {};"
      "  try {"
      "    result.retval = fn.apply(null, args);"
      "  } catch (e) {"
      "    result.exception = e.message || String(e);"
      "  }"
      "  return result;"
      "})";
    NPObject *global = GetGlobalObject();
    NPString np_script = { kEvaluatorScript, ARRAYSIZE(kEvaluatorScript)-1 };
    ScopedNPVariant evaluator;
    if (!NPN_Evaluate(GetContext(), global, &np_script, &evaluator) ||
        !NPVARIANT_IS_OBJECT(evaluator)) {
      assert(false);
      return NULL;
    }

    evaluator_.reset(NPVARIANT_TO_OBJECT(evaluator));
    evaluator.Release();  // give ownership to evaluator_.
    return evaluator_.get();
  }

  // Creates an object by evaluating a string and getting the return value.
  JsObject *NewObjectImpl(const std::string &string_to_eval) {
    NPObject *global_object = GetGlobalObject();
    if (!global_object) {
      LOG(("Could not get global object from script engine."));
      return NULL;
    }
    // Evaluate javascript code: 'ConstructorName()'
    NPString script = {string_to_eval.c_str(), string_to_eval.length()};
    ScopedNPVariant object;
    bool rv = NPN_Evaluate(GetContext(), global_object, &script, &object);
    if (!rv) {
      LOG(("Could not invoke object constructor."));
      return NULL;
    }

    scoped_ptr<JsObject> retval(new JsObject);
    if (!retval->SetObject(object, GetContext())) {
      LOG(("Could not assign to JsObject."));
      return NULL;
    }

    return retval.release();
  }

  bool EvalImpl(const std::string16 &script, bool catch_exceptions) {
    NPObject *global_object = GetGlobalObject();

    std::string script_utf8;
    if (!String16ToUTF8(script.data(), script.length(), &script_utf8)) {
      LOG(("Could not convert script to UTF8."));
      return false;
    }

    if (catch_exceptions) {
      const char kEvaluatorTry[] = "try { ";
      const char kEvaluatorCatch[] =
          "; null; } catch (e) { e.message || String(e); }";
      script_utf8 = kEvaluatorTry + script_utf8 + kEvaluatorCatch;
    }

    NPString np_script = {script_utf8.data(), script_utf8.length()};
    ScopedNPVariant result;
    if (!NPN_Evaluate(GetContext(), global_object, &np_script, &result))
      return false;

    if (catch_exceptions && NPVARIANT_IS_STRING(result)) {
      // We caught an exception.
      NPString exception = NPVARIANT_TO_STRING(result);
      std::string16 exception16;
      if (!UTF8ToString16(exception.UTF8Characters, exception.UTF8Length,
                          &exception16)) {
        exception16 = STRING16(L"Could not get exception text");
      }
      SetException(exception16);
      return false;
    }

    return true;
  }

  std::set<JsEventHandlerInterface *> event_handlers_[MAX_JSEVENTS];
  ScopedNPObject evaluator_;
  DISALLOW_EVIL_CONSTRUCTORS(JsRunnerBase);
};

class JsRunner : public JsRunnerBase {
 public:
  JsRunner() : error_handler_(NULL) {
    V8Locker locker;
    
    // Register our message listener.  Ensure it's only called once.
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      v8::V8::AddMessageListener(&JsMessageHandler);

      // Setting this flag enables the 'gc()' JavaScript method.
      const char kFlags[] = "--expose-gc";
      v8::V8::SetFlagsFromString(kFlags, ARRAYSIZE(kFlags));

      //TimedGC::InitOnMainThread();
    }

    v8_context_ = v8::Context::New();

    // Associate the JsRunner with this thread, so we know which one got an
    // error message in the error handler.
    ThreadLocals::SetValue(kJsErrorHandlerKey, this, NULL);

    // Once the NP runtime bindings are set up, most of our functionality can
    // be implemented by the common NPAPI base class.
    InitNPBindings();
    global_object_ = V8_NPN_GetGlobalObject(GetContext());

    // Add the global object to the registry.  When it gets destroyed, it is
    // unregistered, and all subobjects are released.
    _NPN_RegisterObject(global_object_, NULL);
  }

  virtual ~JsRunner() {
    ThreadLocals::DestroyValue(kJsErrorHandlerKey);

    // Alert modules that the engine is unloading.
    SendEvent(JSEVENT_UNLOAD);

    np_instance_data_.ndata = NULL;

    V8Locker locker;
    V8_NPN_ReleaseObject(global_object_);
    v8_context_.Dispose();
  }

  JsContextPtr GetContext() {
    return &np_instance_data_;
  }

  NPObject *GetGlobalObject() {
    return global_object_;
  }

  bool Start(const std::string16 &full_script) {
    return Eval(full_script);
  }

  bool Stop() {
    // TODO(mpcomplete): what does this mean?
    return false;
  }

  void SetErrorHandler(JsErrorHandlerInterface *error_handler) {
    error_handler_ = error_handler;
  }

  v8::Local<v8::Context> GetV8Context() {
    return v8::Local<v8::Context>::New(v8_context_);
  }

  virtual void ThrowGlobalError(const std::string16 &message) {
    if (error_handler_) {
      JsErrorInfo error_info;
      error_info.line = 1;  // ??
      error_info.message = message;
      error_handler_->HandleError(error_info);
    }
  }

 private:
  void InitNPBindings() {
    // Initialize NPN binding functions.
    static NPNetscapeFuncs browser_funcs;
    static bool initialized = false;
    if (!initialized) {
      initialized = true;
      memset(&browser_funcs, 0, sizeof(browser_funcs));
      browser_funcs.size = sizeof(browser_funcs);
      browser_funcs.version = 1;
      browser_funcs.memalloc = V8_NPN_MemAlloc;
      browser_funcs.memfree = V8_NPN_MemFree;
      browser_funcs.getvalue = V8_NPN_GetValue;
      browser_funcs.releasevariantvalue = V8_NPN_ReleaseVariantValue;
      browser_funcs.getstringidentifier = V8_NPN_GetStringIdentifier;
      browser_funcs.getstringidentifiers = V8_NPN_GetStringIdentifiers;
      browser_funcs.getintidentifier = V8_NPN_GetIntIdentifier;
      browser_funcs.identifierisstring = V8_NPN_IdentifierIsString;
      browser_funcs.utf8fromidentifier = V8_NPN_UTF8FromIdentifier;
      browser_funcs.intfromidentifier = V8_NPN_IntFromIdentifier;
      browser_funcs.createobject = V8_NPN_CreateObject;
      browser_funcs.retainobject = V8_NPN_RetainObject;
      browser_funcs.releaseobject = V8_NPN_ReleaseObject;
      browser_funcs.invoke = V8_NPN_Invoke;
      browser_funcs.invokeDefault = V8_NPN_InvokeDefault;
      browser_funcs.evaluate = V8_NPN_Evaluate;
      browser_funcs.getproperty = V8_NPN_GetProperty;
      browser_funcs.setproperty = V8_NPN_SetProperty;
      browser_funcs.removeproperty = V8_NPN_RemoveProperty;
      browser_funcs.hasproperty = V8_NPN_HasProperty;
      browser_funcs.hasmethod = V8_NPN_HasMethod;
      browser_funcs.setexception = V8_NPN_SetException;
      browser_funcs.enumerate = V8_NPN_Enumerate;
    }

    ThreadLocals::SetValue(kNPNFuncsKey, &browser_funcs, NULL);

    np_instance_data_.ndata = this;  // JsRunner is the browser side.
    np_instance_data_.pdata = NULL;  // for use by the plugin side.
  }

  static void JsMessageHandler(v8::Handle<v8::Message> message,
                               v8::Handle<v8::Value> data) {
    JsRunner *js_runner =
        reinterpret_cast<JsRunner*>(ThreadLocals::GetValue(kJsErrorHandlerKey));
    if (js_runner && js_runner->error_handler_) {
      JsErrorInfo error_info;
      error_info.line = message->GetLineNumber();

      v8::Handle<v8::String> msg_str = message->Get();
      assert(!msg_str.IsEmpty());
      scoped_ptr<uint16_t> msg_data(new uint16_t[msg_str->Length() + 1]);
      msg_str->Write(msg_data.get());
      error_info.message = reinterpret_cast<char16 *>(msg_data.get());

      V8Unlocker unlocker;
      js_runner->error_handler_->HandleError(error_info);
    }
  }

  v8::Persistent<v8::Context> v8_context_;
  NPP_t np_instance_data_;
  NPObject *global_object_;
  JsErrorHandlerInterface *error_handler_;

  DISALLOW_EVIL_CONSTRUCTORS(JsRunner);
};

// These functions are declared in v8_helpers.h but implemented here so they
// can access the JsRunner.
v8::Local<v8::Context> GetV8Context(NPP npp, NPObject *npobj) {
  assert(npp);
  return static_cast<JsRunner *>(npp->ndata)->GetV8Context();
}

NPObject *GetGlobalNPObject(NPP npp) {
  assert(npp);
  return static_cast<JsRunner *>(npp->ndata)->GetGlobalObject();
}

// This class is a stub that is used to present a uniform interface to
// common functionality to both workers and the main thread.
class DocumentJsRunner : public JsRunnerBase {
 public:
  DocumentJsRunner(NPP instance)
      : np_instance_(instance) {
    NPN_GetValue(np_instance_, NPNVWindowNPObject, &global_object_);
    RegisterDocumentJsRunner(np_instance_, this);
  }
  virtual ~DocumentJsRunner() {
    // TODO(mpcomplete): This never gets called.  When should we delete the
    // DocumentJsRunner?
    UnregisterDocumentJsRunner(np_instance_);
    NPN_ReleaseObject(global_object_);
  }

  JsContextPtr GetContext() {
    return np_instance_;
  }

  NPObject *GetGlobalObject() {
    return global_object_;
  }

  bool Start(const std::string16 &full_script) {
    assert(false); // Should not be called on the DocumentJsRunner.
    return false;
  }

  bool Stop() {
    assert(false); // Should not be called on the DocumentJsRunner.
    return false;
  }

  void SetErrorHandler(JsErrorHandlerInterface *handler) {
    assert(false); // Should not be called on the DocumentJsRunner.
  }

  void HandleNPInstanceDestroyed() {
    SendEvent(JSEVENT_UNLOAD);
    UnregisterDocumentJsRunner(np_instance_);
  }

  // TODO(mpcomplete): We only support JSEVENT_UNLOAD.  We should rework this
  // API to make it non-generic, and implement unload similar to NPAPI in
  // other browsers.
  bool AddEventHandler(JsEventType event_type,
                       JsEventHandlerInterface *handler) {
    return JsRunnerBase::AddEventHandler(event_type, handler);
  }

 private:
  NPP np_instance_;
  NPObject *global_object_;
  DISALLOW_EVIL_CONSTRUCTORS(DocumentJsRunner);
};

void NotifyNPInstanceDestroyed(NPP instance) {
  if (!g_document_js_runners)
    return;

  DocumentJsRunnerList::iterator it = g_document_js_runners->find(instance);
  if (it != g_document_js_runners->end()) {
    DocumentJsRunner *js_runner = it->second;
    js_runner->HandleNPInstanceDestroyed();
  }
}

JsRunnerInterface *NewJsRunner() {
  return static_cast<JsRunnerInterface *>(new JsRunner());
}

JsRunnerInterface *NewDocumentJsRunner(void *, JsContextPtr context) {
  return static_cast<JsRunnerInterface *>(new DocumentJsRunner(context));
}
