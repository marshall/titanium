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

// WIN32 systems.
#if defined(WIN32) || defined(WINCE)

#include "gears/base/common/thread_win32.h"

#include <assert.h>
#include <atlsync.h>
#include <windows.h>

#include "gears/base/common/common.h"
#include "gears/base/common/thread.h"
#if defined(WINCE)
#include "gears/base/common/wince_compatibility.h"
#endif

Thread::ThreadInternal::ThreadInternal()
    : handle_(0) {
}

Thread::ThreadInternal::~ThreadInternal() {
  // The thread should have either terminated before destruction, or
  // never have been started.
  assert(handle_ == 0);
}

bool Thread::ThreadInternal::Start(Thread *thread) {
  assert(handle_ == 0);
  // Start the child thread. The child will call ThreadRun with a
  // pointer to the Thread instance.
  handle_ = reinterpret_cast<HANDLE>(
      _beginthreadex(NULL, 0, &ThreadRun, thread, 0, NULL));
  if (handle_ != 0) {
    return true;
  } else {
    LOG(("Failed to start thread: 0x%08x\n", GetLastError()));
    handle_ = 0;
    return false;
  }
}

void Thread::ThreadInternal::Join() {
  if (handle_ == 0) {
    return;
  }
  // This will block until thread termination.
  if (WaitForSingleObject(handle_, INFINITE) != WAIT_OBJECT_0) {
    LOG(("Failed to join child thread: 0x%08x\n", GetLastError()));
  }
  if (!CloseHandle(handle_)) {
    LOG(("Failed to close handle: 0x%08x\n", GetLastError()));
  }
  handle_ = 0;
}

unsigned int __stdcall Thread::ThreadInternal::ThreadRun(void *data) {
  // Call the parent Thread::ThreadMain()
  Thread *thread = reinterpret_cast<Thread *>(data);
  thread->ThreadMain();
  return 0;
}

#endif  // defined(WIN32) || defined(WINCE)
