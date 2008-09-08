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
#ifdef WIN32
#include <windows.h> // must manually #include before nsIEventQueueService.h
#endif
#include <gecko_sdk/include/nspr.h> // for PR_*
#include "gears/base/common/atomic_ops.h"
#include "gears/base/firefox/dom_utils.h"
#include "gears/blob/blob_interface.h"
#include "gears/localserver/common/http_constants.h"
#include "gears/localserver/common/http_cookies.h"
#include "gears/localserver/firefox/async_task_ff.h"
#include "gears/localserver/firefox/ui_thread.h"

const char16 *AsyncTask::kCookieRequiredErrorMessage =
                  STRING16(L"Required cookie is not present");

const char16 *kIsOfflineErrorMessage =
                  STRING16(L"The browser is offline");

//------------------------------------------------------------------------------
// AsyncTask
//------------------------------------------------------------------------------
AsyncTask::AsyncTask(BrowsingContext *browsing_context) :
    is_aborted_(false),
    is_initialized_(false),
    browsing_context_(browsing_context),
    delete_when_done_(false),
    listener_(NULL),
    thread_running_(false),
    thread_id_(0),
    listener_thread_id_(0),
    params_(NULL) {
  Ref();
}

//------------------------------------------------------------------------------
// ~AsyncTask
//------------------------------------------------------------------------------
AsyncTask::~AsyncTask() {
  assert(!thread_running_);
  assert(!http_request_.get());
  assert(!params_);
  assert(GetRef() == 0 || 
         (GetRef() == 1 && !delete_when_done_));  
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
bool AsyncTask::Init() {
  assert(!delete_when_done_);

  if (is_initialized_) {
    assert(!is_initialized_);
    return false;
  }

  if (!static_cast<PRMonitor*>(lock_)) {
    return false;
  }

  ThreadMessageQueue::GetInstance()->InitThreadMessageQueue();
  listener_thread_id_ =
      ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  is_aborted_ = false;
  is_initialized_ = true;
  return true;
}

//------------------------------------------------------------------------------
// SetListener
//------------------------------------------------------------------------------
void AsyncTask::SetListener(Listener *listener) {
  assert(!delete_when_done_);
  assert(IsListenerThread() || listener == NULL);
  listener_ = listener;
}

//------------------------------------------------------------------------------
// Start
//------------------------------------------------------------------------------
bool AsyncTask::Start() {
  assert(!delete_when_done_);
  assert(IsListenerThread());

  if (!is_initialized_ || thread_running_) {
    assert(!(!is_initialized_ || thread_running_));
    return false;
  }

  // Start our worker thread
  CritSecLock locker(lock_);
  is_aborted_ = false;
  thread_running_ = PR_CreateThread(PR_USER_THREAD, ThreadEntry, // type, func
                            this, PR_PRIORITY_NORMAL,   // arg, priority
                            PR_LOCAL_THREAD,          // scheduled by whom?
                            PR_UNJOINABLE_THREAD, // joinable?
                            0) != NULL; // stack bytes
  if (!thread_running_) {
    return false;
  }

  Ref();  // reference is removed upon worker thread exit

  return true;
}

//------------------------------------------------------------------------------
// Abort
//------------------------------------------------------------------------------
void AsyncTask::Abort() {
  LOG(("AsyncTask::Abort\n"));
  assert(!delete_when_done_);

  CritSecLock locker(lock_);
  is_aborted_ = true;

  if (params_) {
    // An http request is in progress that we must terminate.
    // We can only terminate HTTP requests from the UI thread.
    CallAsync(GetUiThread(), kAbortHttpGetMessageCode, NULL);
  }
}

void AsyncTask::OnAbortHttpGet() {
  assert(IsUiThread());
  LOG(("AsyncTask::OnAbortHttpGet - ui thread\n"));

  if (http_request_) {
    http_request_->SetListener(NULL, false);
    http_request_->Abort();
    http_request_.reset(NULL);
  }
  CritSecLock locker(lock_);
  PR_Notify(lock_);  // notify our waiting worker thread
}


//------------------------------------------------------------------------------
// DeleteWhenDone
//------------------------------------------------------------------------------
void AsyncTask::DeleteWhenDone() {
  assert(!delete_when_done_);

  LOG(("AsyncTask::DeleteWhenDone\n"));
  CritSecLock locker(lock_);
  SetListener(NULL);
  delete_when_done_ = true;

  // We have to call unlock prior to calling Unref 
  // otherwise the locker would try to access deleted memory, &lock_,
  // after it's been freed.
  locker.Unlock();
  Unref();  // remove the reference added by the constructor
}


//------------------------------------------------------------------------------
// ThreadEntry - Our worker thread's entry procedure
//------------------------------------------------------------------------------
void AsyncTask::ThreadEntry(void *task) {
  ThreadMessageQueue::GetInstance()->InitThreadMessageQueue();
  AsyncTask *self = reinterpret_cast<AsyncTask*>(task);
  // Don't run until we're sure all state is initialized.
  {
    CritSecLock locker(self->lock_);
    self->thread_id_ = ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  }
  self->Run();
  self->thread_running_ = false;
  self->Unref();  // remove the reference added by the Start
}

//------------------------------------------------------------------------------
// NotifyListener
//------------------------------------------------------------------------------
void AsyncTask::NotifyListener(int code, int param) {
  assert(IsTaskThread());
  if (listener_) {
    CallAsync(listener_thread_id_, code, reinterpret_cast<void*>(param));
  }
}

//------------------------------------------------------------------------------
// OnListenerEvent
//------------------------------------------------------------------------------
void AsyncTask::OnListenerEvent(int msg_code, int msg_param) {
  assert(IsListenerThread());
  if (listener_) {
    listener_->HandleEvent(msg_code, msg_param, this);
  }
}


//------------------------------------------------------------------------------
// struct HttpRequestParameters
//------------------------------------------------------------------------------
struct AsyncTask::HttpRequestParameters {
  const char16 *method;
  const char16 *full_url;
  bool is_capturing;
  const char16 *reason_header_value;
  const char16 *if_mod_since_date;
  const char16 *required_cookie;
  bool disable_browser_cookies;
  BlobInterface *post_body;
  WebCacheDB::PayloadInfo *payload;
  scoped_refptr<BlobInterface>* payload_data;
  bool *was_redirected;
  std::string16 *full_redirect_url;
  std::string16 *error_message;
};

//------------------------------------------------------------------------------
// HttpGet - Performs an HTTP get which appears synchronous to our caller on
// the worker thread. Under the covers a message is posted to the UI thread and
// the current thread waits for our lock to be signaled. The UI thread
// initiates an async HTTP request and signals our lock when the request
// completes.
//------------------------------------------------------------------------------
bool AsyncTask::HttpGet(const char16 *full_url,
                        bool is_capturing,
                        const char16 *reason_header_value,
                        const char16 *if_mod_since_date,
                        const char16 *required_cookie,
                        WebCacheDB::PayloadInfo *payload,
                        scoped_refptr<BlobInterface> *payload_data,
                        bool *was_redirected,
                        std::string16 *full_redirect_url,
                        std::string16 *error_message) {
  return MakeHttpRequest(HttpConstants::kHttpGET,
                         full_url,
                         is_capturing,
                         reason_header_value,
                         if_mod_since_date,
                         required_cookie,
                         false,
                         NULL,
                         payload,
                         payload_data,
                         was_redirected,
                         full_redirect_url,
                         error_message);
}

//------------------------------------------------------------------------------
// HttpPost
//------------------------------------------------------------------------------
bool AsyncTask::HttpPost(const char16 *full_url,
                         bool is_capturing,
                         const char16 *reason_header_value,
                         const char16 *if_mod_since_date,
                         const char16 *required_cookie,
                         bool disable_browser_cookies,
                         BlobInterface *post_body,
                         WebCacheDB::PayloadInfo *payload,
                         scoped_refptr<BlobInterface> *payload_data,
                         bool *was_redirected,
                         std::string16 *full_redirect_url,
                         std::string16 *error_message) {
  return MakeHttpRequest(HttpConstants::kHttpPOST,
                         full_url,
                         is_capturing,
                         reason_header_value,
                         if_mod_since_date,
                         required_cookie,
                         disable_browser_cookies,
                         post_body,
                         payload,
                         payload_data,
                         was_redirected,
                         full_redirect_url,
                         error_message);
}

//------------------------------------------------------------------------------
// MakeHttpRequest
//------------------------------------------------------------------------------
bool AsyncTask::MakeHttpRequest(const char16 *method,
                                const char16 *full_url,
                                bool is_capturing,
                                const char16 *reason_header_value,
                                const char16 *if_mod_since_date,
                                const char16 *required_cookie,
                                bool disable_browser_cookies,
                                BlobInterface *post_body,
                                WebCacheDB::PayloadInfo *payload,
                                scoped_refptr<BlobInterface> *payload_data,
                                bool *was_redirected,
                                std::string16 *full_redirect_url,
                                std::string16 *error_message) {
  // This method should only be called our worker thread.
  assert(IsTaskThread());

  assert(payload);
  if (was_redirected) {
    *was_redirected = false;
  }
  if (full_redirect_url) {
    full_redirect_url->clear();
  }
  if (error_message) {
    error_message->clear();
  }

  if (!DOMUtils::IsOnline()) {
    if (error_message) {
      *error_message = kIsOfflineErrorMessage;
    }
    return false;
  }

  CritSecLock locker(lock_);
  if (is_aborted_) {
    return false;
  }
  payload->data.reset(NULL);

  // We actually initiate / terminate HTTP requests from the UI thread as
  // it's not safe to do from a worker thread.
  // Setup parameters for OnStartHttpGet executing on the UI thread to look at
  HttpRequestParameters params;
  params.method = method;
  params.full_url = full_url;
  params.is_capturing = is_capturing;
  params.reason_header_value = reason_header_value;
  params.if_mod_since_date = if_mod_since_date;
  params.required_cookie = required_cookie;
  params.disable_browser_cookies = disable_browser_cookies;
  params.post_body = post_body;
  params.payload = payload;
  params.payload_data = payload_data;
  params.was_redirected = was_redirected;
  params.full_redirect_url = full_redirect_url;
  params.error_message = error_message;
  params_ = &params;

  // Send a message to the UI thread to initiate the get
  CallAsync(GetUiThread(), kStartHttpGetMessageCode, NULL);

  // Wait for completion
  PR_Wait(lock_, PR_INTERVAL_NO_TIMEOUT);

  params_ = NULL;

  return !is_aborted_ && payload_data->get();
}



//------------------------------------------------------------------------------
// OnStartHttpGet
//------------------------------------------------------------------------------
bool AsyncTask::OnStartHttpGet() {
  if (is_aborted_) {
    // This can happen if Abort() is called after the worker thread has queued
    // a call to start a request but prior to having started that request.
    return false;
  }

  if (params_->required_cookie && params_->required_cookie[0]) {
    std::string16 required_cookie_str(params_->required_cookie);
    std::string16 name, value;
    ParseCookieNameAndValue(required_cookie_str, &name, &value);
    if (value != kNegatedRequiredCookieValue) {
      CookieMap cookie_map;
      cookie_map.LoadMapForUrl(params_->full_url, browsing_context_.get());
      if (!cookie_map.HasLocalServerRequiredCookie(required_cookie_str)) {
        if (params_->error_message) {
          *(params_->error_message) = kCookieRequiredErrorMessage;
        }
        return false;
      }
    }
  }

  scoped_refptr<HttpRequest> http_request;
  if (!HttpRequest::Create(&http_request)) {
    return false;
  }

  http_request->SetCachingBehavior(HttpRequest::BYPASS_ALL_CACHES);

  if (!http_request->Open(params_->method, params_->full_url, true,
                          browsing_context_.get())) {
    return false;
  }

  if (params_->is_capturing) {
    http_request->SetRedirectBehavior(HttpRequest::FOLLOW_NONE);
    if (!http_request->SetRequestHeader(HttpConstants::kXGoogleGearsHeader,
                                        STRING16(L"1"))) {
      return false;
    }
  }

  if (params_->reason_header_value && params_->reason_header_value[0]) {
    if (!http_request->SetRequestHeader(HttpConstants::kXGearsReasonHeader,
                                        params_->reason_header_value)) {
      return false;
    }
  }

  if (!http_request->SetRequestHeader(HttpConstants::kCacheControlHeader,
                                      HttpConstants::kNoCache)) {
    return false;
  }

  if (!http_request->SetRequestHeader(HttpConstants::kPragmaHeader,
                                      HttpConstants::kNoCache)) {
    return false;
  }

  if (params_->if_mod_since_date && params_->if_mod_since_date[0]) {
    if (!http_request->SetRequestHeader(HttpConstants::kIfModifiedSinceHeader,
                                        params_->if_mod_since_date)) {
      return false;
    }
  }

  if (params_->disable_browser_cookies) {
    if (!http_request->SetCookieBehavior(
        HttpRequest::DO_NOT_SEND_BROWSER_COOKIES)) {
      return false;
    }
  }

  http_request->SetListener(this, false);

  http_request_ = http_request;

  // Rely on logic inside HttpRequest to check for valid combinations of
  // method and presence of body.
  bool result = http_request->Send(params_->post_body);
  if (!result) {
    http_request->SetListener(NULL, false);
    http_request_.reset(NULL);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// HttpRequest::HttpListener::ReadyStateChanged
//------------------------------------------------------------------------------
void AsyncTask::ReadyStateChanged(HttpRequest *http_request) {
  assert(params_);
  assert(http_request_ == http_request);
  HttpRequest::ReadyState state;
  if (http_request->GetReadyState(&state)) {
    if (state == HttpRequest::COMPLETE) {
      if (!is_aborted_) {
        int status;
        if (http_request->GetStatus(&status)) {
          WebCacheDB::PayloadInfo *payload = params_->payload;
          payload->status_code = status;
          if (http_request->GetStatusLine(&payload->status_line)) {
            if (http_request->GetAllResponseHeaders(&payload->headers)) {
              http_request->GetResponseBody(params_->payload_data);
            }
          }
        }
      }
      http_request->SetListener(NULL, false);
      if (http_request->WasRedirected()) {
        if (params_->was_redirected) {
          *(params_->was_redirected) = true;
        }
        if (params_->full_redirect_url) {
          http_request->GetFinalUrl(params_->full_redirect_url);
        }
      }
      http_request_.reset(NULL);
      CritSecLock locker(lock_);
      PR_Notify(lock_);
    }
  } else {
    http_request->SetListener(NULL, false);
    http_request->Abort();
    http_request_.reset(NULL);
    CritSecLock locker(lock_);
    PR_Notify(lock_);
  }
}

//------------------------------------------------------------------------------
// OnAsyncCall - Called when a message sent via CallAsync is delivered to us
// on the target thread of control.
//------------------------------------------------------------------------------
void AsyncTask::OnAsyncCall(int msg_code, void *msg_param) {
  switch (msg_code) {
    case kStartHttpGetMessageCode:
      assert(IsUiThread());
      if (!OnStartHttpGet()) {
        CritSecLock locker(lock_);
        PR_Notify(lock_);
      }
      break;
    case kAbortHttpGetMessageCode:
      assert(IsUiThread());
      OnAbortHttpGet();
      break;
    default:
      assert(IsListenerThread());
      OnListenerEvent(msg_code, reinterpret_cast<int>(msg_param));
      break;
  }
}

//------------------------------------------------------------------------------
// AsyncCallEvent - Custom event class used to post messages across threads
//------------------------------------------------------------------------------
class AsyncTask::AsyncCallEvent : public AsyncFunctor {
public:
  AsyncCallEvent(AsyncTask *task, int code, void *param)
      : task(task), msg_code(code), msg_param(param) {
    task->Ref();
  }

  ~AsyncCallEvent() {
    task->Unref();
  }

  void Run() {
    task->OnAsyncCall(msg_code, msg_param);
  }

private:
  // Note: we can't make this a scoped_refptr without making AsyncTask's
  // Ref()/Unref() members public.
  AsyncTask *task;
  int msg_code;
  void *msg_param;
};

//------------------------------------------------------------------------------
// CallAsync - Posts a message to another thead's event queue. The message will
// be delivered to this AsyncTask instance on that thread via OnAsyncCall.
//------------------------------------------------------------------------------
nsresult AsyncTask::CallAsync(ThreadId thread_id,
                              int msg_code, void *msg_param) {

  AsyncRouter::GetInstance()->CallAsync(
      thread_id,
      new AsyncCallEvent(this, msg_code, msg_param));
  return NS_OK;
}
