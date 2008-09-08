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
//
// In IE, JsRunner is a wrapper and most work happens in
// JsRunnerImpl.  The split is necessary so we can do two things:
// (1) provide a simple NewJsRunner/delete interface (not ref-counting), plus
// (2) derive from ATL for the internal implementation (e.g. for ScriptSiteImpl)
//
// JAVASCRIPT ENGINE DETAILS: Internet Explorer uses the JScript engine.
// The interface is IActiveScript, a shared scripting engine interface.
// Communication from the engine to external objects, and communication
// from externally into the engine, is all handled via IDispatch pointers.

#include <assert.h>
#include <dispex.h>
#include <map>
#include <set>
#include <vector>
#include "third_party/scoped_ptr/scoped_ptr.h"

#include "gears/base/common/js_runner.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/basictypes.h"  // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/exception_handler.h"
#ifdef WINCE
// WinCE does not use HtmlEventMonitor to monitor page unloading.
#else
#include "gears/base/common/html_event_monitor.h"
#endif
#include "gears/base/common/js_runner_utils.h"  // For ThrowGlobalErrorImpl()
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"  // For CallWindowOnerror()
#endif
#include "gears/base/ie/activex_utils.h"
#include "gears/base/ie/atl_headers.h"
#include "third_party/AtlActiveScriptSite.h"


// Internal base class used to share some code between DocumentJsRunner and
// JsRunner. Do not override these methods from JsRunner or DocumentJsRunner.
// Either share the code here, or move it to those two classes if it's
// different.
class JsRunnerBase : public JsRunnerInterface {
 public:
  JsRunnerBase() {}

  virtual ~JsRunnerBase() {
    for (int i = 0; i < MAX_JSEVENTS; i++) {
      assert(0 == event_handlers_[i].size());
    }
  }

  JsContextPtr GetContext() {
    return NULL;  // not used in IE
  }

  JsObject *NewObject(bool dump_on_error = false) {
    return NewObjectWithArguments(STRING16(L"Object"), 0, NULL, dump_on_error);
  }

  JsObject *NewError(const std::string16 &message,
                     bool dump_on_error = false) {
    JsParamToSend argv[] = { {JSPARAM_STRING16, &message} };
    return NewObjectWithArguments(STRING16(L"Error"), ARRAYSIZE(argv), argv,
                                  dump_on_error);
  }

  JsObject *NewDate(int64 milliseconds_since_epoch) {
    JsParamToSend argv[] = { {JSPARAM_INT64, &milliseconds_since_epoch} };
    return NewObjectWithArguments(STRING16(L"Date"), ARRAYSIZE(argv), argv,
                                  false);
  }

  JsArray* NewArray() {
    scoped_ptr<JsObject> js_object(NewObjectWithArguments(STRING16(L"Array"), 0,
                                   NULL, false));
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

    if (V_VT(&(obj->token())) != VT_DISPATCH) {
      return false;
    }
    CComPtr<IDispatch> dispatch = V_DISPATCH(&(obj->token()));

    DISPID function_iid;
    if (FAILED(ActiveXUtils::GetDispatchMemberId(dispatch,
                                                 STRING16(L"getTime"),
                                                 &function_iid))) {
      return false;
    }
    DISPPARAMS parameters = {0};
    VARIANT retval = {0};
    EXCEPINFO exception;
    HRESULT hr = dispatch->Invoke(
        function_iid,           // member to invoke
        IID_NULL,               // reserved
        LOCALE_SYSTEM_DEFAULT,  // locale
        DISPATCH_METHOD,        // dispatch/invoke as...
        &parameters,            // parameters
        &retval,                // receives result
        &exception,             // receives exception
        NULL);                  // receives badarg index
    if (FAILED(hr)) {
      return false;
    }

    if (V_VT(&retval) != VT_R8) {
      return false;
    }

    *milliseconds_since_epoch = static_cast<int64>(V_R8(&retval));
    return true;
  }

