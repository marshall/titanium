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

#include "gears/factory/factory_ie.h"

#include "gears/base/ie/dispatcher_to_idispatch.h"
#include "gears/factory/factory_utils.h"

#ifdef WINCE
// On regular Windows, the factory_impl_ is created in SetSite. On WinCE,
// we need to explicitly call privateSetGlobalObject on the factory object,
// and this kPrivateSetGlobalObject string plays the role of a DISPID for the
// privateSetGlobalObject method.
// A DISPID is just a LONG. On WinCE, we'll just re-use the address of the
// the string objects, the same way Dispatcher just re-uses a DispatchId (which
// is just a pointer) as a DISPID.
// Also, gears_init.js checks getBuildInfo to see whether or not we are on
// WinCE (and hence whether or not to call privateSetGlobalObject), so we also
// need to be able to respond to getBuildInfo *before* the factory_impl_ is
// initialized.
// Finally, if we try to call create on a GearsFactory that hasn't had
// privateSetGlobalObject called on it yet, then we should return a helpful
// exception message, and kUninitializedGearsFactoryImpl plays the DISPID in
// that case.
const std::string16 GearsFactory::kGetBuildInfo(STRING16(L"getBuildInfo"));
const std::string16 GearsFactory::kPrivateSetGlobalObject(
    STRING16(L"privateSetGlobalObject"));
const std::string16 GearsFactory::kUninitializedGearsFactoryImpl(
    STRING16(L"uninitializedGearsFactoryImpl"));
#endif


// ModuleEnvironment::CreateFromDOM needs the object to be sited.  We override
// SetSite just to know when this happens, so we can do some one-time setup
// afterward.
STDMETHODIMP GearsFactory::SetSite(IUnknown *site) {
  HRESULT hr = IObjectWithSiteImpl<GearsFactory>::SetSite(site);
#ifdef WINCE
  // See comments in GearsFactory::Invoke for why this is disabled for WinCE.
#else
  scoped_refptr<ModuleEnvironment> module_environment(
      ModuleEnvironment::CreateFromDOM(m_spUnkSite));
  if (!module_environment ||
      !CreateModule<GearsFactoryImpl>(module_environment.get(),
                                      NULL, &factory_impl_)) {
    return E_FAIL;
  }
#endif
  return hr;
}


HRESULT GearsFactory::GetTypeInfoCount(unsigned int FAR* retval) {
  if (!factory_impl_.get()) { return E_FAIL; }
  return DispatcherGetTypeInfoCount(
      factory_impl_->GetWrapper()->GetDispatcher(), retval);
}


HRESULT GearsFactory::GetTypeInfo(unsigned int index, LCID lcid,
                                  ITypeInfo FAR* FAR* retval) {
  if (!factory_impl_.get()) { return E_FAIL; }
  return DispatcherGetTypeInfo(factory_impl_->GetWrapper()->GetDispatcher(),
                               index, lcid, retval);
}


HRESULT GearsFactory::GetIDsOfNames(REFIID iid, OLECHAR FAR* FAR* names,
                                    unsigned int num_names, LCID lcid, 
                                    DISPID FAR* retval) {
  if (!factory_impl_.get()) {
#ifdef WINCE
    std::string16 name_as_string(static_cast<char16 *>(*names));
    if (kPrivateSetGlobalObject == name_as_string) {
      *retval = reinterpret_cast<DISPID>(&kPrivateSetGlobalObject);
      return S_OK;
    } else if (kGetBuildInfo == name_as_string) {
      *retval = reinterpret_cast<DISPID>(&kGetBuildInfo);
      return S_OK;
    } else {
      *retval = reinterpret_cast<DISPID>(&kUninitializedGearsFactoryImpl);
      return S_OK;
    }
#else
    return E_FAIL;
#endif
  }
  return DispatcherGetIDsOfNames(factory_impl_->GetWrapper()->GetDispatcher(),
                                 iid, names, num_names, lcid, retval);
}


HRESULT GearsFactory::Invoke(DISPID member_id, REFIID iid, LCID lcid,
                             WORD flags, DISPPARAMS FAR* params,
                             VARIANT FAR* retval, EXCEPINFO FAR* exception,
                             unsigned int FAR* arg_error_index) {
  if (!factory_impl_.get()) {
#ifdef WINCE
    // On WinCE, we intercept the privateSetGlobalObject method call. On
    // regular Windows, this is done during SetSite, but on WinCE we are not
    // able to get the necessary IWebBrowser2 from the IUnknown *site, and
    // instead initialize via an explicit JS call to privateSetGlobalObject.
    //
    // We can't implement privateSetGlobalObject in GearsFactoryImpl and rely
    // on the Dispatcher to dispatch the call, since we don't have a
    // GearsFactoryImpl instance yet, since CreateModule<GearsFactoryImpl>
    // requires a valid ModuleEnvironment. Thus, we explicitly intercept the
    // privateSetGlobalObject method call and initialize the ModuleEnvironment
    // here.
    //
    // TODO(steveblock): Ideally, we would avoid the need for this method and
    // somehow pass this pointer to the GearsFactory constructor. This could
    // either be through ActiveXObject() or through the OBJECT tag's parameters.
    // However, it looks like neither approach is possible.
    if (member_id == reinterpret_cast<DISPID>(&kPrivateSetGlobalObject)) {
      if (factory_impl_.get()) {
        // privateSetGlobalObject has already been called, and so subsequent
        // calls are just a no-op.
        return S_OK;
      }
      VARIANT variant;
      JsCallContext js_call_context(params, retval, exception);
      const int argc = 1;
      JsArgument argv[argc] = {
        { JSPARAM_REQUIRED, JSPARAM_TOKEN, &variant },
      };
      js_call_context.GetArguments(argc, argv);
      if (js_call_context.is_exception_set() ||
          variant.vt != VT_DISPATCH) {
        return DISP_E_EXCEPTION;
      }
      scoped_refptr<ModuleEnvironment> module_environment(
          ModuleEnvironment::CreateFromDOM(variant.pdispVal));
      if (!module_environment ||
          !CreateModule<GearsFactoryImpl>(module_environment.get(),
                                          NULL, &factory_impl_)) {
        js_call_context.SetException(STRING16(
            L"Failed to initialize " PRODUCT_FRIENDLY_NAME L"."));
        return DISP_E_EXCEPTION;
      }
      return S_OK;
    } else if (member_id == reinterpret_cast<DISPID>(&kGetBuildInfo)) {
      std::string16 build_info;
      AppendBuildInfo(&build_info);
      JsCallContext js_call_context(params, retval, exception);
      js_call_context.SetReturnValue(JSPARAM_STRING16, &build_info);
      return S_OK;
    } else {
      JsCallContext js_call_context(params, retval, exception);
      js_call_context.SetException(STRING16(PRODUCT_FRIENDLY_NAME
          L" has not been initialized correctly. Please ensure that"
          L" you are using the most recent version of gears_init.js."));
      return DISP_E_EXCEPTION;
    }
#else
    return E_FAIL;
#endif
  }
  return DispatcherInvoke(factory_impl_->GetWrapper()->GetDispatcher(),
                          member_id, iid, lcid, flags, params, retval,
                          exception, arg_error_index);
}
