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

#include <assert.h>
#ifdef WINCE
#include <piedocvw.h>
#include <webvw.h>
#else
#include <mshtml.h>
#include <sensapi.h>
#endif
#include <shlguid.h>      // for SID_SWebBrowserApp (except on WINCE)
#include <wininet.h>
#include "gears/base/common/browsing_context.h"
#include "gears/base/common/common.h"
#include "gears/base/common/exception_handler.h"
#include "gears/base/common/security_model.h"
#include "gears/base/ie/activex_utils.h"
#include <dispex.h>  // for IDispatchEx
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"
#endif

const CComBSTR ActiveXUtils::kEmptyBSTR(STRING16(L""));

bool ActiveXUtils::GetPageLocation(IUnknown *site,
                                   std::string16 *page_location_url) {
  assert(site);
  assert(page_location_url);

  HRESULT hr;
  CComBSTR location;

#ifdef WINCE
  CComPtr<IHTMLDocument2> document2;
  hr = GetHtmlDocument2(site, &document2);
  if (FAILED(hr) || !document2) { return false; }
  hr = document2->get_URL(&location);
  if (FAILED(hr)) { return false; }
#else
  CComPtr<IWebBrowser2> web_browser2;
  hr = GetWebBrowser2(site, &web_browser2);
  if (FAILED(hr) || !web_browser2) { return false; }
  hr = web_browser2->get_LocationURL(&location);
  if (FAILED(hr)) { return false; }
#endif
  *page_location_url = location.m_str;
  return true;
}


bool ActiveXUtils::GetPageOrigin(IUnknown *site,
                                 SecurityOrigin *security_origin) {
  std::string16 location;
  if (!GetPageLocation(site, &location))
    return false;
  return security_origin->InitFromUrl(location.c_str());
}

bool ActiveXUtils::GetPageBrowsingContext(
   IUnknown *site, scoped_refptr<BrowsingContext> *browsing_context) {
  // TODO(mpcomplete): implement me.
  browsing_context->reset();
  return true;
}

#ifdef WINCE
// We can't get IWebBrowser2 for WinCE.
#else
HRESULT ActiveXUtils::GetWebBrowser2(IUnknown *site, IWebBrowser2 **browser2) {
  CComQIPtr<IServiceProvider> service_provider = site;
  if (!service_provider) { return E_FAIL; }

  return service_provider->QueryService(SID_SWebBrowserApp,
                                        IID_IWebBrowser2,
                                        reinterpret_cast<void**>(browser2));
}
#endif


HRESULT ActiveXUtils::GetHtmlDocument2(IUnknown *site,
                                       IHTMLDocument2 **document2) {
  HRESULT hr;

#ifdef WINCE
  // Follow path Window2 -> Window -> Document -> Document2
  CComPtr<IPIEHTMLWindow2> window2;
  hr = GetHtmlWindow2(site, &window2);
  if (FAILED(hr) || !window2) { return false; }
  CComQIPtr<IPIEHTMLWindow> window = window2;
  CComPtr<IHTMLDocument> document;
  hr = window->get_document(&document);
  if (FAILED(hr) || !document) { return E_FAIL; }
  return document->QueryInterface(__uuidof(*document2),
                                  reinterpret_cast<void**>(document2));
#else
  CComPtr<IWebBrowser2> web_browser2;
  hr = GetWebBrowser2(site, &web_browser2);
  if (FAILED(hr) || !web_browser2) { return E_FAIL; }

  CComPtr<IDispatch> doc_dispatch;
  hr = web_browser2->get_Document(&doc_dispatch);
  if (FAILED(hr) || !doc_dispatch) { return E_FAIL; }

  return doc_dispatch->QueryInterface(document2);
#endif
}


HRESULT ActiveXUtils::GetHtmlWindow2(IUnknown *site,
#ifdef WINCE
                                     IPIEHTMLWindow2 **window2) {
  // site is javascript IDispatch pointer.
  return site->QueryInterface(__uuidof(*window2),
                              reinterpret_cast<void**>(window2));
#else
                                     IHTMLWindow2 **window2) {
  CComPtr<IHTMLDocument2> html_document2;
  // To hook an event on a page's window object, follow the path
  // IWebBrowser2->document->parentWindow->IHTMLWindow2

  HRESULT hr = GetHtmlDocument2(site, &html_document2);
  if (FAILED(hr) || !html_document2) { return E_FAIL; }

  return html_document2->get_parentWindow(window2);
#endif
}


#ifdef WINCE
// WinCE does not provide I(PIE)HTMLWindow3, but we do not need it.
#else
HRESULT ActiveXUtils::GetHtmlWindow3(IUnknown *site, IHTMLWindow3 **window3) {
  CComPtr<IHTMLWindow2> html_window2;
  HRESULT hr = GetHtmlWindow2(site, &html_window2);
  if (FAILED(hr) || !html_window2) { return E_FAIL; }

  return html_window2->QueryInterface(window3);
}
#endif