  bool InvokeCallbackImpl(const JsRootedCallback *callback,
                          int argc, JsParamToSend *argv,
                          JsRootedToken **optional_alloc_retval,
                          std::string16 *optional_exception) {
    assert(callback && (!argc || argv));
    if (callback->token().vt != VT_DISPATCH) { return false; }

    // Setup argument array.
    scoped_array<CComVariant> js_engine_argv(new CComVariant[argc]);
    for (int i = 0; i < argc; ++i) {
      int dest = argc - 1 - i;  // args are expected in reverse order!!
      ConvertJsParamToToken(argv[i], NULL, &js_engine_argv[dest]);
    }

    // Invoke the method.
    DISPPARAMS invoke_params = {0};
    invoke_params.cArgs = argc;
    invoke_params.rgvarg = js_engine_argv.get();

    VARIANT retval = {0};
    EXCEPINFO exception;
    // If the JavaScript function being invoked throws an exception, Invoke
    // returns DISP_E_EXCEPTION, or the undocumented 0x800A139E if we don't pass
    // an EXECINFO pointer. See
    // http://asp3wiki.wrox.com/wiki/show/G.2.2-+Runtime+Errors for a
    // description of 0x800A139E.
    HRESULT hr = callback->token().pdispVal->Invoke(
        DISPID_VALUE, IID_NULL,  // DISPID_VALUE = default action
        LOCALE_SYSTEM_DEFAULT,   // TODO(cprince): should this be user default?
        DISPATCH_METHOD,         // dispatch/invoke as...
        &invoke_params,          // parameters
        optional_alloc_retval ? &retval : NULL,  // receives result (NULL okay)
        &exception,              // receives exception (NULL okay)
        NULL);                   // receives badarg index (NULL okay)
    if (FAILED(hr)) {
      if (DISP_E_EXCEPTION == hr && optional_exception) {
        *optional_exception = exception.bstrDescription;
      }
      return false;
    }

    if (optional_alloc_retval) {
      // Note: A valid VARIANT is returned no matter what the js function
      // returns. If it returns nothing, or explicitly returns <undefined>, the
      // variant will contain VT_EMPTY. If it returns <null>, the variant will
      // contain VT_NULL. Always returning a JsRootedToken should allow us to
      // coerce these values to other types correctly in the future.
      *optional_alloc_retval = new JsRootedToken(NULL, retval);
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

#ifdef DEBUG
  void ForceGC() {
    // CollectGarbage is not available through COM, so we call through JS.
    if (!Eval(STRING16(L"CollectGarbage();"))) {
      assert(false);
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

  // TODO(zork): Remove the argument when we find the error.
  virtual IDispatch *GetGlobalObject(bool dump_on_error = false) = 0;

 private:
  JsObject *NewObjectWithArguments(const std::string16 &ctor,
                                   int argc, JsParamToSend *argv,
                                   bool dump_on_error) {
    assert(!argc||argv);
    CComPtr<IDispatch> global_object = GetGlobalObject(dump_on_error);
    if (!global_object) {
      LOG(("Could not get global object from script engine."));
      return NULL;
    }

    CComQIPtr<IDispatchEx> global_dispex = global_object;
    if (!global_dispex) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      return NULL;
    }

    DISPID error_dispid;
    CComBSTR ctor_name(ctor.c_str());
    HRESULT hr = global_dispex->GetDispID(ctor_name, fdexNameCaseSensitive,
                                          &error_dispid);
    if (FAILED(hr)) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      return NULL;
    }

    CComVariant result;

    // Setup argument array.
    scoped_array<CComVariant> js_engine_argv(new CComVariant[argc]);
    for (int i = 0; i < argc; ++i) {
      int dest = argc - 1 - i;  // args are expected in reverse order!!
      ConvertJsParamToToken(argv[i], NULL, &js_engine_argv[dest]);
    }
    DISPPARAMS args = {0};
    args.cArgs = argc;
    args.rgvarg = js_engine_argv.get();

    hr = global_dispex->InvokeEx(
                            error_dispid, LOCALE_USER_DEFAULT,
                            DISPATCH_CONSTRUCT, &args, &result,
                            NULL,  // receives exception (NULL okay)
                            NULL);  // pointer to "this" object (NULL okay)
    if (FAILED(hr)) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      LOG(("Could not invoke object constructor."));
      return NULL;
    }

    scoped_ptr<JsObject> retval(new JsObject);
    if (!retval->SetObject(result, GetContext())) {
      LOG(("Could not assign to JsObject."));
      return NULL;
    }

    return retval.release();
  }

  std::set<JsEventHandlerInterface *> event_handlers_[MAX_JSEVENTS];

  DISALLOW_EVIL_CONSTRUCTORS(JsRunnerBase);
};


// Internal helper COM object that implements IActiveScriptSite so we can host
// new ActiveScript engine instances. This class does the majority of the work
// of the real JsRunner.
class ATL_NO_VTABLE JsRunnerImpl
    : public CComObjectRootEx<CComMultiThreadModel>,
      public IDispatchImpl<IDispatch>,
      public IActiveScriptSiteImpl,
      public IInternetHostSecurityManager,
      public IServiceProviderImpl<JsRunnerImpl> {
 public:
  BEGIN_COM_MAP(JsRunnerImpl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IActiveScriptSite)
    COM_INTERFACE_ENTRY(IInternetHostSecurityManager)
    COM_INTERFACE_ENTRY(IServiceProvider)
  END_COM_MAP()

  // For IServiceProviderImpl (used to disable ActiveX objects, along with
  // IInternetHostSecurityManager).
  BEGIN_SERVICE_MAP(JsRunnerImpl)
    SERVICE_ENTRY(SID_SInternetHostSecurityManager)
  END_SERVICE_MAP()

  JsRunnerImpl() : coinit_succeeded_(false) {}
  ~JsRunnerImpl();

  // JsRunnerBase implementation
  IDispatch *GetGlobalObject(bool dump_on_error = false) {
    IDispatch *global_object;
    HRESULT hr = javascript_engine_->GetScriptDispatch(
                                         NULL,  // get global object
                                         &global_object);
    if (FAILED(hr)) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      return NULL;
    }
    return global_object;
  }

