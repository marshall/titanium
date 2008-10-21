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

#ifndef GEARS_LOCALSERVER_COMMON_SAFE_HTTP_REQUEST_H__
#define GEARS_LOCALSERVER_COMMON_SAFE_HTTP_REQUEST_H__


#include "gears/base/common/async_router.h"
#include "gears/base/common/common.h"
#include "gears/base/common/http_utils.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/blob/blob_interface.h"
#include "gears/localserver/common/http_request.h"
#include "third_party/scoped_ptr/scoped_ptr.h"


// "Safe" means that it can be used from a background thread. "Safe" does not
// mean that an single instance can be used from multiple threads of control.
//
// Specifically, compare this to the Firefox port's native HttpRequest,
// which can only be used from the main (safe) thread.
//
// For browsers that have this constraint, a SafeHttpRequest can co-ordinate
// with the safe thread to execute a native HttpRequest on a background thread's
// behalf. Clients should use HttpRequest::CreateSafeRequest factory method
// to create an instance of an object that implements the HttpRequest interface
// and is safe to use from any thread. However clients should not assume that
// the instance returned is an instance of this class. The "native" HttpRequest
// class for some ports work equally well on background and foreground threads.
//
// This helper class is effectively an implementation detail of the native
// HttpRequest implementation for a given port.

class SafeHttpRequest
    : public HttpRequest,
      public HttpRequest::HttpListener {
 public:
  virtual void Ref() {
    ref_count_.Ref();
  }

  virtual void Unref() {
    if (ref_count_.Unref())
      delete this;
  }

  virtual bool Open(const char16 *method, const char16* url, bool async,
                    BrowsingContext *browsing_context);
  virtual bool GetResponseBody(scoped_refptr<BlobInterface>* blob);
  virtual bool GetAllResponseHeaders(std::string16 *headers);
  virtual std::string16 GetResponseCharset();
  virtual bool GetResponseHeader(const char16* name, std::string16 *header);

  virtual bool GetStatus(int *status);
  virtual bool GetStatusText(std::string16 *status_text);
  virtual bool GetStatusLine(std::string16 *status_line);

  virtual bool WasRedirected();
  virtual bool GetFinalUrl(std::string16 *full_url);
  virtual bool GetInitialUrl(std::string16 *full_url);

  virtual bool SetRequestHeader(const char16 *name, const char16 *value);

  virtual bool Send(BlobInterface* blob);

  virtual bool Abort();

  virtual bool GetReadyState(ReadyState *state);

  virtual CachingBehavior GetCachingBehavior();
  virtual bool SetCachingBehavior(CachingBehavior behavior);
  virtual RedirectBehavior GetRedirectBehavior();
  virtual bool SetRedirectBehavior(RedirectBehavior behavior);
  virtual bool SetCookieBehavior(CookieBehavior behavior);

  virtual bool SetListener(HttpRequest::HttpListener *listener,
                           bool enable_data_available);
 private:
  struct ResponseInfo {
    int status;
    bool was_redirected;
    std::string16 status_text;
    std::string16 status_line;
    std::string16 final_url;
    std::string16 headers;
    std::string16 charset;
    scoped_ptr<HTTPHeaders> parsed_headers;
    // Valid when the ReadyState is either INTERACTIVE or COMPLETE.
    scoped_refptr<BlobInterface> body;
    ResponseInfo() : status(0), was_redirected(false), body(new EmptyBlob()) {}
  };

  struct ProgressInfo {
    ProgressInfo();
    int64 position;
    int64 total;
    int64 reported;
  };

  struct RequestInfo {
    ReadyState ready_state;
    ReadyState upcoming_ready_state;
    CachingBehavior caching_behavior;
    RedirectBehavior redirect_behavior;
    CookieBehavior cookie_behavior;
    std::string16 method;
    std::string16 full_url;
    std::vector< std::pair<std::string16, std::string16> > headers;
    scoped_refptr<BrowsingContext> browsing_context;

    scoped_refptr<BlobInterface> post_data_blob;
    ProgressInfo download_progress;
    ProgressInfo upload_progress;
    ResponseInfo response;

    RequestInfo()
        : ready_state(UNINITIALIZED),
          upcoming_ready_state(UNINITIALIZED),
          caching_behavior(USE_ALL_CACHES),
          redirect_behavior(FOLLOW_WITHIN_ORIGIN),
          cookie_behavior(SEND_BROWSER_COOKIES) {}
  };

  friend bool TestHttpRequest();
  friend bool HttpRequest::CreateSafeRequest(
                               scoped_refptr<HttpRequest>* request);
  
  SafeHttpRequest(ThreadId safe_thread_id);
  virtual ~SafeHttpRequest();

  bool IsSafeThread() {
     return safe_thread_id_ ==
        ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
 }

  bool IsApartmentThread() {
    return apartment_thread_id_ ==
        ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  }

  // Apartment / Safe thread ping-pong methods.
  enum AsyncCallType {
    kSend,   // invoked from apartment to execute on safe thread
    kAbort,  // invoked from apartment to execute on safe thread
    kReadyStateChanged,  // invoked from safe to execute on apartment thread
    kDataAvailable  // invoked from safe to execute on apartment thread
  };
  bool CallAbortOnSafeThread();
  bool CallSendOnSafeThread();
  bool CallReadyStateChangedOnApartmentThread();
  bool CallDataAvailableOnApartmentThread();
  bool CallUploadProgressOnApartmentThread();
  void OnAbortCall();
  void OnSendCall();
  void OnReadyStateChangedCall();
  void OnDataAvailableCall();
  void OnUploadProgressCall();

  // Async ping-pong support.
  typedef void (SafeHttpRequest::*Method)();
  class AsyncMethodCall;
  void CallAsync(ThreadId thread_id, Method method);

  // HttpListener implementation.
  void DataAvailable(HttpRequest *source, int64 position);
  void ReadyStateChanged(HttpRequest *source);
  void UploadProgress(HttpRequest *source, int64 position, int64 total);

  // Other private methods
  ReadyState GetState();
  bool IsValidResponse();
  void CreateNativeRequest();
  void RemoveNativeRequest();

  // Terminology, the apartment thread is the thread of control that the
  // request instance was created on.
  //
  // The following members represent the state of the current request.
  // Methods of this class running in both the apartment and safe threads
  // access this shared state. The apartment thread manages the life-cycle
  // of the RequestInfo ptr.  
  Mutex request_info_lock_;
  RequestInfo request_info_;
  bool was_aborted_;
  bool was_sent_;
  bool was_response_accessed_;
  bool was_data_available_called_;

  RefCount ref_count_;
  HttpRequest::HttpListener *listener_;
  bool listener_data_available_enabled_;

  // Used only on the safe thread 
  scoped_refptr<HttpRequest> native_http_request_;

  ThreadId safe_thread_id_;
  ThreadId apartment_thread_id_;

  DISALLOW_EVIL_CONSTRUCTORS(SafeHttpRequest);
};


#endif // GEARS_LOCALSERVER_COMMON_SAFE_HTTP_REQUEST_H__
