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

#ifndef GEARS_LOCALSERVER_SAFARI_HTTP_REQUEST_SF_H__
#define GEARS_LOCALSERVER_SAFARI_HTTP_REQUEST_SF_H__

#import <Foundation/NSStream.h>
#include <vector>

#include "gears/base/common/common.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/security_model.h"
#include "gears/localserver/common/http_constants.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/common/progress_event.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class BlobInterface;

//------------------------------------------------------------------------------
// SFHttpRequest
//------------------------------------------------------------------------------
class SFHttpRequest : public HttpRequest, public ProgressEvent::Listener {
 public:
 
  // refcounting
  virtual void Ref();
  virtual void Unref();
 
  // Get or set whether to use or bypass caches, the default is USE_ALL_CACHES
  // May only be set prior to calling Send.
  virtual CachingBehavior GetCachingBehavior() {
    return caching_behavior_;
  }

  virtual bool SetCachingBehavior(CachingBehavior behavior) {
    if (was_sent_) return false;
    caching_behavior_ = behavior;
    return true;
  }

  // Get or set the redirect behavior, the default is FOLLOW_ALL
  // May only be set prior to calling Send.
  virtual RedirectBehavior GetRedirectBehavior() { 
    return redirect_behavior_;
  }

  virtual bool SetRedirectBehavior(RedirectBehavior behavior) {
    if (was_sent_) return false;
    redirect_behavior_ = behavior;
    return true;
  }

  // Set whether browser cookies are sent with the request. The default is
  // SEND_BROWSER_COOKIES. May only be set prior to calling Send.
  virtual bool SetCookieBehavior(CookieBehavior behavior) {
    if (was_sent_) return false;
    cookie_behavior_ = behavior;
    return true;
  }

  // properties
  virtual bool GetReadyState(ReadyState *state);
  virtual bool GetResponseBody(scoped_refptr<BlobInterface>* blob);
  virtual bool GetStatus(int *status);
  virtual bool GetStatusText(std::string16 *status_text);
  virtual bool GetStatusLine(std::string16 *status_line);

  virtual bool WasRedirected();
  virtual bool GetFinalUrl(std::string16 *full_url);
  virtual bool GetInitialUrl(std::string16 *full_url);

  // methods
  virtual bool Open(const char16 *method, const char16 *url, bool async,
                    BrowsingContext *browsing_context);
  virtual bool SetRequestHeader(const char16 *name, const char16 *value);
  virtual bool Send(BlobInterface *data);
  virtual bool GetAllResponseHeaders(std::string16 *headers);
  virtual std::string16 GetResponseCharset();
  virtual bool GetResponseHeader(const char16 *name, std::string16 *header);
  virtual bool Abort();

  // events
  virtual bool SetListener(HttpListener *listener, bool enable_data_available);

 // Methods used to communicate between Obj C delegate and C++ class.
 // You can't make an objc-c selector a friend of a C++ class, so these
 // need to be declared public.
 public:
 
  // Called by the delegate if a redirect is requested by the server
  // returns: true - allow redirect, false - deny redirect.
  bool AllowRedirect(const std::string16 &redirect_url);
  void SetReadyState(ReadyState state);
  // New data has arrived over the connection.
  void OnDataAvailable(int64 length);
  void SetResponseCharset(const std::string16 &charset);

  // Holders for http headers.
  typedef std::pair<std::string16, std::string16> HttpHeader;
  typedef std::vector<HttpHeader> HttpHeaderVector;
  typedef HttpHeaderVector::const_iterator HttpHeaderVectorConstIterator;

 private:
  // ProgressEvent::Listener implementation
  virtual void OnUploadProgress(int64 position, int64 total);

  friend bool HttpRequest::Create(scoped_refptr<HttpRequest>* request);
 
  SFHttpRequest();
  ~SFHttpRequest();

  bool IsUninitialized() { return ready_state_ == UNINITIALIZED; }
  bool IsOpen() { return ready_state_ == OPEN; }
  bool IsSent() { return ready_state_ == SENT; }
  bool IsInteractive() { return ready_state_ == INTERACTIVE; }
  bool IsComplete() { return ready_state_ == COMPLETE; }
  bool IsInteractiveOrComplete() { return IsInteractive() || IsComplete(); }

  bool IsPostOrPut() {
    return method_ == HttpConstants::kHttpPOST || 
           method_ == HttpConstants::kHttpPUT;
  }
  
  void Reset();

  HttpRequest::HttpListener *listener_;
  bool listener_data_available_enabled_;
  ReadyState ready_state_;
  RefCount count_;
  std::string16 method_;
  std::string16 url_;
  SecurityOrigin origin_;
  CachingBehavior caching_behavior_;
  RedirectBehavior redirect_behavior_;
  CookieBehavior cookie_behavior_;
  bool was_sent_;
  bool was_aborted_;
  bool was_redirected_;
  bool async_;
  std::string16 redirect_url_;
  std::string16 charset_;
  
  HttpHeaderVector headers_to_send_;
  
  // Use PIMPL idiom to mask obj-c object, so we can include
  // this header in C++ code.
  struct HttpRequestData;
  HttpRequestData *delegate_holder_;
};

#endif  // GEARS_LOCALSERVER_SAFARI_HTTP_REQUEST_SF_H__