  // JsRunner implementation
  bool AddGlobal(const std::string16 &name, IUnknown *object);
  bool Start(const std::string16 &full_script);
  bool Stop();
  bool Eval(const std::string16 &script);
  void SetErrorHandler(JsErrorHandlerInterface *error_handler) {
    error_handler_ = error_handler;
  }

  bool CleanUpJsGlobalVariables();

  // IActiveScriptSiteImpl overrides
  STDMETHOD(LookupNamedItem)(const OLECHAR *name, IUnknown **item);
  STDMETHOD(HandleScriptError)(EXCEPINFO *ei, ULONG line, LONG pos, BSTR src);

  // Implement IInternetHostSecurityManager. We use this to prevent the script
  // engine from creating ActiveX objects, using Java, using scriptlets and
  // various other questionable activities.
  STDMETHOD(GetSecurityId)(BYTE *securityId, DWORD *securityIdSize,
                           DWORD_PTR reserved);
  STDMETHOD(ProcessUrlAction)(DWORD action, BYTE *policy, DWORD policy_size,
                              BYTE *context, DWORD context_size, DWORD flags,
                              DWORD reserved);
  STDMETHOD(QueryCustomPolicy)(REFGUID guid_key, BYTE **policy,
                               DWORD *policy_size, BYTE *context,
                               DWORD context_size, DWORD reserved);

 private:
  CComPtr<IActiveScript> javascript_engine_;
  JsErrorHandlerInterface *error_handler_;

  typedef std::map<std::string16, IUnknown *> NameToObjectMap;
  NameToObjectMap global_name_to_object_;

  bool coinit_succeeded_;

  DISALLOW_EVIL_CONSTRUCTORS(JsRunnerImpl);
};


JsRunnerImpl::~JsRunnerImpl() {
  // decrement all refcounts incremented by AddGlobal()
  JsRunnerImpl::NameToObjectMap::const_iterator it;
  const JsRunnerImpl::NameToObjectMap &map = global_name_to_object_;
  for (it = map.begin(); it != map.end(); ++it) {
    it->second->Release();
  }

  if (coinit_succeeded_) {
    CoUninitialize();
  }
}


