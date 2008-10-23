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
//
// A collection of static utility methods.

#ifndef GEARS_BASE_IE_ACTIVEX_UTILS_H__
#define GEARS_BASE_IE_ACTIVEX_UTILS_H__

#include <vector>
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "gears/base/ie/atl_headers.h"

struct IHTMLElement;
class BrowsingContext;
class SecurityOrigin;

#ifdef WINCE
struct IWebBrowser2;
struct IPIEHTMLDocument;
struct IPIEHTMLDocument2;
struct IPIEHTMLWindow2;
typedef IPIEHTMLDocument IHTMLDocument;
typedef IPIEHTMLDocument2 IHTMLDocument2;
// WinMo 5 SDK defines IHTMLWindow2, so we can't typedef IPIEHTMLWindow2.
#endif

class ActiveXUtils {
 public:
  // Returns the location url of the containing webBrowser as determined by
  // IWebBrowser2.LocationURL.
  // TODO(cprince): across the codebase, change PageOrigin to PageSecurityOrigin
  // and PageLocation to PageLocationUrl, for consistency.
  static bool GetPageLocation(IUnknown *site, std::string16 *page_location_url);
  static bool GetPageOrigin(IUnknown *site, SecurityOrigin *security_origin);

  // Returns the page's browsing context.
  // Returns true on success.
  static bool GetPageBrowsingContext(
      IUnknown *site, scoped_refptr<BrowsingContext> *browsing_context);

#ifdef WINCE
  // We're not able to get IWebBrowser2 on WinCE. Instead we obtain the window
  // and document objects from the script engine's IDispatch pointer.
#else
  // Returns the IWebBrowser2 interface corresponding to the given site.
  static HRESULT GetWebBrowser2(IUnknown *site,
                                IWebBrowser2 **browser2);
#endif

  // Returns the IHTMLDocument2 interface corresponding to the given site.
  static HRESULT GetHtmlDocument2(IUnknown *site,
                                  IHTMLDocument2 **document2);

  // Returns the IHTMLWindow2 interface corresponding to the given site.
  static HRESULT GetHtmlWindow2(IUnknown *site,
#ifdef WINCE
                                IPIEHTMLWindow2 **window2);
#else
                                IHTMLWindow2 **window2);
#endif

#ifdef WINCE
  // WinCE does not provide I(PIE)HTMLWindow3, but we do not need it.
#else
  // Returns the IHTMLWindow3 interface corresponding to the given site.
  // Can be used with our HtmlEventMonitor.
  static HRESULT GetHtmlWindow3(IUnknown *site,
                                IHTMLWindow3 **window3);
#endif

  // Returns the IDispatch interface for the script engine at the given site.
  // TODO(zork): Remove dump_on_error.
  static HRESULT GetScriptDispatch(IUnknown *site,
                                   IDispatch **script_dispatch,
                                   bool dump_on_error = false);

  // Returns the dispatch id of the named member.
  static HRESULT GetDispatchMemberId(IDispatch *dispatch, const WCHAR *name,
                                     DISPID *dispid) {
    *dispid = DISPID_UNKNOWN;
    return dispatch->GetIDsOfNames(IID_NULL, const_cast<WCHAR**>(&name), 1, 0,
                                   dispid);
  }

  // Appends to the vector out the names of dispatch's properties.
  static HRESULT GetDispatchPropertyNames(IDispatch *dispatch,
                                          std::vector<std::string16> *out);

  // Returns the property value by name.
  // The caller is responsible for freeing the returned VARIANT.
  static HRESULT GetDispatchProperty(IDispatch *dispatch, const WCHAR *name,
                                     VARIANT *value) {
    DISPID dispid;
    HRESULT hr = GetDispatchMemberId(dispatch, name, &dispid);
    if (FAILED(hr)) return hr;
    return GetDispatchProperty(dispatch, dispid, value);
  }

  // Returns the property value by dispid.
  // The caller is responsible for freeing the returned VARIANT.
  static HRESULT GetDispatchProperty(IDispatch *dispatch, DISPID dispid,
                                     VARIANT *value);

  // Sets the property value by name.
  static HRESULT SetDispatchProperty(IDispatch *dispatch, const WCHAR *name,
                                     const VARIANT *value) {
    DISPID dispid;
    HRESULT hr = GetDispatchMemberId(dispatch, name, &dispid);
    if (FAILED(hr)) return hr;
    return SetDispatchProperty(dispatch, dispid, value);
  }

  // Sets the property value by dispid.
  static HRESULT SetDispatchProperty(IDispatch *dispatch, DISPID dispid,
                                     const VARIANT *value);

  // Adds a new property to object.
  // If the property already exists then it is returned.
  // Parameters:
  //  dispatch - in - the properties are added to this object
  //  name - in - the case sensitive name of the property
  //  dispid - out - the identifier for the property
  static HRESULT AddDispatchProperty(IDispatch* dispatch, const char16* name,
                                     DISPID* dispid);

  // Convenience function for calling AddDispatchProperty
  // and then SetDispathProperty.
  static HRESULT AddAndSetDispatchProperty(IDispatch* dispatch,
                                           const char16* name,
                                           const VARIANT* value);

#ifdef WINCE
  // TODO(andreip): implement on Windows Mobile.
#else
  // Returns the html attribute value of the given HTMLElement.
  // The caller is responsible for freeing the returned VARIANT.
  static HRESULT GetHTMLElementAttributeValue(IHTMLElement *element,
                                              const WCHAR *name,
                                              VARIANT *value);
#endif

  // Convert NULL BSTR to the empty string.
  static const CComBSTR kEmptyBSTR;
  static const BSTR SafeBSTR(const BSTR value) {
    return value ? value : kEmptyBSTR.m_str;
  }
  // Returns true if there the browser is in 'online' mode and the local
  // system is connected to a network.
  static bool IsOnline();
};


#endif  // GEARS_BASE_IE_ACTIVEX_UTILS_H__
