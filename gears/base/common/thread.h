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

#ifndef GEARS_BASE_COMMON_THREAD_H__
#define GEARS_BASE_COMMON_THREAD_H__

#include "gears/base/common/event.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// TODO(michaeln): Perhaps always use an 'int' to avoid native thread types from
// leaking thru in this common interface.
#if BROWSER_FF
typedef int ThreadId;
#elif BROWSER_WEBKIT || defined(LINUX) || defined(ANDROID)
typedef pthread_t ThreadId;
#elif BROWSER_IE || defined(WIN32) || defined(WINCE)
typedef DWORD ThreadId;
#endif

// This class abstracts creating a new thread. Derived classes implement the Run
// method, which is run in a new thread when Start is called.
class Thread {
 public:
  Thread();
  virtual ~Thread();

  // Starts a thread and invokes this->Run() as its body. Returns the
  // child thread's ID if successfully started, or 0 on error.
  ThreadId Start();

  // Waits for the Run method to complete, freeing any OS-specific
  // thread handle resources associated with the child thread. Must be
  // called before destruction if a thread was started successfully.
  void Join();
  // Returns true if the child thread is running.
  bool IsRunning() const { return is_running_; }
  // Returns the child thread's ID, or 0 if not running.
  ThreadId GetThreadId() const { return is_running_ ? thread_id_ : 0; }

 protected:
  // This method is called in the child thread. The caller ensures the
  // thread environment is setup before the call and destructed on
  // return.
  virtual void Run() = 0;

 private:
  // OS-specific details, forward declared here.
  class ThreadInternal;

  // Set and cleared by the child thread around the call to Run().
  bool is_running_;
  // OS-independent thread identifier, set by the child thread, reset
  // by Join().
  ThreadId thread_id_;
  // Event signalled by the child thread after initialization.
  Event started_event_;
  // OS-specific details such as the underlying handle.
  scoped_ptr<ThreadInternal> internal_;

  // Initializes the message queue for this thread and calls Run().
  // Called by ThreadInternal on successful child thread creation.
  void ThreadMain();
};

#endif  // GEARS_BASE_COMMON_THREAD_H__
