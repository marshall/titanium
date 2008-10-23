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

#include "gears/ui/ie/html_dialog_bridge_iemobile.h"

#include <assert.h>
#include <oleidl.h>
#include <piedocvw.h>
#include <pvdispid.h>
#include <shlguid.h>
#include <webvw.h>
#include <windows.h>

#include "gears/base/common/common.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/ie/activex_utils.h"
#include "gears/base/ie/atl_headers.h"
#include "third_party/AtlActiveScriptSite.h"

// Set the permissions dialog object.
// window.external does not work on PIE, so we use this instead...
// basically we get the browser object used by the permissions dialog
// and traverse the dom to find a PIEDialogBridge instance. If there is one,
// we can safely set the dialog pointer.

HRESULT PIEDialogBridge::AccessPermissionsDialog() {
  // First we get the global permissions dialog object...
  // (permissions dialog is modal)

  CComQIPtr<PIEDialogHostInterface> permissions_dialog(
      HtmlDialogHost::html_permissions_dialog_);

  if (!permissions_dialog)
    return S_FALSE;

  HtmlDialogHost* html_dialog = HtmlDialogHost::html_permissions_dialog_;
  CComPtr<IPIEHTMLDocument2> document(html_dialog->document_);

  if (!document) return S_FALSE;

  // Then we retrieve all the elements in the document, and we look for us.
  CComPtr<IPIEHTMLElementCollection> html_collection;
  HRESULT hr = document->get_all(&html_collection);
  ASSERT(html_collection);

  long l_len = 0;
  hr = html_collection->get_length(&l_len);
  int len = static_cast<int> (l_len);

  if (hr != S_OK)
    return S_FALSE;

  for (int i = 0; i < len; i++) {
    VARIANT index;
    VariantInit(&index);
    index.vt = VT_I4;
    index.intVal = i;

    CComPtr<IDispatch> disp;
    hr = html_collection->item(index, index, &disp);

    if (disp) {
      // Get the current element.
      CComQIPtr<IPIEHTMLObjectElement> elem(disp);

      // If we have an object element, we check that it's a
      // PIEDialogBridge object; we only set one object per document.
      if (elem) {
        CComQIPtr<PIEDialogBridgeInterface> bridge_pointer;
        CComPtr<IDispatch> disp_object;
        hr = elem->get_object(&disp_object);
        bridge_pointer = disp_object;
        if (bridge_pointer) {
          bridge_pointer->SetDialog(permissions_dialog);
          return S_OK;
        }
      }
    }
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::SetDialog(PIEDialogHostInterface* dialog) {
  if (!dialog_) {
    dialog_ = dialog;
    if (dialog_) {
      return S_OK;
    } else {
      return S_FALSE;
    }
  }
  return S_OK;
}

HRESULT PIEDialogBridge::IsPocketIE(VARIANT_BOOL *retval) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    return dialog_->IsPocketIE(retval);
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::IsSmartPhone(VARIANT_BOOL *retval) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    return dialog_->IsSmartPhone(retval);
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::GetDialogArguments(BSTR *args_string) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    HRESULT hr = dialog_->GetDialogArguments(args_string);
    return hr;
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::CloseDialog(const BSTR result_string) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    return dialog_->CloseDialog(result_string);
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::ResizeDialog() {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    HRESULT hr = dialog_->ResizeDialog();
    return hr;
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::SetScriptContext(IDispatch *val) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    return dialog_->SetScriptContext(val);
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::SetButton(const BSTR label, const BSTR script) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    return dialog_->SetButton(label, script);
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::SetButtonEnabled(VARIANT_BOOL *val) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    return dialog_->SetButtonEnabled(val);
  }
  return S_FALSE;
}

HRESULT PIEDialogBridge::SetCancelButton(const BSTR label) {
  if (!dialog_) {
    AccessPermissionsDialog();
  }
  if (dialog_) {
    return dialog_->SetCancelButton(label);
  }
  return S_FALSE;
}
