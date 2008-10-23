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

#if defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>

#include "gears/base/common/mutex.h"
#include "gears/base/common/stopwatch.h"  // For GetCurrentTimeMillis()


void ThreadYield() {
  sched_yield();
}


Mutex::Mutex()
#ifdef DEBUG
    : owner_(0)
#endif
{
  pthread_mutex_init(&mutex_, NULL);
}


Mutex::~Mutex() {
  pthread_mutex_destroy(&mutex_);
}


void Mutex::Lock() {
#ifdef DEBUG
  // Google frowns upon mutex re-entrancy
  if (owner_ == pthread_self()) {
    LOG(("Thread %p locked mutex %p recursively\n",
         reinterpret_cast<void *>(pthread_self()),
         this));
    assert(false);
  }
#endif
  pthread_mutex_lock(&mutex_);
#ifdef DEBUG
  if (owner_ != 0) {
    LOG(("Thread %p locked mutex %p despite ownership by thread %p\n",
         reinterpret_cast<void *>(pthread_self()),
         this,
         reinterpret_cast<void *>(owner_)));
    assert(false);
  }
  owner_ = pthread_self();
#endif
}


void Mutex::Unlock() {
#ifdef DEBUG
  if (owner_ != pthread_self()) {
    LOG(("Thread %p tried to unlock mutex %p owned by thread %p\n",
         reinterpret_cast<void *>(pthread_self()),
         this,
         reinterpret_cast<void *>(owner_)));
    assert(false);
  }
  owner_ = 0;
#endif
  pthread_mutex_unlock(&mutex_);
}


// CondVar implementation
//
// For POSIX-conforming systems, we implement CondVar in terms of
// pthread conditions.


CondVar::CondVar() {
  pthread_cond_init(&cond_, NULL);
}


CondVar::~CondVar() {
#ifdef DEBUG
  int retcode = pthread_cond_destroy(&cond_);
  assert(retcode != EBUSY);
#else
  pthread_cond_destroy(&cond_);
#endif
}


void CondVar::Wait(Mutex *mutex) {
#ifdef DEBUG
  if (mutex->owner_ != pthread_self()) {
    LOG(("Thread %p called CondVar::Wait() with mutex %p owned by thread %p\n",
         reinterpret_cast<void *>(pthread_self()),
         mutex,
         reinterpret_cast<void *>(mutex->owner_)));
    assert(false);
  }
  mutex->owner_ = 0;
#endif
  pthread_cond_wait(&cond_, &(mutex->mutex_));
#ifdef DEBUG
  if (mutex->owner_ != 0) {
    LOG(("Thread %p left CondVar::Wait() with mutex %p owned by thread %p\n",
         reinterpret_cast<void *>(pthread_self()),
         mutex,
         reinterpret_cast<void *>(mutex->owner_)));
    assert(false);
  }
  mutex->owner_ = pthread_self();
#endif
}


bool CondVar::WaitWithTimeout(Mutex *mutex, int milliseconds) {
  // We've been handed a time duration, but pthread_cond_timedwait requires
  // an absolute time for its argument.
  struct timeval current_time;
  struct timespec timeout;
  gettimeofday(&current_time, NULL);
  timeout.tv_sec = current_time.tv_sec + (milliseconds / 1000);
  timeout.tv_nsec = (current_time.tv_usec * 1000 +
                     (milliseconds % 1000) * 1000000);
  if (timeout.tv_nsec > 999999999) {
    timeout.tv_sec += 1;
    timeout.tv_nsec -= 1000000000;
  }
#ifdef DEBUG
  if (mutex->owner_ != pthread_self()) {
    LOG(("Thread %p called CondVar::WaitWithTimeout() "
         "with mutex %p owned by thread %p\n",
         reinterpret_cast<void *>(pthread_self()),
         mutex,
         reinterpret_cast<void *>(mutex->owner_)));
    assert(false);
  }
  mutex->owner_ = 0;
#endif
  int retcode;
  do {
    retcode = pthread_cond_timedwait(&cond_, &(mutex->mutex_), &timeout);
  } while (retcode == EINTR);
#ifdef DEBUG
  if (mutex->owner_ != 0) {
    LOG(("Thread %p left CondVar::WaitWithTimeout() "
         "with mutex %p owned by thread %p\n",
         reinterpret_cast<void *>(pthread_self()),
         mutex,
         reinterpret_cast<void *>(mutex->owner_)));
    assert(false);
  }
  mutex->owner_ = pthread_self();
#endif
  return retcode == ETIMEDOUT;
}


void CondVar::SignalAll() {
  pthread_cond_broadcast(&cond_);
}

#endif  // defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