HRESULT ActiveXUtils::GetScriptDispatch(IUnknown *site,
                                        IDispatch **script_dispatch,
                                        bool dump_on_error) {
#if BROWSER_NPAPI
  // not used in NPAPI.
  // TODO(mpcomplete): clean this up.
  return E_FAIL;
#elif WINCE
  // site is JavaScript IDispatch pointer.
  return site->QueryInterface(script_dispatch);
#else
  CComPtr<IHTMLDocument2> html_document2;
  HRESULT hr = GetHtmlDocument2(site, &html_document2);
  if (FAILED(hr) || !html_document2) {
    if (dump_on_error) ExceptionManager::ReportAndContinue();
    return E_FAIL;
  }

  CComQIPtr<IHTMLDocument> html_document = html_document2;
  assert(html_document);

  return html_document->get_Script(script_dispatch);
#endif
}


HRESULT ActiveXUtils::GetDispatchPropertyNames(
    IDispatch *dispatch, std::vector<std::string16> *out) {
  CComQIPtr<IDispatchEx> dispatchex = dispatch;
  if (!dispatchex)
    return E_NOINTERFACE;

  HRESULT hr;
  BSTR bstrName;
  DISPID dispid;
  hr = dispatchex->GetNextDispID(fdexEnumDefault, DISPID_STARTENUM, &dispid);
  while (hr == NOERROR) {
    hr = dispatchex->GetMemberName(dispid, &bstrName);
    out->push_back(std::string16(bstrName));
    SysFreeString(bstrName);
    hr = dispatchex->GetNextDispID(fdexEnumDefault, dispid, &dispid);
  }
  return S_OK;
}


HRESULT ActiveXUtils::GetDispatchProperty(IDispatch *dispatch,
                                          DISPID dispid,
                                          VARIANT *value) {
  assert(dispatch);
  assert(value);
  DISPPARAMS dispparamsNoArgs = { NULL, NULL, 0, 0 };
  return dispatch->Invoke(dispid, IID_NULL,
                          LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                          &dispparamsNoArgs, value, NULL, NULL);
}


HRESULT ActiveXUtils::SetDispatchProperty(IDispatch *dispatch,
                                          DISPID dispid,
                                          const VARIANT *value) {
  assert(dispatch);
  assert(value);
  DISPPARAMS dispparams = {NULL, NULL, 1, 1};
  dispparams.rgvarg = const_cast<VARIANT*>(value);
  DISPID dispidPut = DISPID_PROPERTYPUT;
  dispparams.rgdispidNamedArgs = &dispidPut;
  WORD flags = (value->vt == VT_DISPATCH)
    ? DISPATCH_PROPERTYPUTREF
    : DISPATCH_PROPERTYPUT;
  return dispatch->Invoke(dispid, IID_NULL,
                          LOCALE_USER_DEFAULT, flags,
                          &dispparams, NULL, NULL, NULL);
}

HRESULT ActiveXUtils::AddDispatchProperty(IDispatch* dispatch,
                                            const char16* name,
                                            DISPID* dispid) {
  CComQIPtr<IDispatchEx> dispatchex = dispatch;
  if (!dispatchex)
    return E_NOINTERFACE;

  return dispatchex->GetDispID(
    CComBSTR(name), fdexNameCaseSensitive | fdexNameEnsure, dispid);
}

HRESULT ActiveXUtils::AddAndSetDispatchProperty(IDispatch* dispatch,
                                                const char16* name,
                                                const VARIANT *value) {
  DISPID dispid;
  HRESULT hr = AddDispatchProperty(dispatch, name, &dispid);
  if (FAILED(hr))
    return hr;

  return SetDispatchProperty(dispatch, dispid, value);
}

#ifdef WINCE
// TODO(andreip): Implement on Windows Mobile
#else
HRESULT ActiveXUtils::GetHTMLElementAttributeValue(IHTMLElement *element,
                                                   const WCHAR *name,
                                                   VARIANT *value) {
  assert(element);
  assert(name);
  assert(value);

  // Get the DOM node from the element
  CComQIPtr<IHTMLDOMNode> node(element);
  if (!node) {
    return E_NOINTERFACE;
  }

  // Get the attributes collection from the node
  CComPtr<IDispatch> collection_disp;
  HRESULT hr = node->get_attributes(&collection_disp);
  if (FAILED(hr)) {
    return hr;
  }
  CComQIPtr<IHTMLAttributeCollection> collection(collection_disp);
  if (!collection) {
    return E_NOINTERFACE;
  }

  // Get the attribute we're interested in by name
  CComVariant name_var(name);
  CComPtr<IDispatch> attribute_disp;
  hr = collection->item(&name_var, &attribute_disp);
  if (FAILED(hr)) {
    return hr;
  }

  // Get the value from the attribute
  CComQIPtr<IHTMLDOMAttribute> attribute(attribute_disp);
  if (!attribute) {
    return E_NOINTERFACE;
  }
  return attribute->get_nodeValue(value);
}
#endif


bool ActiveXUtils::IsOnline() {
  // Note: InternetGetConnectedState detects IE's workoffline mode.
  DWORD connected_state_flags_out = 0;
  DWORD network_alive_flags_out = 0;
  BOOL connected = InternetGetConnectedState(&connected_state_flags_out, 0);
  BOOL alive = IsNetworkAlive(&network_alive_flags_out);
  return connected && alive &&
         ((connected_state_flags_out & INTERNET_CONNECTION_OFFLINE) == 0);
}
