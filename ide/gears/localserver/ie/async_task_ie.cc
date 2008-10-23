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

#include "gears/localserver/ie/async_task_ie.h"

#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"
#endif
#include "gears/base/ie/activex_utils.h"
#include "gears/blob/blob_interface.h"
#include "gears/localserver/common/critical_section.h"
#include "gears/localserver/common/http_constants.h"
#include "gears/localserver/common/http_cookies.h"

const char16 *AsyncTask::kCookieRequiredErrorMessage =
                  STRING16(L"Required cookie is not present");

const char16 *kIsOfflineErrorMessage =
                  STRING16(L"The browser is offline");

//------------------------------------------------------------------------------
// AsyncTask
//------------------------------------------------------------------------------
AsyncTask::AsyncTask(BrowsingContext *browsing_context)
    : is_initialized_(false),
      is_aborted_(false),
      browsing_context_(browsing_context),
      delete_when_done_(false),
      listener_window_(NULL),
      listener_message_base_(WM_USER),
      thread_(NULL) {
}

//------------------------------------------------------------------------------
// ~AsyncTask
//------------------------------------------------------------------------------
AsyncTask::~AsyncTask() {
  assert(!thread_);
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
bool AsyncTask::Init() {
  if (is_initialized_) {
    assert(!is_initialized_);
    return false;
  }

  is_aborted_ = false;

  // create events, both are auto-reset and initially unsignalled
  if (!abort_event_.Create(NULL, FALSE, FALSE, NULL)) {
    return false;
  }
  if (!ready_state_changed_event_.Create(NULL, FALSE, FALSE, NULL)) {
    return false;
  }
  is_initialized_ = true;
  return true;
}

//------------------------------------------------------------------------------
// SetListenerWindow
//------------------------------------------------------------------------------
void AsyncTask::SetListenerWindow(HWND listener_window,
                                  int listener_message_base) {
  CritSecLock locker(lock_);
  listener_window_ = listener_window;
  listener_message_base_ = listener_message_base;
}

//------------------------------------------------------------------------------
// Start
//------------------------------------------------------------------------------
bool AsyncTask::Start() {
  if (!is_initialized_ || thread_) {
    return false;
  }

  CritSecLock locker(lock_);
  is_aborted_ = false;
  abort_event_.Reset();
  ready_state_changed_event_.Reset();
  unsigned int thread_id;
  thread_ = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &ThreadMain,
                                                    this, 0, &thread_id));
  return (thread_ != NULL);
}

//------------------------------------------------------------------------------
// Abort
//------------------------------------------------------------------------------
void AsyncTask::Abort() {
  CritSecLock locker(lock_);
  if (thread_ && !is_aborted_) {
    LOG16((L"AsyncTask::Abort\n"));
    is_aborted_ = true;
    abort_event_.Set();
  }
}

//------------------------------------------------------------------------------
// DeleteWhenDone
//------------------------------------------------------------------------------
void AsyncTask::DeleteWhenDone() {
  CritSecLock locker(lock_);
  assert(!delete_when_done_);
  if (!delete_when_done_) {
    LOG16((L"AsyncTask::DeleteWhenDone\n"));
    SetListenerWindow(NULL, 0);
    if (!thread_) {
      // In this particular code path, we have to call unlock prior to delete
      // otherwise the locker would try to access deleted memory, &lock_,
      // after it's been freed.
      locker.Unlock();
      delete this;
    } else {
      delete_when_done_ = true;
    }
  }
}


//------------------------------------------------------------------------------
// ThreadMain
//------------------------------------------------------------------------------
unsigned int __stdcall AsyncTask::ThreadMain(void *task) {
  // If initialization fails, don't return immediately. Let the thread continue
  // to run to let signaling around startup/shutdown performed by clients work
  // properly. Down the road, COM objects will fail to be created or function
  // and the thread will unwind normally.
  if (FAILED(CoInitializeEx(NULL, GEARS_COINIT_THREAD_MODEL))) {
    LOG(("AsyncTask::ThreadMain - failed to initialize new thread.\n"));
  }

  AsyncTask *self = reinterpret_cast<AsyncTask*>(task);

  self->Run();

  {
    CritSecLock locker(self->lock_);
    CloseHandle(self->thread_);
    self->thread_ = NULL;
    if (self->delete_when_done_) {
      // In this particular code path, we have to call unlock prior to delete
      // otherwise the locker would try to access deleted memory, &lock_,
      // after it's been freed.
      locker.Unlock();
      delete self;
    }
  }

  CoUninitialize();
  return 0;
}

