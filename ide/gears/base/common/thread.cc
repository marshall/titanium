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

#include <assert.h>
#include "gears/base/common/event.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/thread.h"
#if defined(WIN32) || defined(WINCE)
#include "gears/base/common/thread_win32.h"
#elif defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
#include "gears/base/common/thread_posix.h"
#else
#error "Unknown threading platform"
#endif

Thread::Thread()
    : is_running_(false),
      thread_id_(0),
      started_event_(),
      internal_(new ThreadInternal()) {
}

Thread::~Thread() {
  // The thread should have either terminated before destruction, or
  // never have been started.
  assert(!is_running_ && thread_id_ == 0);
}

ThreadId Thread::Start() {
  assert(!is_running_ && thread_id_ == 0);
  if (internal_->Start(this)) {
    // Wait for the child thread to reach its Run() method.
    started_event_.Wait();
    // The thread should have set thread_id_ and is_running_ before
    // signalling started_event_.
    assert(thread_id_ != 0);
    return thread_id_;
  } else {
    LOG(("Failed to start thread."));
    return 0;
  }
}

void Thread::Join() {
  if (thread_id_ != 0) {
    // Wait for the child thread to terminate, and free its resources.
    internal_->Join();
    thread_id_ = 0;
  }
}

// Called by ThreadInternal on successful thread creation.
void Thread::ThreadMain() {
#ifdef BROWSER_NONE
  // TODO: remove the following when ThreadMessageQueue is implemented
  // per-platform, instead of per-browser.
#if defined(WIN32) || defined(WINCE)
  thread_id_ = ::GetCurrentThreadId();
#elif defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
  thread_id_ = pthread_self();
#endif  // defined(WIN32) || defined(WINCE)
#else
  // Initialize the message queue.
  ThreadMessageQueue* queue = ThreadMessageQueue::GetInstance();
  queue->InitThreadMessageQueue();
  // Get this thread's id.
  thread_id_ = queue->GetCurrentThreadId();
#endif  // !BROWSER_NONE
  // Set running state.
  is_running_ = true;
  // Let our creator know that we started ok.
  started_event_.Signal();
  // Do the actual work.
  Run();
  // Clear running state.
  is_running_ = false;
}
