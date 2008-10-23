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

#ifndef GEARS_LOCALSERVER_COMMON_HTTP_REQUEST_H__
#define GEARS_LOCALSERVER_COMMON_HTTP_REQUEST_H__

#include <assert.h>
#include <vector>
#include "gears/base/common/basictypes.h"
#include "gears/base/common/browsing_context.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/http_constants.h"

class BlobInterface;

//------------------------------------------------------------------------------
// A cross-platform interface for sending HTTP requests.  Implementations
// use the underlying capabilities of different browsers.
//------------------------------------------------------------------------------
class HttpRequest {
 public:
  // Creates a platform specific instance
  static bool Create(scoped_refptr<HttpRequest>* request);
  
  // Creates an instance that is guaranteed to work in background threads
  // TODO(michaeln): ween ourselves off of Create vs CreateSafeRequest, Create
  // should always return a "safe" instance.
  static bool CreateSafeRequest(scoped_refptr<HttpRequest>* request);

  // Returns true if the given scheme is supported by HttpRequest.
  static bool IsSchemeSupported(const char16 *scheme) {
    if (StringCompareIgnoreCase(scheme, HttpConstants::kHttpScheme) == 0 ||
        StringCompareIgnoreCase(scheme, HttpConstants::kHttpsScheme) == 0) {
      return true;
    } else {
      return false;
    }
  }

  // refcounting
  virtual void Ref() = 0;
  virtual void Unref() = 0;

  // Get or set whether to use or bypass caches, the default is USE_ALL_CACHES.
  // May only be set prior to calling Send.
  enum CachingBehavior {
    USE_ALL_CACHES,
    BYPASS_ALL_CACHES  // bypass the browser cache and our local server
  };
  virtual CachingBehavior GetCachingBehavior() = 0;
  virtual bool SetCachingBehavior(CachingBehavior behavior) = 0;

  // Get or set the redirect behavior, the default is FOLLOW_ALL.
  // May only be set prior to calling Send.
  enum RedirectBehavior {
    FOLLOW_ALL,
    FOLLOW_WITHIN_ORIGIN,
    FOLLOW_NONE
  };
  virtual RedirectBehavior GetRedirectBehavior() = 0;
  virtual bool SetRedirectBehavior(RedirectBehavior behavior) = 0;

  // Set whether browser cookies are sent with the request. The default is
  // SEND_BROWSER_COOKIES. May only be set prior to calling Send.
  enum CookieBehavior {
    SEND_BROWSER_COOKIES,
    DO_NOT_SEND_BROWSER_COOKIES
  };
  virtual bool SetCookieBehavior(CookieBehavior behavior) = 0;

  enum ReadyState {
    UNINITIALIZED = 0,
    OPEN = 1,
    SENT = 2,
    INTERACTIVE = 3,
    COMPLETE = 4
  };
  virtual bool GetReadyState(ReadyState *state) = 0;
  virtual bool GetResponseBody(scoped_refptr<BlobInterface>* blob) = 0;
  virtual bool GetStatus(int *status) = 0;
  virtual bool GetStatusText(std::string16 *status_text) = 0;
  virtual bool GetStatusLine(std::string16 *status_line) = 0;

  // Whether or not this request has followed a redirect
  virtual bool WasRedirected() = 0;

  // Sets full_url to final location of the request, including any redirects.
  // Returns false on failure (e.g. final redirect url not yet known).
  virtual bool GetFinalUrl(std::string16 *full_url) = 0;

  // Similar to GetFinalUrl, but retrieves the initial URL requested.
  // Always succeeds.
  virtual bool GetInitialUrl(std::string16 *full_url) = 0;

  // methods
  virtual bool Open(const char16 *method, const char16* url, bool async,
                    BrowsingContext *browsing_context) = 0;
  virtual bool SetRequestHeader(const char16 *name, const char16 *value) = 0;
  virtual bool Send(BlobInterface* blob) = 0;
  virtual bool GetAllResponseHeaders(std::string16 *headers) = 0;
  virtual std::string16 GetResponseCharset() = 0;
  virtual bool GetResponseHeader(const char16* name, std::string16 *header) = 0;
  virtual bool Abort() = 0;

  // events and listeners
  class HttpListener {
   public:
    virtual void DataAvailable(HttpRequest *source, int64 position) {}
    virtual void ReadyStateChanged(HttpRequest *source) {}
    virtual void UploadProgress(HttpRequest *source,
                                int64 position, int64 total) {}
  };

  // Should be called prior to Send with the exception of setting
  // the listener to NULL which can be done at anytime.
  virtual bool SetListener(HttpListener *listener,
                           bool enable_data_availbable) = 0;

 protected:
  HttpRequest() {}
  virtual ~HttpRequest() {}

  bool ShouldBypassLocalServer() {
    return GetCachingBehavior() == BYPASS_ALL_CACHES;
  }

  bool ShouldBypassBrowserCache() {
    return GetCachingBehavior() == BYPASS_ALL_CACHES;
  }
};

#endif  // GEARS_LOCALSERVER_COMMON_HTTP_REQUEST_H__
