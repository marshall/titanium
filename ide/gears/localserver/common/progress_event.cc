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

#include "gears/localserver/common/progress_event.h"

#include "gears/localserver/common/http_request.h"

//------------------------------------------------------------------------------
// ProgressEvent::Listener
//------------------------------------------------------------------------------

ProgressEvent::Listener::Listener() {
  // TODO(bgarcia): Is there a better location to call InitThreadMessageQueue?
  //                Currently needed here for IE.
  ThreadMessageQueue::GetInstance()->InitThreadMessageQueue();
  // Establishes the thread on which we should make progress calls.
  thread_ = ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
}

//------------------------------------------------------------------------------
// ProgressEvent
//------------------------------------------------------------------------------

void ProgressEvent::Update(HttpRequest *request, Listener *listener,
                           int64 position, int64 total) {
  bool event_pending;
  {
    MutexLock locker(&listener->lock_);
    event_pending = (listener->info_.position != listener->info_.reported);
    listener->info_.position = position;
    listener->info_.total = total;
  }
  if (!event_pending) {
    ProgressEvent *event(new ProgressEvent(request, listener));
    AsyncRouter::GetInstance()->CallAsync(listener->thread_, event);
  }
}

ProgressEvent::ProgressEvent(HttpRequest *request, Listener *listener)
    : request_(request), listener_(listener) {
}

void ProgressEvent::Run() {
  int64 position;
  int64 total;
  {
    MutexLock locker(&listener_->lock_);
    position = listener_->info_.position;
    total = listener_->info_.total;
    listener_->info_.reported = position;
  }
  listener_->OnUploadProgress(position, total);
}
