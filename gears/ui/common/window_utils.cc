// Copyright 2008, Google Inc.
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

#include "gears/ui/common/window_utils.h"

#include "gears/base/common/base_class.h"

#if BROWSER_FF
#include "gears/base/firefox/dom_utils.h"
#elif BROWSER_IE
#include "gears/base/ie/activex_utils.h"
#ifdef WINCE
#include "gears/base/ie/bho.h"
#endif
#elif BROWSER_NPAPI
#include "gears/base/npapi/scoped_npapi_handles.h"
#endif

bool GetBrowserWindow(const ModuleImplBaseClass* module,
                      NativeWindowPtr* window) {
#if BROWSER_FF

  if (NS_OK != DOMUtils::GetNativeWindow(module->EnvPageJsContext(), window))
    return false;
  return true;

#elif BROWSER_WEBKIT
  // TODO(bpm): this causes Safari to hang
  return false;

  // Webkit doesn't support NPNVnetscapeWindow on MacOSX, so we have to do
  // something more hackish.  In this case, give the JS page the keyboard focus,
  // and then ask the OS for the window which has the keyboard focus.
  ScopedNPObject js_window;
  if (NPERR_NO_ERROR != NPN_GetValue(module->EnvPageJsContext(),
                                     NPNVWindowNPObject,
                                     as_out_parameter(js_window)))
    return false;
  std::string script_utf8("window.focus()");
  NPString script = {script_utf8.data(), script_utf8.length()};
  ScopedNPVariant result;
  if (!NPN_Evaluate(module->EnvPageJsContext(), js_window.get(), &script,
                    &result))
    return false;
  *window = GetKeyWindow();
  return (NULL != *window);

#elif BROWSER_NPAPI

  if (NPERR_NO_ERROR != NPN_GetValue(module->EnvPageJsContext(),
                                     NPNVnetscapeWindow,
                                     window))
    return false;
  return true;

#elif BROWSER_IE

#ifdef WINCE
  // On WinCE, only the BHO has an IWebBrowser2 pointer to the browser.
  *window = BrowserHelperObject::GetBrowserWindow();
  return NULL != *window;
#else  // !WINCE
  IWebBrowser2* web_browser = NULL;
  HRESULT hr = ActiveXUtils::GetWebBrowser2(module->EnvPageIUnknownSite(),
                                            &web_browser);
  if (FAILED(hr))
    return false;
  hr = web_browser->get_HWND(reinterpret_cast<long*>(window));
  if (FAILED(hr)) {
    web_browser->Release();
    return false;
  }
  web_browser->Release();
  return true;
#endif  // !WINCE

#endif  // BROWSER_IE

  return false;
}