bool JsRunnerImpl::AddGlobal(const std::string16 &name,
                             IUnknown *object) {
  if (!object) { return false; }

  // We AddRef() to make sure the object lives as long as the JS engine.
  // This gets removed in ~JsRunnerImpl.
  object->AddRef();
  global_name_to_object_[name] = object;

  // Start() will insert any globals added before the JS engine started.
  // But we must call AddNamedItem() here if the JS engine is already running.
  if (javascript_engine_) {
    HRESULT hr = javascript_engine_->AddNamedItem(name.c_str(),
                                                  SCRIPTITEM_ISVISIBLE);
    if (FAILED(hr)) { return false; }
  }

  return true;
}


bool JsRunnerImpl::Start(const std::string16 &full_script) {
  HRESULT hr;

  coinit_succeeded_ = SUCCEEDED(CoInitializeEx(NULL,
                                               GEARS_COINIT_THREAD_MODEL));
  if (!coinit_succeeded_) { return false; }
  // CoUninitialize is handled in dtor

  //
  // Instantiate a JavaScript engine
  //

  CLSID id;
  hr = CLSIDFromProgID(L"JScript", &id);
  if (FAILED(hr)) { return false; }

  hr = javascript_engine_.CoCreateInstance(id);
  if (FAILED(hr)) { return false; }

  // Set the engine's site (which the engine queries when it encounters
  // an unknown name).
  hr = javascript_engine_->SetScriptSite(this);
  if (FAILED(hr)) { return false; }


  // Tell the script engine up to use a custom security manager implementation.
  CComQIPtr<IObjectSafety> javascript_engine_safety;
  javascript_engine_safety = javascript_engine_;
  if (!javascript_engine_safety) { return false; }

  hr = javascript_engine_safety->SetInterfaceSafetyOptions(
                                     IID_IDispatch,
                                     INTERFACE_USES_SECURITY_MANAGER,
                                     INTERFACE_USES_SECURITY_MANAGER);
  if (FAILED(hr)) { return false; }


  //
  // Tell the engine about named globals (set earlier via AddGlobal).
  //

  JsRunnerImpl::NameToObjectMap::const_iterator it;
  const JsRunnerImpl::NameToObjectMap &map = global_name_to_object_;
  for (it = map.begin(); it != map.end(); ++it) {
    const std::string16 &name = it->first;
    hr = javascript_engine_->AddNamedItem(name.c_str(), SCRIPTITEM_ISVISIBLE);
    // TODO(cprince): see if need |= SCRIPTITEM_GLOBALMEMBERS
    if (FAILED(hr)) { return false; }
  }


  //
  // Add script code to the engine instance
  //

  CComQIPtr<IActiveScriptParse> javascript_engine_parser;

  javascript_engine_parser = javascript_engine_;
  if (!javascript_engine_parser) { return false; }

  hr = javascript_engine_parser->InitNew();
  if (FAILED(hr)) { return false; }
  // why does ParseScriptText also AddRef the object?
  hr = javascript_engine_parser->ParseScriptText(full_script.c_str(),
                                                 NULL, NULL, NULL, 0, 0,
                                                 SCRIPTITEM_ISVISIBLE,
                                                 NULL, NULL);
  if (FAILED(hr)) { return false; }

  //
  // Start the engine running
  //

  // TODO(cprince): do we need STARTED *and* CONNECTED? (Does it matter?)
  // CONNECTED "runs the script and blocks until the script is finished"
  hr = javascript_engine_->SetScriptState(SCRIPTSTATE_STARTED);
  if (FAILED(hr)) { return false; }
  hr = javascript_engine_->SetScriptState(SCRIPTSTATE_CONNECTED);
  if (FAILED(hr)) { return false; }

  // NOTE: at this point, the JS code has returned, and it should have set
  // an onmessage handler.  (If it didn't, the worker is most likely cut off
  // from other workers.  There are ways to prevent this, but they are poor.)

  return true;  // succeeded
}

