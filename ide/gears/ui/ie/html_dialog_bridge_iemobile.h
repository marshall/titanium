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

#ifndef GEARS_UI_IE_HTML_DIALOG_BRIDGE_IEMOBILE_H__
#define GEARS_UI_IE_HTML_DIALOG_BRIDGE_IEMOBILE_H__

#ifdef WINCE

#include <piedocvw.h>
#include "gears/base/common/base_class.h"
#include "gears/base/ie/resource.h"  // for .rgs resource ids (IDR_*)
#include "gears/ui/ie/html_dialog_host_iemobile.h" 

#include "genfiles/interfaces.h"
#include "gears/base/common/common.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/ui/ie/ui_resources.h"

#include "gears/factory/factory_utils.h"

class ATL_NO_VTABLE PIEDialogBridge
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComControl<PIEDialogBridge>,
      public CComCoClass<PIEDialogBridge, &CLSID_PIEDialogBridge>,
      public IObjectWithSiteImpl <PIEDialogBridge>,
      public IObjectSafetyImpl<PIEDialogBridge,
                               INTERFACESAFE_FOR_UNTRUSTED_CALLER +
                               INTERFACESAFE_FOR_UNTRUSTED_DATA>,
      public IDispatchImpl<PIEDialogBridgeInterface> {
 public:
  BEGIN_COM_MAP(PIEDialogBridge)
    COM_INTERFACE_ENTRY(PIEDialogBridgeInterface)
    COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

  DECLARE_PROTECT_FINAL_CONSTRUCT()
  DECLARE_REGISTRY_RESOURCEID(IDR_PIEDIALOGBRIDGE)

  PIEDialogBridge() {}

  // Called by the bridge to set the dialog object in the activex
  STDMETHODIMP SetDialog(PIEDialogHostInterface *val);

  // Called by script to check if we are in Pocket IE or Desktop IE
  STDMETHODIMP IsPocketIE(VARIANT_BOOL *retval);

  // Called by script to check if we are on a Smartphone.
  STDMETHODIMP IsSmartPhone(VARIANT_BOOL *retval);

  // Called by script to get dialog arguments.
  STDMETHODIMP GetDialogArguments(BSTR *args_string);

  // Called by script inside the dialog to close and send back result.
  STDMETHODIMP CloseDialog(const BSTR result_string);

  // Used by the script to resize the dialog on Pocket IE
  STDMETHODIMP ResizeDialog();

  STDMETHODIMP SetScriptContext(IDispatch* val);

  // Used by the script to set an action on the Allow button
  STDMETHODIMP SetButton(const BSTR label, const BSTR script);

  // Used by the script to enable/disable the Allow button
  STDMETHODIMP SetButtonEnabled(VARIANT_BOOL *val);

  // Used by javascript to set the cancel button label.
  STDMETHODIMP SetCancelButton(const BSTR label);

 private:

  // Set the dialog object in ActiveX by traversing the DOM.
  STDMETHODIMP AccessPermissionsDialog();

  CComQIPtr<PIEDialogHostInterface> dialog_;

  DISALLOW_EVIL_CONSTRUCTORS(PIEDialogBridge);
};
OBJECT_ENTRY_AUTO(__uuidof(PIEDialogBridge), PIEDialogBridge)

#endif

#endif  // GEARS_UI_IE_HTML_DIALOG_BRIDGE_IEMOBILE_H__