//------------------------------------------------------------------------------
// NotifyListener
//------------------------------------------------------------------------------
void AsyncTask::NotifyListener(int code, int param) {
  CritSecLock locker(lock_);
  if (listener_window_) {
    PostMessage(listener_window_,
                code + listener_message_base_,
                param,
                reinterpret_cast<LPARAM>(this));
  }
}

//------------------------------------------------------------------------------
// HttpGet
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

  if (!ActiveXUtils::IsOnline()) {
    if (error_message) {
      *error_message = kIsOfflineErrorMessage;
    }
    return false;
  }

  if (required_cookie && required_cookie[0]) {
    std::string16 required_cookie_str(required_cookie);
    std::string16 name, value;
    ParseCookieNameAndValue(required_cookie_str, &name, &value);
    if (value != kNegatedRequiredCookieValue) {
      CookieMap cookie_map;
      cookie_map.LoadMapForUrl(full_url, browsing_context_.get());
      if (!cookie_map.HasLocalServerRequiredCookie(required_cookie_str)) {
        if (error_message) {
          *(error_message) = kCookieRequiredErrorMessage;
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

  if (!http_request->Open(method, full_url, true, browsing_context_.get())) {
    return false;
  }

  if (is_capturing) {
    http_request->SetRedirectBehavior(HttpRequest::FOLLOW_NONE);
    if (!http_request->SetRequestHeader(HttpConstants::kXGoogleGearsHeader,
                                        STRING16(L"1"))) {
      return false;
    }
  }

  if (reason_header_value && reason_header_value[0]) {
    if (!http_request->SetRequestHeader(HttpConstants::kXGearsReasonHeader,
                                        reason_header_value)) {
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

  if (if_mod_since_date && if_mod_since_date[0]) {
    if (!http_request->SetRequestHeader(HttpConstants::kIfModifiedSinceHeader,
                                        if_mod_since_date)) {
      return false;
    }
  }

  if (disable_browser_cookies) {
    if (!http_request->SetCookieBehavior(
        HttpRequest::DO_NOT_SEND_BROWSER_COOKIES)) {
      return false;
    }
  }

  payload->data.reset();
  http_request->SetListener(this, false);
  ready_state_changed_event_.Reset();

  // Rely on logic inside HttpRequest to check for valid combinations of
  // method and presence of body.
  bool result = http_request->Send(post_body);
  if (!result) {
    http_request->SetListener(NULL, false);
    return false;
  }

  enum {
    kReadyStateChangedEvent = WAIT_OBJECT_0,
    kAbortEvent
  };
  HANDLE handles[] = { ready_state_changed_event_.m_h,
                       abort_event_.m_h };
  bool done = false;
  while (!done) {
    // We need to run a message loop receive COM callbacks from URLMON
    DWORD rv = MsgWaitForMultipleObjectsEx(
                   ARRAYSIZE(handles), handles, INFINITE, QS_ALLINPUT,
#ifdef WINCE
                   MWMO_INPUTAVAILABLE);
#else
                   MWMO_INPUTAVAILABLE | MWMO_ALERTABLE);
#endif
    if (rv == kReadyStateChangedEvent) {
      HttpRequest::ReadyState state;
      if (http_request->GetReadyState(&state)) {
        if (state == HttpRequest::COMPLETE) {
          done = true;
          if (!is_aborted_) {
            int status;
            if (http_request->GetStatus(&status)) {
              payload->status_code = status;
              if (http_request->GetStatusLine(&payload->status_line)) {
                if (http_request->GetAllResponseHeaders(&payload->headers)) {
                  // TODO(bgarcia): Make WebCacheDB::PayloadInfo.data a Blob.
                  http_request->GetResponseBody(payload_data);
                }
              }
            }
          }
        }
      } else {
        LOG16((L"AsyncTask - getReadyState failed, aborting request\n"));
        http_request->Abort();
        done = true;
      }
    } else if (rv == kAbortEvent) {
      LOG16((L"AsyncTask - abort event signalled, aborting request\n"));
      // We abort the request but continue the loop waiting for it to complete
      // TODO(michaeln): paranoia, what if it never does complete, timer?
      http_request->Abort();
    } else {
      // We have message queue input to pump. We pump the queue dry
      // prior to looping and calling MsgWaitForMultipleObjects
      MSG msg;
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  http_request->SetListener(NULL, false);

  if (http_request->WasRedirected()) {
    if (was_redirected) {
      *was_redirected = true;
    }
    if (full_redirect_url) {
      http_request->GetFinalUrl(full_redirect_url);
    }
  }

  return !is_aborted_ && payload_data->get();
}


//------------------------------------------------------------------------------
// HttpRequest::HttpListener::ReadyStateChanged
//------------------------------------------------------------------------------
void AsyncTask::ReadyStateChanged(HttpRequest *source) {
  ready_state_changed_event_.Set();
}