bool JsRunnerImpl::Eval(const std::string16 &script) {
  CComQIPtr<IActiveScriptParse> javascript_engine_parser;

  // Get the parser interface
  javascript_engine_parser = javascript_engine_;
  if (!javascript_engine_parser) { return false; }

  // Execute the script
  HRESULT hr = javascript_engine_parser->ParseScriptText(script.c_str(),
                                                         NULL, NULL, 0, 0, 0,
                                                         SCRIPTITEM_ISVISIBLE,
                                                         NULL, NULL);
  if (FAILED(hr)) { return false; }
  return true;
}

bool JsRunnerImpl::CleanUpJsGlobalVariables() {
  // NOTE(nigeltao): Yes, this is as hack. Without this method, Gears is
  // leaking all global JavaScript objects in worker ActiveScript instances.
  // For example, this worker code:
  // var foo = google.gears.factory.create('beta.timer');
  // google.gears.workerPool.onmessage = function(a, b, msg) {
  //   var bar = google.gears.factory.create('beta.timer');
  //   google.gears.workerPool.sendMessage(msg.body, msg.sender);
  // }
  // will leak three objects: foo, and the two implicit global objects, viz.
  // the GearsFactory and the GearsWorkerPool, inserted with the global names
  // kWorkerInsertedFactoryName and kWorkerInsertedWorkerPoolName. Note that
  // the object bound to the local variable bar is not leaked.
  //
  // To avoid leaking these objects, this code paragraph here first finds the
  // names of all global variables, and then sets them to (JavaScript) null,
  // reducing their COM ref-count and hence allowing them to be garbage
  // collected.
  //
  // This solution is less than ideal - the fundamental problem is probably
  // deeper, such as leaking the ActiveScript instance itself, but after some
  // effort, I (nigeltao) was insufficiently awesome to pinpoint where our
  // fundamental leak is, and as a hacky workaround, we instead clean up our
  // Gears modules (i.e. instances of ModuleImplBaseClass). We may still be
  // leaking memory upon worker exit, but at least we're not leaking anything
  // that we're explicitly tracking (via Gears' LEAK_COUNTER_XXX macros).
  IDispatch *dispatch = GetGlobalObject();
  std::vector<std::string16> names;
  HRESULT hr = ActiveXUtils::GetDispatchPropertyNames(dispatch, &names);
  if (FAILED(hr)) { return false; }
  std::string16 clean_up_script;

  std::vector<std::string16>::const_iterator it;
  for (it = names.begin(); it != names.end(); ++it) {
    clean_up_script += STRING16(L"var ");
    clean_up_script += *it;
    clean_up_script += STRING16(L"=null;");
  }
  return Eval(clean_up_script);
}

bool JsRunnerImpl::Stop() {
  // Check pointer because dtor calls Stop() regardless of whether Start()
  // happened.
  if (javascript_engine_) {
    if (!CleanUpJsGlobalVariables()) { return false; }
    javascript_engine_->Close();
  }
  return S_OK;
}

STDMETHODIMP JsRunnerImpl::LookupNamedItem(const OLECHAR *name,
                                           IUnknown **item) {
  IUnknown *found_item = global_name_to_object_[name];
  if (!found_item) { return TYPE_E_ELEMENTNOTFOUND; }

  // IActiveScript expects items coming into it to already be AddRef()'d. It
  // will Release() these references on IActiveScript.Close().
  // For an example of this, see: http://support.microsoft.com/kb/q221992/.
  found_item->AddRef();
  *item = found_item;
  return S_OK;
}


STDMETHODIMP JsRunnerImpl::HandleScriptError(EXCEPINFO *ei, ULONG line,
                                             LONG pos, BSTR src) {
  if (!error_handler_) { return E_FAIL; }

  const JsErrorInfo error_info = {
    line + 1,  // Reported lines start at zero.
    ei->bstrDescription ? static_cast<char16*>(ei->bstrDescription)
                        : STRING16(L"")
  };

  error_handler_->HandleError(error_info);
  return S_OK;
}


