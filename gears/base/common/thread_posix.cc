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

// POSIX systems.
#if defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)

#include <assert.h>
#include <pthread.h>
#include "gears/base/common/thread_posix.h"
#include "gears/base/common/thread.h"
#if defined(OS_ANDROID)
#include "gears/base/android/java_jni.h"
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
  if (pthread_create(&handle_, NULL, &ThreadRun, thread) == 0) {
    return true;
  } else {
    LOG(("Failed to start thread\n"));
    handle_ = 0;
    return false;
  }
}

void Thread::ThreadInternal::Join() {
  if (handle_ == 0) {
    return;
  }
  // This will block until thread termination.
  if (pthread_join(handle_, NULL) != 0) {
    LOG(("Failed to join child thread\n"));
    assert(false);
  }
  handle_ = 0;
}

void *Thread::ThreadInternal::ThreadRun(void *data) {
#ifdef OS_ANDROID
  // Android requires threads to register themselves with the VM, in
  // case the therad ends up calling some managed code.
  JniAttachCurrentThread();
#endif
  // Call the parent Thread::ThreadMain()
  Thread *thread = reinterpret_cast<Thread *>(data);
  thread->ThreadMain();
#ifdef OS_ANDROID
  // Unregister from the Android VM.
  JniDetachCurrentThread();
#endif
  return NULL;
}

#endif // defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
