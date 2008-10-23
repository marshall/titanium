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

#ifdef BROWSER_WEBKIT
#include <WebKit/npapi.h>
#else
#include "third_party/npapi/nphostapi.h"
#endif

#include "third_party/scoped_ptr/scoped_ptr.h"

#include "gears/base/common/basictypes.h" // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/html_event_monitor.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_runner_utils.h"  // For ThrowGlobalErrorImpl()
#include "gears/base/common/js_standalone_engine.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/npapi/browser_utils.h"
#include "gears/base/npapi/module.h"
#include "gears/base/npapi/np_utils.h"
#include "gears/base/npapi/scoped_npapi_handles.h"
#ifdef BROWSER_WEBKIT
#include "gears/base/safari/npapi_patches.h" 
#endif

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

  // TODO(nigeltao): Are we leaking the DocumentJsRunner (since the map's
  // value type is just a raw pointer, not a linked_ptr)? If so, this would
  // be a good time to delete it.
  g_document_js_runners->erase(instance);
  if (g_document_js_runners->empty()) {
    delete g_document_js_runners;
    g_document_js_runners = NULL;
  }
}

// Internal base class used to share some code between DocumentJsRunner and
// JsRunner. Do not override these methods from JsRunner or DocumentJsRunner.
// Either share the code here, or move it to those two classes if it's
// different.
class JsRunnerBase : public JsRunnerInterface {
 public:
  JsRunnerBase() : evaluator_(NULL) {
  }
  
  virtual ~JsRunnerBase() {
    // Should have called Cleanup() to kill this before getting here();
    assert(evaluator_.get() == NULL);
  }
  
  void OnModuleEnvironmentAttach() {
    // TODO(nigeltao): implement on NPAPI, i.e. plug the DocumentJsRunner leak.
  }

  void OnModuleEnvironmentDetach() {
    // TODO(nigeltao): implement on NPAPI, i.e. plug the DocumentJsRunner leak.
  }

  virtual NPObject *GetGlobalObject() = 0;
  
  // Because JsRunner destroys the JSEngine in it's destructor, we add this
  // function which cleans up objects that need a valid JSEngine in order
  // to be succesfully destructed.
  void Cleanup() {
    evaluator_.reset(NULL);
  }
  
  bool AddGlobal(const std::string16 &name, ModuleImplBaseClass *object) {
    NPVariant np_window;
    OBJECT_TO_NPVARIANT(GetGlobalObject(), np_window);
    return SetProperty(np_window, name.c_str(), object->GetWrapperToken());
  }

  JsObject *NewObject(bool dump_on_error = false) {
    return NewObjectImpl("Object()");
  }

