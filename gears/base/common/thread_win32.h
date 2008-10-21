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

#ifndef GEARS_BASE_COMMON_THREAD_WIN32_H__
#define GEARS_BASE_COMMON_THREAD_WIN32_H__

// WIN32 systems.
#if defined(WIN32) || defined(WINCE)

#include <atlsync.h>
#include "gears/base/common/thread.h"
#if defined(WINCE)
#include "gears/base/common/wince_compatibility.h"
#endif

// WIN32 implementation of ThreadInternal.
class Thread::ThreadInternal {
 public:
  ThreadInternal();
  ~ThreadInternal();
  
  // Start a new thread. On success, the child thread will invoke
  // thread->ThreadMain() and return true. Returns false on failure.
  bool Start(Thread *thread);
  // Wait for thread termination and clean up thread resources. In the
  // case of WIN32, child thread termination automatically deletes its
  // stack, but this will additionally ensure the thread handle is
  // closed.
  void Join();

 private:
  // WIN32 thread handle.
  HANDLE handle_;

  // Function called on the child thread. This will invoke
  // thread->ThreadMain().
  static unsigned int __stdcall ThreadRun(void *data);
};

#endif // defined(WIN32) || defined(WINCE)

#endif  // GEARS_BASE_COMMON_THREAD_WIN32_H__
