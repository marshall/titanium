// Copyright 2005, Google Inc.
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

#ifndef GEARS_BASE_FIREFOX_DOM_UTILS_H__
#define GEARS_BASE_FIREFOX_DOM_UTILS_H__

#include "gears/base/common/common_ff.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "gears/ui/common/window_utils.h"  // for NativeWindowPtr

struct JSContext;
class nsIDOMWindowInternal;
class nsIScriptContext;
class nsIURI;

class BrowsingContext;
class SecurityOrigin;

// Various DOM utilities.
class DOMUtils {
 public:
  // Returns the current JS context.
  static bool GetJsContext(JSContext **context);

  // Returns the window corresponding to given JSContext.
  static nsresult GetDOMWindowInternal(JSContext *context,
                                       nsIDOMWindowInternal **window);

  // Returns the native window for the browser.
  static nsresult GetNativeWindow(JSContext *context, NativeWindowPtr* window);

  // Creates a new nsIURI object. The 'url' parameter can be a relative url or
  // an absolute url. Returns true on success
  // This function does NOT require the DOM. It can be used in worker threads.
  static bool NewResolvedURI(const char16 *base_url,
                             const char16 *url,
                             nsIURI **url_obj);

  // Creates a new nsIURI object and returns true on success
  // This function does NOT require the DOM. It can be used in worker threads.
  static bool NewAbsoluteURI(const char16 *url, nsIURI **url_obj);

  // TODO(cprince): across the codebase, change PageOrigin to PageSecurityOrigin
  // and PageLocation to PageLocationUrl, for consistency.

  // Returns the page's location url (absolute)
  // Returns true on success
  static bool GetPageLocation(std::string16 *location_url);

  // Returns the page's security origin which is based on the location url.
  // Returns true on success
  static bool GetPageOrigin(SecurityOrigin *security_origin);

  // Returns the page's browsing context.
  // Returns true on success.
  static bool GetPageBrowsingContext(
      scoped_refptr<BrowsingContext> *browsing_context);

  // Returns true if there the browser is in 'online' mode
  static bool IsOnline();
};

#endif // GEARS_BASE_FIREFOX_DOM_UTILS_H__
