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

#import <assert.h>

#import "gears/blob/blob_interface.h"
#import "gears/localserver/common/http_constants.h"
#import "gears/localserver/common/http_cookies.h"
#import "gears/localserver/safari/async_task_sf.h"

const char16 *AsyncTask::kCookieRequiredErrorMessage =
                  STRING16(L"Required cookie is not present");

//------------------------------------------------------------------------------
// AsyncTask
//------------------------------------------------------------------------------
AsyncTask::AsyncTask(BrowsingContext *browsing_context) :
    is_aborted_(false),
    is_initialized_(false),
    browsing_context_(browsing_context),
    is_destructing_(false),
    delete_when_done_(false),
    thread_(NULL),
    listener_(NULL),
    msg_port_(NULL) {
}

//------------------------------------------------------------------------------
// ~AsyncTask
//------------------------------------------------------------------------------
AsyncTask::~AsyncTask() {
  if (thread_) {
    LOG(("~AsyncTask - thread is still running: %p\n", this));
    // extra scope to lock our monitor
    {
      CritSecLock locker(lock_);
      is_destructing_ = true;
      locker.Unlock();
      Abort();
    }
    pthread_join(thread_, NULL);
    thread_ = NULL;
  }
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
  is_initialized_ = true;

  // Create a mach port to communicate between the threads
  CFMachPortContext context;
  context.version = 0;
  context.info = this;
  context.retain = NULL;
  context.release = NULL;
  context.copyDescription = NULL;
  Boolean should_free = false;    // Don't free context.info on dealloc
  msg_port_.reset(CFMachPortCreate(kCFAllocatorDefault, OnAsyncCall, 
                                   &context, &should_free));

  if (!msg_port_.get() || should_free) {
    LOG(("Couldn't make mach port\n"));
    return false;
  }

  // Create a RL source and add it to the current runloop.  This is so that
  // the thread can send a message to the main thread via OnAsyncCall()
  scoped_CFRunLoopSource 
    msg_source(CFMachPortCreateRunLoopSource(kCFAllocatorDefault, 
                                             msg_port_.get(), 0));
  CFRunLoopAddSource(CFRunLoopGetCurrent(), msg_source.get(),
                     kCFRunLoopCommonModes);

  return true;
}

//------------------------------------------------------------------------------
// SetListener
//------------------------------------------------------------------------------
void AsyncTask::SetListener(Listener *listener) {
  listener_ = listener;
}

//------------------------------------------------------------------------------
// Start
//------------------------------------------------------------------------------
bool AsyncTask::Start() {
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  is_aborted_ = false;
  if (pthread_create(&thread_, NULL, ThreadEntry, this))
    return false;

  return (thread_ != NULL);
}

//------------------------------------------------------------------------------
// Abort
//------------------------------------------------------------------------------
void AsyncTask::Abort() {
  CritSecLock locker(lock_);
  if (thread_ && !is_aborted_) {
    is_aborted_ = true;
  }
}

//------------------------------------------------------------------------------
// DeleteWhenDone
//------------------------------------------------------------------------------
void AsyncTask::DeleteWhenDone() {
  CritSecLock locker(lock_);
  assert(!delete_when_done_);
  if (!delete_when_done_) {
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
// ThreadEntry - Our worker thread's entry procedure
//------------------------------------------------------------------------------
void *AsyncTask::ThreadEntry(void *task) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  AsyncTask *asyncTask = reinterpret_cast<AsyncTask*>(task);
  asyncTask->Run();
  asyncTask->NotifyListener(kThreadDoneMessageCode, 0);
  
  [pool release];

  return NULL;
}

//------------------------------------------------------------------------------
// NotifyListener
//------------------------------------------------------------------------------
void AsyncTask::NotifyListener(int code, int param) {
  ThreadMessage msg = {0};
  
  // Post a Mach message to be recieved by the main thread via OnAsyncCall()
  msg.header.msgh_size = sizeof(ThreadMessage);
  msg.header.msgh_remote_port = CFMachPortGetPort(msg_port_.get());
  msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
                                        MACH_MSG_TYPE_MAKE_SEND_ONCE);
  msg.code = code;
  msg.param = param;
  mach_msg(&(msg.header),
           MACH_SEND_MSG | MACH_SEND_TIMEOUT,
           msg.header.msgh_size, 0, 0, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
}

//------------------------------------------------------------------------------
// OnThreadDone
//------------------------------------------------------------------------------
void AsyncTask::OnThreadDone() {
  CritSecLock locker(lock_);
  if (delete_when_done_) {
    thread_ = NULL;
    // In this particular code path, we have to call unlock prior to delete
    // otherwise the locker would try to access deleted memory, &lock_,
    // after it's been freed.
    locker.Unlock();
    delete this;
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
  if (!HttpRequest::Create(&http_request))
    return false;
    
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

  if (!http_request->SetRequestHeader(HttpConstants::kCacheControlHeader,
                                      HttpConstants::kNoCache)) {
    return false;
  }

  if (!http_request->SetRequestHeader(HttpConstants::kPragmaHeader,
                                      HttpConstants::kNoCache)) {
    return false;
  }
  
  if (reason_header_value && reason_header_value[0]) {
    if (!http_request->SetRequestHeader(HttpConstants::kXGearsReasonHeader,
                                        reason_header_value)) {
      return false;
    }
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

  // Rely on logic inside HttpRequest to check for valid combinations of
  // method and presence of body.
  bool result = http_request->Send(post_body);
  if (!result) {
    return false;
  }

  // Wait for the data to come back from the HTTP request
  while (1) {
    HttpRequest::ReadyState ready_state;
    http_request->GetReadyState(&ready_state);

    if (ready_state == HttpRequest::COMPLETE) {
      break;
    }

    if (is_aborted_) {
      http_request->Abort();
      break;
    }

    // Run for a second in the run loop
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, true);
  }

  // Extract the status & data
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

  // If we were redirected during the load, setup the return variables
  if (http_request->WasRedirected()) {
    if (was_redirected)
      *was_redirected = true;

    if (full_redirect_url)
      http_request->GetFinalUrl(full_redirect_url);
  }

  return !is_aborted_ && payload_data->get();
}

//------------------------------------------------------------------------------
// static OnAsyncCall
//------------------------------------------------------------------------------
void AsyncTask::OnAsyncCall(CFMachPortRef port, void *mach_msg, CFIndex size,
                            void *info) {
  ThreadMessage *msg = (ThreadMessage *)mach_msg;
  AsyncTask *asyncTask = (AsyncTask *)info;
  
  if (msg->code == kThreadDoneMessageCode) {
    asyncTask->OnThreadDone();
    return;
  }

  if (asyncTask->listener_)
    asyncTask->listener_->HandleEvent(msg->code, msg->param, asyncTask);
}
