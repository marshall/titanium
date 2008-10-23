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

#ifndef GEARS_BASE_NPAPI_BROWSER_UTILS_H__
#define GEARS_BASE_NPAPI_BROWSER_UTILS_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"

class BrowsingContext;
class JsCallContext;

class BrowserUtils {
 public:
  // Called when JavaScript calls into a Gears class to access a property or
  // method.
  static void EnterScope(JsCallContext *context);

  // Called when we are done handling a JavaScript callback and return
  // execution to the script.
  static void ExitScope();

  // Sets a JavaScript exception to be thrown upon return from plugin entry
  // point.  It is an error to call this when not inside a plugin entry point.
  static void SetJsException(const std::string16 &message);

  // Returns the current JsCallContext.  Returns NULL when not inside a plugin
  // entry point.
  static JsCallContext *GetCurrentJsCallContext();

  // Returns the page's location url (absolute)
  // Returns true on success
  static bool GetPageLocationUrl(JsContextPtr context,
                                 std::string16 *location_url);

  // Returns the page's security origin which is based on the location url.
  // Returns true on success.
  static bool GetPageSecurityOrigin(JsContextPtr context,
                                    SecurityOrigin *security_origin);

  // Returns the page's browsing context.
  // Returns true on success.
  static bool GetPageBrowsingContext(
      JsContextPtr context, scoped_refptr<BrowsingContext> *browsing_context);

  // Get the current browser's user agent string.
  static bool GetUserAgentString(std::string16 *user_agent);

  // Returns true if the browser is not in "offline" mode and there is access
  // to the network.
  static bool IsOnline();
};

#endif  // GEARS_BASE_NPAPI_BROWSER_UTILS_H__