STDMETHODIMP JsRunnerImpl::ProcessUrlAction(DWORD action, BYTE *policy,
                                            DWORD policy_size, BYTE *context,
                                            DWORD context_size, DWORD flags,
                                            DWORD reserved) {
  // There are many different values of action that could conceivably be
  // asked about: http://msdn2.microsoft.com/en-us/library/ms537178.aspx.
  // Many of them probably don't apply to the scripting host alone, but there
  // is no documentation on which are used, so we just say no to everything to
  // be paranoid. We can whitelist things back if we find that necessary.
  //
  // TODO(aa): Consider whitelisting XMLHttpRequest. IE7 now has a global
  // XMLHttpRequest object, like Safari and Mozilla. I don't believe this
  // object "counts" as an ActiveX Control. If so, it seems like whitelisting
  // the ActiveX version might not matter. In any case, doing this would
  // require figuring out the parallel thing for Mozilla and figuring out how
  // to get XMLHttpRequest the context it needs to make decisions about same-
  // origin.
  *policy = URLPOLICY_DISALLOW;

  // MSDN says to return S_FALSE if you were successful, but don't want to
  // allow the action: http://msdn2.microsoft.com/en-us/library/ms537151.aspx.
  return S_FALSE;
}


STDMETHODIMP JsRunnerImpl::QueryCustomPolicy(REFGUID guid_key, BYTE **policy,
                                             DWORD *policy_size, BYTE *context,
                                             DWORD context_size,
                                             DWORD reserved) {
  // This method is only used when ProcessUrlAction allows ActiveXObjects to be
  // created. Since we always say no, we do not need to implement this method.
  return E_NOTIMPL;
}


STDMETHODIMP JsRunnerImpl::GetSecurityId(BYTE *security_id,
                                         DWORD *security_id_size,
                                         DWORD_PTR reserved) {
  // Again, not used in the configuration we use.
  return E_NOTIMPL;
}


// A wrapper class to AddRef/Release the internal COM object,
// while exposing a simpler new/delete interface to users.
class JsRunner : public JsRunnerBase {
 public:
  JsRunner() {
    LEAK_COUNTER_INCREMENT(JsRunner);
    HRESULT hr = CComObject<JsRunnerImpl>::CreateInstance(&com_obj_);
    if (com_obj_) {
      com_obj_->AddRef();  // MSDN says call AddRef after CreateInstance
    }
  }
  virtual ~JsRunner() {
    LEAK_COUNTER_DECREMENT(JsRunner);
    // Alert modules that the engine is unloading.
    SendEvent(JSEVENT_UNLOAD);

    if (com_obj_) {
      com_obj_->Stop();
      com_obj_->Release();
    }
  }

  void OnModuleEnvironmentAttach() {
    // No-op. A JsRunner is not self-deleting, unlike a DocumentJsRunner.
  }

  void OnModuleEnvironmentDetach() {
    // No-op. A JsRunner is not self-deleting, unlike a DocumentJsRunner.
  }

  IDispatch *GetGlobalObject(bool dump_on_error = false) {
    return com_obj_->GetGlobalObject(dump_on_error);
  }
  bool AddGlobal(const std::string16 &name, ModuleImplBaseClass *object) {
    VARIANT &variant = object->GetWrapperToken();
    assert(variant.vt == VT_DISPATCH);
    return com_obj_->AddGlobal(name, variant.pdispVal);
  }
  bool Start(const std::string16 &full_script) {
    return com_obj_->Start(full_script);
  }
  bool Stop() {
    return com_obj_->Stop();
  }
  bool Eval(const std::string16 &script) {
    return com_obj_->Eval(script);
  }
  void SetErrorHandler(JsErrorHandlerInterface *error_handler) {
    return com_obj_->SetErrorHandler(error_handler);
  }
  bool InvokeCallback(const JsRootedCallback *callback,
                      int argc, JsParamToSend *argv,
                      JsRootedToken **optional_alloc_retval) {
    return InvokeCallbackImpl(callback, argc, argv, optional_alloc_retval,
                              NULL);
  }

