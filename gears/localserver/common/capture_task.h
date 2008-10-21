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

#ifndef GEARS_LOCALSERVER_COMMON_CAPTURE_TASK_H__
#define GEARS_LOCALSERVER_COMMON_CAPTURE_TASK_H__

#include <set>
#include <vector>
#include "gears/base/common/js_types.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/async_task.h"
#include "gears/localserver/common/resource_store.h"


//------------------------------------------------------------------------------
// A CaptureRequest encapslates the parameters to a 
// ResourceStore.capture() API call. Multiple urls can be specified
// in a single API call.
//------------------------------------------------------------------------------
struct CaptureRequest {
  // captureId assigned by GearsResourceStore for this request
  int id;

  // list of un-resolved urls provided by the JavaScript caller
  std::vector<std::string16> urls;

  // list of resolved urls in the same order as urls
  std::vector<std::string16> full_urls;

  // JavaScript function to call when the request is complete
  scoped_ptr<JsRootedCallback> callback;

  CaptureRequest() : id(0) {}
};

//------------------------------------------------------------------------------
// A CaptureTask processes a CaptureRequest asynchronously in the background. 
// Notification messages are sent to the listener as each url in the request
// is completed.
//------------------------------------------------------------------------------
class CaptureTask : public AsyncTask {
 public:
  // Notification message codes sent to listeners
  enum {
    CAPTURE_TASK_COMPLETE = 0,
    CAPTURE_URL_SUCCEEDED = 1,
    CAPTURE_URL_FAILED = 2
  };

  CaptureTask(BrowsingContext *context)
      : AsyncTask(context), capture_request_(NULL) {}

  bool Init(ResourceStore *store, CaptureRequest *request);

 private:
  virtual void Run();
  int GetUrlCount();
  bool GetUrl(int index, std::string16 *url);
  bool ProcessUrl(const std::string16 &url);
  void NotifyUrlComplete(int index, bool success);
  void NotifyTaskComplete(bool success);
  bool HttpGetUrl(const char16 *full_url,
                  const char16 *if_mod_since_date,
                  WebCacheDB::PayloadInfo *payload);

  ResourceStore store_;
  CaptureRequest *capture_request_;
  std::set<std::string16> processed_urls_;
};

#endif  // GEARS_LOCALSERVER_COMMON_CAPTURE_TASK_H__
