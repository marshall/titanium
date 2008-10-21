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

#ifndef GEARS_LOCALSERVER_FIREFOX_HTTP_REQUEST_FF_H__
#define GEARS_LOCALSERVER_FIREFOX_HTTP_REQUEST_FF_H__

#include <string>
#include <vector>
#include "gecko_sdk/include/nsIInterfaceRequestor.h"
#include "gecko_sdk/include/nsILoadGroup.h"
#include "gecko_sdk/include/nsIStreamListener.h"
#include "gecko_internal/nsIChannelEventSink.h"
#include "gecko_internal/nsIDocShellTreeItem.h"
#include "genfiles/interfaces.h"
#include "gears/base/common/byte_store.h"
#include "gears/base/common/common.h"
#include "gears/base/common/security_model.h"
#include "gears/localserver/common/http_constants.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/common/progress_event.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class BlobInterface;
class ProgressInputStream;

class nsIChannel;
class nsIHttpChannel;
class nsIRequest;

// The HttpRequestObserver class is an implementation detail of FFHttpRequest,
// used to remove browser cookies from outgoing requests.
class HttpRequestObserver;

#if BROWSER_FF3
class GearsLoadGroup : public nsILoadGroup,
                       public nsIDocShellTreeItem,
                       public nsIInterfaceRequestor {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUEST
  NS_DECL_NSILOADGROUP
  NS_DECL_NSIDOCSHELLTREEITEM
  NS_DECL_NSIINTERFACEREQUESTOR
  NS_DECL_NSIDOCSHELLTREENODE

  // refcounting
  virtual void Ref();
  virtual void Unref();
};
#endif

//------------------------------------------------------------------------------
// FFHttpRequest
//------------------------------------------------------------------------------
class FFHttpRequest : public HttpRequest,
                      public nsIStreamListener,
                      public nsIChannelEventSink,
                      public nsIInterfaceRequestor,
                      public SpecialHttpRequestInterface,
                      public ProgressEvent::Listener {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSICHANNELEVENTSINK
  NS_DECL_NSIINTERFACEREQUESTOR
  NS_DECL_SPECIALHTTPREQUESTINTERFACE  // see localserver.idl.m4

  // Our C++ HttpRequest interface

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
  virtual bool Send(BlobInterface *blob);
  virtual bool GetAllResponseHeaders(std::string16 *headers);
  virtual std::string16 GetResponseCharset();
  virtual bool GetResponseHeader(const char16 *name, std::string16 *header);
  virtual bool Abort();

  // events
  virtual bool SetListener(HttpListener *listener, bool enable_data_available);

 private:
  // ProgressEvent::Listener implementation
  virtual void OnUploadProgress(int64 position, int64 total);

  friend bool HttpRequest::Create(scoped_refptr<HttpRequest>* request);

  FFHttpRequest();
  ~FFHttpRequest();

  already_AddRefed<nsIHttpChannel> GetCurrentHttpChannel();

  void SetReadyState(ReadyState state);
  bool IsUninitialized() { return ready_state_ == HttpRequest::UNINITIALIZED; }
  bool IsOpen() { return ready_state_ == HttpRequest::OPEN; }
  bool IsSent() { return ready_state_ == HttpRequest::SENT; }
  bool IsInteractive() { return ready_state_ == HttpRequest::INTERACTIVE; }
  bool IsComplete() { return ready_state_ == HttpRequest::COMPLETE; }
  bool IsInteractiveOrComplete() { return IsInteractive() || IsComplete(); }

  bool NewByteInputStream(nsIInputStream **stream,
                          const char *data,
                          int data_size);
  bool IsPostOrPut() {
    return method_ == HttpConstants::kHttpPOST ||
           method_ == HttpConstants::kHttpPUT;
  }

  bool IsFileGet() {
#ifdef OFFICIAL_BUILD
    return false;
#else
    return method_ == HttpConstants::kHttpGET &&
           origin_.scheme() == HttpConstants::kFileScheme;
#endif
  }

  static NS_METHOD StreamReaderFunc(nsIInputStream *in,
                                    void *closure,
                                    const char *fromRawSegment,
                                    PRUint32 toOffset,
                                    PRUint32 count,
                                    PRUint32* writeCount);
  ReadyState ready_state_;
  bool async_;
  std::string16 method_;
  nsCOMPtr<ProgressInputStream> post_data_stream_;
  bool post_data_stream_attached_;
  scoped_refptr<ByteStore> response_body_;
  std::string16 url_;
  SecurityOrigin origin_;
  CachingBehavior caching_behavior_;
  RedirectBehavior redirect_behavior_;
  CookieBehavior cookie_behavior_;
  nsCOMPtr<HttpRequestObserver> observer_;
  bool was_sent_;
  bool was_aborted_;
  bool was_redirected_;
  std::string16 redirect_url_;
  nsCOMPtr<nsIChannel> channel_;
  HttpRequest::HttpListener *listener_;
  bool listener_data_available_enabled_;
#if BROWSER_FF3
  scoped_refptr<GearsLoadGroup> load_group_;
#endif
};

#endif  // GEARS_LOCALSERVER_FIREFOX_HTTP_REQUEST_FF_H__