 private:
  CComObject<JsRunnerImpl> *com_obj_;

  DISALLOW_EVIL_CONSTRUCTORS(JsRunner);
};


// This class is a stub that is used to present a uniform interface to
// common functionality to both workers and the main thread.
class DocumentJsRunner : public JsRunnerBase {
 public:
  DocumentJsRunner(IUnknown *site)
      : site_(site),
        is_module_environment_detached_(true),
        is_unloaded_(true) {
    LEAK_COUNTER_INCREMENT(DocumentJsRunner);
  }

  virtual ~DocumentJsRunner() {
    LEAK_COUNTER_DECREMENT(DocumentJsRunner);
    assert(is_module_environment_detached_ && is_unloaded_);
  }

  void OnModuleEnvironmentAttach() {
    is_module_environment_detached_ = false;
  }

  void OnModuleEnvironmentDetach() {
    is_module_environment_detached_ = true;
    DeleteIfNoLongerRelevant();
  }

  IDispatch *GetGlobalObject(bool dump_on_error = false) {
    IDispatch *global_object;
    HRESULT hr = ActiveXUtils::GetScriptDispatch(site_, &global_object,
                                                 dump_on_error);
    if (FAILED(hr)) {
      if (dump_on_error) ExceptionManager::ReportAndContinue();
      return NULL;
    }
    return global_object;
  }

  bool AddGlobal(const std::string16 &name, ModuleImplBaseClass *object) {
    // TODO(zork): Add this functionality to DocumentJsRunner.
    return false;
  }

  bool Start(const std::string16 &full_script) {
    assert(false);  // Should not be called on the DocumentJsRunner.
    return false;
  }

  bool Stop() {
    assert(false);  // Should not be called on the DocumentJsRunner.
    return false;
  }

  bool EvalImpl(const std::string16 &script,
                std::string16 *optional_exception) {
    // There appears to be no way to execute script code directly on WinCE.
    // - IPIEHTMLWindow2 does not provide execScript.
    // - We have the script engine's IDispatch pointer but there's no way to get
    //   back to the IActiveScript object to use
    //   IActiveScriptParse::ParseScriptText.
    // Instead, we pass the script as a parameter to JavaScript's 'eval'
    // function. We use this approach on both Win32 and WinCE to prevent
    // branching.
    //
    // TODO(steveblock): Test the cost of using GetDispatchMemberId vs
    // execScript using Stopwatch.

    CComPtr<IDispatch> javascript_engine_dispatch = GetGlobalObject();
    if (!javascript_engine_dispatch) { return false; }
    DISPID function_iid;
    if (FAILED(ActiveXUtils::GetDispatchMemberId(javascript_engine_dispatch,
                                                 STRING16(L"eval"),
                                                 &function_iid))) {
      return false;
    }
    CComVariant script_variant(script.c_str());
    DISPPARAMS parameters = {0};
    parameters.cArgs = 1;
    parameters.rgvarg = &script_variant;
    EXCEPINFO exception;
    // If the JavaScript code being invoked throws an exception, Invoke returns
    // DISP_E_EXCEPTION, or the undocumented 0x800A139E if we don't pass an
    // EXECINFO pointer. See
    // http://asp3wiki.wrox.com/wiki/show/G.2.2-+Runtime+Errors for a
    // description of 0x800A139E.
    HRESULT hr = javascript_engine_dispatch->Invoke(
        function_iid,           // member to invoke
        IID_NULL,               // reserved
        LOCALE_SYSTEM_DEFAULT,  // TODO(cprince): should this be user default?
        DISPATCH_METHOD,        // dispatch/invoke as...
        &parameters,            // parameters
        NULL,                   // receives result (NULL okay)
        &exception,             // receives exception (NULL okay)
        NULL);                  // receives badarg index (NULL okay)
    if (FAILED(hr)) {
      if (DISP_E_EXCEPTION == hr && optional_exception) {
        *optional_exception = exception.bstrDescription;
      }
      return false;
    }
    return true;
  }

