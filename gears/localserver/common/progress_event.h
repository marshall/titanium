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

#ifndef GEARS_OPENSOURCE_GEARS_LOCALSERVER_COMMON_PROGRESS_EVENT_H__
#define GEARS_OPENSOURCE_GEARS_LOCALSERVER_COMMON_PROGRESS_EVENT_H__

#include "gears/base/common/async_router.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/scoped_refptr.h"

class HttpRequest;

// ProgressEvent is used to signal progress from the thread that is
// transferring data to the ProgressEvent::Listener, within the Listener's
// thread of existence.
class ProgressEvent : public AsyncFunctor {
 public:
  class Listener {
   public:
    Listener();
    // The callback to be made when progress occurs.
    virtual void OnUploadProgress(int64 position, int64 total) = 0;
   private:
    friend class ProgressEvent;
    struct ProgressInfo {
      ProgressInfo() : position(0), total(0), reported(0) {}
      int64 position;
      int64 total;
      int64 reported;
    };
    ThreadId thread_;
    Mutex lock_;
    ProgressInfo info_;
  };

  static void Update(HttpRequest *request, Listener *listener,
                     int64 position, int64 total);
  ProgressEvent(HttpRequest *request, Listener *listener);
  virtual void Run();

 private:
  // Keep a handle on request_ to make sure it is not destroyed.
  scoped_refptr<HttpRequest> request_;
  Listener *listener_;
};

#endif  // GEARS_OPENSOURCE_GEARS_LOCALSERVER_COMMON_PROGRESS_EVENT_H__