  JsObject *NewError(const std::string16 &message,
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

  JsObject *NewDate(int64 milliseconds_since_epoch) {
    return NewObjectImpl("new Date(" +
                         Integer64ToString(milliseconds_since_epoch) +
                         ")");
  }

  JsArray* NewArray() {
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

  bool ConvertJsObjectToDate(JsObject *obj,
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

  bool InvokeCallback(const JsRootedCallback *callback,
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
#ifdef DEBUG
      LOG(("Exception during eval: %s\n", exception.UTF8Characters));
#endif
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
  // JavaScript context (in which case we throw the error globally).
  void SetException(const std::string16& exception) {
    JsCallContext *context = BrowserUtils::GetCurrentJsCallContext();
    if (context) {
      context->SetException(exception);
    } else {
      ThrowGlobalError(exception);
    }
  }

  NPObject *global_object_;

 private:
  bool SetProperty(JsToken object, const char16 *name, const NPVariant &value) {
    if (!NPVARIANT_IS_OBJECT(object)) { return false; }

    std::string name_utf8;
    if (!String16ToUTF8(name, &name_utf8)) { return false; }

    NPObject *np_object = NPVARIANT_TO_OBJECT(object);
    NPIdentifier np_name = NPN_GetStringIdentifier(name_utf8.c_str());
    return NPN_SetProperty(GetContext(), np_object, np_name, &value);
  }
  
  NPObject *GetEvaluator() {
    if (evaluator_.get())
      return evaluator_.get();

    // Wierd Safari bug: if you remove the surrounding parenthesis, this ceases
    // to work.
    const char kEvaluatorScript[] = 
      "(function () {"  // variable number of arguments
      "  var fn = arguments[0];"
      "  var args = Array.prototype.slice.call(arguments, 1);"
      "   var result = {};"
      "   try {"
      "     result.retval = fn.apply(null, args);"
      "   } catch (e) {"
      "     result.exception = String(e);"
      "   }"
      "   return result;"
      " })";
    NPObject *global = GetGlobalObject();
    NPString np_script = { kEvaluatorScript, ARRAYSIZE(kEvaluatorScript) - 1};
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

  std::set<JsEventHandlerInterface *> event_handlers_[MAX_JSEVENTS];
  ScopedNPObject evaluator_;
  DISALLOW_EVIL_CONSTRUCTORS(JsRunnerBase);
};

// Runner used by workers.
// Spools up an external JS Engine (Spidermonkey in the case of Safari), and
// uses it's NPAPI bindings.  This allows us to share most of the code with
// DocumentJSRunner.
class JsRunner : public JsRunnerBase {
 public:
  JsRunner() {
#ifdef BROWSER_WEBKIT
    static GearsNPNetscapeFuncs browser_callbacks;
#else
    static NPNetscapeFuncs browser_callbacks;
#endif
    static bool initialized = false;
    static Mutex browser_callbacks_mutex;
    
    if (!initialized) {
      MutexLock lock(&browser_callbacks_mutex); 
      JSStandaloneEngine::GetNPNEntryPoints(
          (NPNetscapeFuncs *)&browser_callbacks);
      initialized = true;
    }
    
    JSStandaloneEngine::InitEngine(this, &np_instance_data_);
    ThreadLocals::SetValue(kNPNFuncsKey, &browser_callbacks, NULL);
    NPN_GetValue(GetContext(), NPNVWindowNPObject, &global_object_);
  }
  
  virtual ~JsRunner() {
    // Alert modules that the engine is unloading.
    SendEvent(JSEVENT_UNLOAD);
    
    // Must do this here as cannot call after TerminateEngine().
    Cleanup();
    
    NPN_ReleaseObject(global_object_);
    JSStandaloneEngine::TerminateEngine();
  }

  JsContextPtr GetContext() {
    return &np_instance_data_;
  }

  NPObject *GetGlobalObject() {
    return global_object_;
  }
  
  bool Start(const std::string16 &full_script) {
    LOG(("Starting Worker\n"));
    return Eval(full_script);
  }
  
  bool Stop() {
    LOG(("Worker Stopped\n"));
    // TODO(mpcomplete): what does this mean?
    return false;
  }
  
#ifdef DEBUG
  void ForceGC() {
    Eval(STRING16(L"GearsInternalCollectGarbage();"));
  }
#endif

  bool Eval(const std::string16 &script) {
    // EvalImpl wraps the calling code in a try{}catch block which
    // changes the behavior of the code in SpiderMonkey (possibly other
    // JS engines.  For an example of this try running:
    // var a = b; function b() {}
    // Then try wrapping in a try/catch and notice the syntax error.
    // In order to get around this, we pass in false for the catch_exceptions
    // parameter.  We use the native exception handling mechanism of the
    // external JS engine to capture exceptions thrown here.
    return EvalImpl(script, false);
  }
  
  void SetErrorHandler(JsErrorHandlerInterface *error_handler) {
    error_handler_ = error_handler;
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
  // Needs access to error_handler_.
  friend void HandleJSError(const JsRunner *js_runner, JsErrorInfo &error_info);
   
  NPP_t np_instance_data_;
  NPObject *global_object_;
  JsErrorHandlerInterface *error_handler_;
  DISALLOW_EVIL_CONSTRUCTORS(JsRunner);
};


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
    Cleanup();
    UnregisterDocumentJsRunner(np_instance_);
    NPN_ReleaseObject(global_object_);
  }
   
  NPObject *GetGlobalObject() {
    return global_object_;
  }

  JsContextPtr GetContext() {
    return np_instance_;
  }

  bool Start(const std::string16 &full_script) {
    assert(false); // Should not be called on the DocumentJsRunner.
    return false;
  }

  bool Stop() {
    assert(false); // Should not be called on the DocumentJsRunner.
    return false;
  }
  
#ifdef DEBUG
  void ForceGC() {
    // TODO(playmobil): implement for main thread.
  }
#endif

  bool Eval(const std::string16 &script) {
    return EvalImpl(script, true);
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

// Referenced from js_external_engine_mozjs.cpp, defined here.
void HandleJSError(const JsRunner *js_runner, JsErrorInfo &error_info) {
  if (js_runner->error_handler_) {
    js_runner->error_handler_->HandleError(error_info);
  }
}

JsRunnerInterface *NewJsRunner() {
  return static_cast<JsRunnerInterface *>(new JsRunner());
}

JsRunnerInterface *NewDocumentJsRunner(void *, JsContextPtr context) {
  return static_cast<JsRunnerInterface *>(new DocumentJsRunner(context));
}