  bool Eval(const std::string16 &script) {
#ifdef WINCE
    // On WinCE, exceptions do not get thrown from JavaScript code that is
    // invoked from C++ in the context of the main page. Therefore, to allow
    // the user to handle these exceptions, we manually call window.onerror.
    std::string16 exception = L"";
    bool result = EvalImpl(script, &exception);
    // If the exception string is empty, the callback failed for some other
    // reason.
    if (!result && !exception.empty()) {
      CallWindowOnerror(this, EscapeMessage(exception));
    }
    return result;
#else
    return EvalImpl(script, NULL);
#endif
  }

  void SetErrorHandler(JsErrorHandlerInterface *handler) {
    assert(false);  // Should not be called on the DocumentJsRunner.
  }

  bool ListenForUnloadEvent() {
#ifdef WINCE
    // WinCE does not use HtmlEventMonitor to monitor page unloading.
#else
    // Create an HTML event monitor to send the unload event when the page
    // goes away.
    unload_monitor_.reset(new HtmlEventMonitor(kEventUnload,
                                               HandleEventUnload, this));
    CComPtr<IHTMLWindow3> event_source;
    if (FAILED(ActiveXUtils::GetHtmlWindow3(site_, &event_source))) {
      return false;
    }
    unload_monitor_->Start(event_source);
    is_unloaded_ = false;
#endif
    return true;
  }

  bool InvokeCallback(const JsRootedCallback *callback,
                      int argc, JsParamToSend *argv,
                      JsRootedToken **optional_alloc_retval) {
#ifdef WINCE
    // On WinCE, exceptions do not get thrown from JavaScript code that is
    // invoked from C++ in the context of the main page. Therefore, to allow
    // the user to handle these exceptions, we manually call window.onerror.
    std::string16 exception = L"";
    bool result = InvokeCallbackImpl(callback, argc, argv,
                                     optional_alloc_retval, &exception);
    // If the exception string is empty, the callback failed for some other
    // reason.
    if (!result && !exception.empty()) {
      CallWindowOnerror(this, EscapeMessage(exception));
    }
    return result;
#else
    return InvokeCallbackImpl(callback, argc, argv, optional_alloc_retval,
                              NULL);
#endif
  }

#ifdef WINCE
  // On WinCE, exceptions thrown from C++ don't trigger the default JS exception
  // handler, so we call window.onerror manually.
  virtual void ThrowGlobalError(const std::string16 &message) {
    CallWindowOnerror(this, message);
  }
#endif

 private:
  static void HandleEventUnload(void *user_param) {
    DocumentJsRunner *document_js_runner =
        static_cast<DocumentJsRunner*>(user_param);
    document_js_runner->SendEvent(JSEVENT_UNLOAD);
    document_js_runner->is_unloaded_ = true;
    document_js_runner->DeleteIfNoLongerRelevant();
  }

  void DeleteIfNoLongerRelevant() {
    if (is_module_environment_detached_ && is_unloaded_) {
      delete this;
    }
  }

#ifdef WINCE
  // WinCE does not use HtmlEventMonitor to monitor page unloading.
#else
  scoped_ptr<HtmlEventMonitor> unload_monitor_;  // For 'onunload' notifications
#endif
  CComPtr<IUnknown> site_;
  bool is_module_environment_detached_;
  bool is_unloaded_;

  DISALLOW_EVIL_CONSTRUCTORS(DocumentJsRunner);
};


JsRunnerInterface* NewJsRunner() {
  return static_cast<JsRunnerInterface*>(new JsRunner());
}

JsRunnerInterface* NewDocumentJsRunner(IUnknown *base, JsContextPtr) {
  scoped_ptr<DocumentJsRunner> document_js_runner(
      new DocumentJsRunner(base));
  if (!document_js_runner->ListenForUnloadEvent()) {
    return NULL;
  }
  // A DocumentJsRunner who has had ListenForUnloadEvent called successfully
  // is self-deleting, so we can release it from our scoped_ptr.
  return document_js_runner.release();
}
