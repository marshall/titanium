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

#ifdef WINCE
// FileSubmitter is not implemented for WinCE.
#else

#include "gears/localserver/ie/file_submit_behavior.h"

const wchar_t *SubmitFileBehavior::kName_DispName = L"name";


//------------------------------------------------------------------------------
// InitFromBehaviorFactory
//------------------------------------------------------------------------------
void SubmitFileBehavior::InitFromBehaviorFactory(std::string16 &filename) {
  filename_ = filename.c_str();
}


//------------------------------------------------------------------------------
// IElementBehavior::Init
//------------------------------------------------------------------------------
HRESULT SubmitFileBehavior::Init(IElementBehaviorSite *behavior_site) {
  behavior_site_ = behavior_site;

  // Don't allow this behavior to be attached to <input> elements. IE does
  // not call GetSubmitInfo() for behaviors attached to them.
  CComPtr<IHTMLElement> element;
  HRESULT hr = GetHTMLElement(&element);
  if (FAILED(hr)) {
    return hr;
  }
  CComQIPtr<IHTMLInputElement> input(element);
  if (input) {
    LOG16((L"SubmitFileBehavior::Init"
           L" - cannot attach to an <input> element\n"));
    return E_FAIL;
  }

  return S_OK;
}


//------------------------------------------------------------------------------
// IElementBehavior::Detach
//------------------------------------------------------------------------------
HRESULT SubmitFileBehavior::Detach(void) {
  Reset();
  behavior_site_.Release();
  return S_OK;
}


//------------------------------------------------------------------------------
// IElementBehavior::Notify
//------------------------------------------------------------------------------
HRESULT SubmitFileBehavior::Notify(long lEvent, VARIANT *var) {
  if (!behavior_site_) {
    return E_UNEXPECTED;
  }

  switch (lEvent) {
    // End tag of element has been parsed (we can get at attributes)
    case BEHAVIOREVENT_CONTENTREADY: {
      CComVariant name_value;
      HRESULT hr = GetHTMLElementAttributeValue(kName_DispName,
                                                &name_value);
      if (SUCCEEDED(hr) && (name_value.vt == VT_BSTR)) {
        name_ = name_value.bstrVal;
      }
      return S_OK;
    }

    // HTML document has been parsed (we can get at the document object model)
    case BEHAVIOREVENT_DOCUMENTREADY:
      return S_OK;

    default:
      return S_OK;
  }
}


//------------------------------------------------------------------------------
// IElementBehaviorSubmit::Reset
//------------------------------------------------------------------------------
HRESULT SubmitFileBehavior::Reset(void) {
  return S_OK;
}


//------------------------------------------------------------------------------
// IElementBehaviorSubmit::GetSubmitInfo
//------------------------------------------------------------------------------
HRESULT SubmitFileBehavior::GetSubmitInfo(IHTMLSubmitData *submit_data) {
  if (!name_ || !filename_) {
    return S_OK;
  }
  return submit_data->appendNameFilePair(name_, filename_);
}


//------------------------------------------------------------------------------
// IDispatch::GetIDsOfNames
// Implemented to look up the IDs of the properties exposed by this behavior.
//------------------------------------------------------------------------------
HRESULT SubmitFileBehavior::GetIDsOfNames(REFIID riid,
                                          LPOLESTR *names,
                                          UINT num_names,
                                          LCID lcid,
                                          DISPID *dispids) {
  if (!names || !dispids) {
    return E_POINTER;
  }

  if (num_names == 0) {
    return E_INVALIDARG;  // An odd case
  }

  HRESULT hr = S_OK;

  if (_wcsicmp(*names, kName_DispName) == 0) {
    *dispids++ = kName_DispId;
    hr = (num_names == 1) ? S_OK : DISP_E_UNKNOWNNAME;
  } else {
    *dispids++ = DISPID_UNKNOWN;
    hr = DISP_E_UNKNOWNNAME;
  }

  for (UINT i = 1; i < num_names; ++i) {
    *dispids++ = DISPID_UNKNOWN;
  }

  return hr;
}

//------------------------------------------------------------------------------
// IDispatch::Invoke
// Implemented to get/set the properties exposed by this behavior.
//------------------------------------------------------------------------------
HRESULT SubmitFileBehavior::Invoke(DISPID dispid,
                                   REFIID riid,
                                   LCID lcid,
                                   WORD flags,
                                   DISPPARAMS *params,
                                   VARIANT* result,
                                   EXCEPINFO* exceptinfo,
                                   unsigned int *argerr) {
  // Property Put
  if (flags & DISPATCH_PROPERTYPUT) {
    if (!params) return E_POINTER;
    if ((params->cArgs != 1)
        || !params->rgvarg
        || (VT_BSTR != params->rgvarg->vt)) {
      return E_INVALIDARG;
    }

    if (dispid == kName_DispId) {
      name_ = params->rgvarg->bstrVal;
      return S_OK;
    } else {
      return DISP_E_MEMBERNOTFOUND;
    }

  // Property Get
  } else if (flags & DISPATCH_PROPERTYGET) {
    if (!result) return E_POINTER;
    VariantInit(result);

    if (dispid == kName_DispId) {
      return name_.CopyTo(result);
    } else {
      return DISP_E_MEMBERNOTFOUND;
    }

  // Caller is trying to invoke our property as a method
  } else {
    return DISP_E_MEMBERNOTFOUND;
  }
}

#endif  // WINCE
