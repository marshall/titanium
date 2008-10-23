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

#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsXPCOM.h>
#include <gecko_internal/nsIDOMClassInfo.h>
#include <gecko_internal/nsIXPConnect.h>

#include "gears/factory/factory_ff.h"

#include "gears/base/common/js_types.h"
#include "gears/base/common/module_wrapper.h"


// Boilerplate. == NS_IMPL_ISUPPORTS + ..._MAP_ENTRY_EXTERNAL_DOM_CLASSINFO
NS_IMPL_ADDREF(GearsFactory)
NS_IMPL_RELEASE(GearsFactory)
NS_INTERFACE_MAP_BEGIN(GearsFactory)
  NS_INTERFACE_MAP_ENTRY(GearsFactoryInterface)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, GearsFactoryInterface)
  NS_INTERFACE_MAP_ENTRY_EXTERNAL_DOM_CLASSINFO(GearsFactory)
NS_INTERFACE_MAP_END


const char *kGearsFactoryContractId = "@google.com/gears/factory;1";
const char *kGearsFactoryClassName = "GearsFactory";
const nsCID kGearsFactoryClassId = {0x93b2e433, 0x35ab, 0x46e7,
    {0xa9, 0x50, 0x41, 0x8f, 0x92, 0x2c, 0xc6, 0xef}};


NS_IMETHODIMP GearsFactory::InitFactoryFromDOM() {
  scoped_refptr<ModuleEnvironment> module_environment(
      ModuleEnvironment::CreateFromDOM());
  if (!module_environment ||
      !CreateModule<GearsFactoryImpl>(module_environment.get(),
                                      NULL, &factory_impl_)) {
    return NS_ERROR_FAILURE;
  }
  unload_monitor_.reset(new JsEventMonitor(module_environment->js_runner_,
                                           JSEVENT_UNLOAD, this));
  return NS_OK;
}


NS_IMETHODIMP GearsFactory::Create(nsISupports **retval) {
  return DelegateToFactoryImpl("create", false);
}


NS_IMETHODIMP GearsFactory::GetBuildInfo(nsAString &retval) {
  return DelegateToFactoryImpl("getBuildInfo", false);
}


NS_IMETHODIMP GearsFactory::GetHasPermission(PRBool *retval) {
  return DelegateToFactoryImpl("hasPermission", true);
}


NS_IMETHODIMP GearsFactory::GetPermission(PRBool *retval) {
  return DelegateToFactoryImpl("getPermission", false);
}


NS_IMETHODIMP GearsFactory::GetVersion(nsAString &retval) {
  return DelegateToFactoryImpl("version", true);
}


NS_IMETHODIMP GearsFactory::DelegateToFactoryImpl(const char *name,
                                                  bool is_property) {
  // If the factory no longer exists, fail early.
  if (!factory_impl_.get()) {
    return NS_ERROR_FAILURE;
  }

  nsresult nr;
  nsCOMPtr<nsIXPConnect> xpc =
      do_GetService("@mozilla.org/js/xpc/XPConnect;1", &nr);
  if (!xpc || NS_FAILED(nr)) { return NS_ERROR_FAILURE; }

#if BROWSER_FF3
  nsAXPCNativeCallContext *ncc = NULL;
  nr = xpc->GetCurrentNativeCallContext(&ncc);
#else
  nsCOMPtr<nsIXPCNativeCallContext> ncc;
  nr = xpc->GetCurrentNativeCallContext(getter_AddRefs(ncc));
#endif
  if (!ncc || NS_FAILED(nr)) { return NS_ERROR_FAILURE; }

  PRUint32 argc;
  JsToken *argv;
  JsContextPtr js_context;
  JsToken *retval;
  ncc->GetArgc(&argc);
  ncc->GetArgvPtr(&argv);
  ncc->GetJSContext(&js_context);
  ncc->GetRetValPtr(&retval);

  DispatcherInterface *dispatcher =
      factory_impl_->GetWrapper()->GetDispatcher();
  DispatchId dispatch_id = dispatcher->GetDispatchId(name);
  assert(dispatch_id != NULL);
  JsCallContext context(js_context, factory_impl_->GetJsRunner(),
                        static_cast<int>(argc), argv, retval);
  if (is_property) {
    if (!dispatcher->GetProperty(dispatch_id, &context)) {
      return NS_ERROR_FAILURE;
    }
  } else {
    if (!dispatcher->CallMethod(dispatch_id, &context)) {
      return NS_ERROR_FAILURE;
    }
  }
  if (context.is_exception_set()) {
    ncc->SetExceptionWasThrown(PR_TRUE);
  } else {
    if (!context.is_return_value_set()) {
      // Properties should always either throw an exception, or return a value.
      assert(!is_property);
      // We had a method call that didn't throw an exception, but instead
      // returned void. That's perfectly fine.
      *retval = JSVAL_VOID;
    }
    ncc->SetReturnValueWasSet(PR_TRUE);
  }
  return NS_OK;
}

void GearsFactory::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  factory_impl_.reset();
}
