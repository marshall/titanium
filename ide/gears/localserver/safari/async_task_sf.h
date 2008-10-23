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

#ifndef GEARS_LOCALSERVER_SAFARI_ASYNC_TASK_SF_H__
#define GEARS_LOCALSERVER_SAFARI_ASYNC_TASK_SF_H__

#include <mach/mach.h>
#include <pthread.h>
#include <vector>

#include "gears/base/common/browsing_context.h"
#include "gears/base/common/common.h"
#include "gears/base/safari/scoped_cf.h"
#include "gears/localserver/common/critical_section.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/common/localserver_db.h"

class BlobInterface;

class AsyncTask {
 public:
  // Starts a worker thread which will call the Run method
  bool Start();

  // Gracefully aborts a worker thread that was previously started
  void Abort();

  // Instructs the AsyncTask to delete itself when the worker thread terminates
  void DeleteWhenDone();

  // Safari specific API
  // Sets where notification messages will be sent.
  class Listener {
   public:
    virtual void HandleEvent(int msg_code,
                             int msg_param,
                             AsyncTask *source) = 0;
  };
  void SetListener(Listener *listener);

 protected:
  // Members that derived classes in common code depend on
  // TODO(michaeln): perhaps define these in a interface IAsyncTask

  static const char16 *kCookieRequiredErrorMessage;

  AsyncTask(BrowsingContext *browsing_context);
  virtual ~AsyncTask();

  bool Init();
  virtual void Run() = 0;

  // Posts a message to our listener
  void NotifyListener(int code, int param);

  // Synchronously fetches a url on the worker thread.  This should only
  // be called on the Run method's thread of control.
  //
  // If 'is_capturing' is true, the custom "X-Gears-Google" header is
  // added, and redirects will not be followed, rather the 30x response
  // will be directly returned.
  //
  // If 'if_mode_since_date' is not null and not an empty string, the
  // request will be a conditional HTTP GET.
  //
  // If 'required_cookie' is not null or empty, the presense of the
  // cookie is tested prior to issueing the request. If the test fails
  // the return value is false and 'error_message' will contain
  // kCookieRequiredErrorMessage.
  //
  // Upon successful return, the 'payload' structure will contain the
  // response. If redirection was followed, 'was_redirected' will be set
  // to true, 'full_redirect_url' will contain the final location in
  // the chain of redirects, and the 'payload' will contain the response
  // for the final location. The 'was_redirected', 'full_redirect_url',
  // and 'error_message' parameters may be null.
  //
  // If Abort() is called from another thread of control while a request
  // is pending, the request is cancelled and HttpGet will return
  // shortly thereafter.

  // 'payload.data' will not be initialized upon return. Instead, the
  // response body will be contained by the 'payload_data' Blob.
  //  TODO(andreip): remove payload_data argument once
  // WebCacheDB::PayloadInfo.data is a Blob.
  bool HttpGet(const char16 *full_url,
               bool is_capturing,
               const char16 *reason_header_value,
               const char16 *if_mod_since_date,
               const char16 *required_cookie,
               WebCacheDB::PayloadInfo *payload,
               scoped_refptr<BlobInterface> *payload_data,
               bool *was_redirected,
               std::string16 *full_redirect_url,
               std::string16 *error_message);

  // As HttpGet, but for POST.
  bool HttpPost(const char16 *full_url,
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
                std::string16 *error_message);

  CriticalSection lock_;
  bool is_aborted_;
  bool is_initialized_;
  scoped_refptr<BrowsingContext> browsing_context_;

 private:
  // Implementation of HttpGet and HttpPost.
  bool MakeHttpRequest(const char16 *method,
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
                       std::string16 *error_message);

  static const int kThreadDoneMessageCode = -1;
  
  // Thread entry point
  static void *ThreadEntry(void *task);
  
  void OnThreadDone();

  struct ThreadMessage {
    mach_msg_header_t header;
    int               code;
    int               param;
  };

  // Main thread will receive messages from worker threads here
  static void OnAsyncCall(CFMachPortRef port, void *msg, CFIndex size,
                          void *info);

  bool is_destructing_;
  bool delete_when_done_;
  pthread_t thread_;
  Listener *listener_;
  scoped_CFMachPort msg_port_;
};

#endif  // GEARS_LOCALSERVER_SAFARI_ASYNC_TASK_SF_H__
