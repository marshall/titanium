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

#ifndef GEARS_DATABASE2_THREAD_SAFE_QUEUE_H__
#define GEARS_DATABASE2_THREAD_SAFE_QUEUE_H__

#include <queue>

#include "gears/base/common/common.h"
#include "gears/base/common/mutex.h"

template <class T>
class Database2ThreadSafeQueue {
 public:
  Database2ThreadSafeQueue() {};

  // Adds an item to the queue. If *first is non-null, returns whether the
  // pushed item is now first in the queue.
  void Push(T *t, bool *first) {
    MutexLock lock(&mutex_);
    if (first) {
      *first = queue_.empty();
    }
    queue_.push(t);
  }

  // Adds an item to the queue, but only if the queue is empty.
  bool PushIfEmpty(T *t) {
    MutexLock lock(&mutex_);
    if (!queue_.empty()) {
      return false;
    }

    queue_.push(t);
    return true;
  }

  // Removes and returns the front item from the queue. Returns NULL if there
  // are no items in the queue.
  T *Pop() {
    MutexLock lock(&mutex_);
    if (queue_.empty()) {
      return NULL;
    }

    T *t = queue_.front();
    queue_.pop();
    return t;
  }

 private:
  Mutex mutex_;
  std::queue<T*> queue_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2ThreadSafeQueue);
};


#endif // GEARS_DATABASE2_THREAD_SAFE_QUEUE_H__
