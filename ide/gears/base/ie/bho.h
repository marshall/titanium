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

#ifndef GEARS_BASE_IE_BHO_H__
#define GEARS_BASE_IE_BHO_H__

#include "gears/base/ie/atl_headers.h"
#include "gears/base/ie/resource.h" // for .rgs resource ids (IDR_*)
#include "genfiles/interfaces.h"

class ATL_NO_VTABLE BrowserHelperObject
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<BrowserHelperObject, &CLSID_BrowserHelperObject>,
      public IObjectWithSiteImpl<BrowserHelperObject> {
 public:
  DECLARE_REGISTRY_RESOURCEID(IDR_BROWSERHELPEROBJECT)
  DECLARE_NOT_AGGREGATABLE(BrowserHelperObject)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(BrowserHelperObject)
    COM_INTERFACE_ENTRY(IObjectWithSite)
  END_COM_MAP()

  STDMETHOD(SetSite)(IUnknown *pUnkSite);
#ifdef WINCE
  static HWND GetBrowserWindow();
 private:
  static HWND browser_window_;
#endif
};
OBJECT_ENTRY_AUTO(__uuidof(BrowserHelperObject), BrowserHelperObject)

#endif  // GEARS_BASE_IE_BHO_H__
