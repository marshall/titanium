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
//
// The Event class encapsulates a 'signal' bool along with a Mutex and Condition
// to guard its access.  This makes the common Signal() and Wait() semantics
// simpler to use, and less error-prone.
//
// An event is either signalled or un-signalled. A call to Event::Wait() will
// block the calling thread until the event is signalled from another thread. An
// event is signalled by calling Event::Signal(). This will unblock one waiting
// thread. New Event objects are initially in the unsignalled state. Event
// objects reset to the unsignalled state as soon as a thread waiting on the
// event has been unblocked. It is valid to signal an event before any threads
// are waiting on it. If a thread subsequently waits on such an event, it will
// proceed immediately, and the event will reset.

// Event::WaitWithTimeout() blocks as does Wait(), but will unblock after the
// specified time interval has elapsed, if the event is not signalled in this
// time. Note that there is no guarantee that the thread will be unblocked
// immediately once the specified interval has elapsed.
//
// See event_test.cc for simple examples.

#ifndef GEARS_BASE_COMMON_EVENT_H__
#define GEARS_BASE_COMMON_EVENT_H__

#include "gears/base/common/basictypes.h"  // For DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/mutex.h"

class Event {
 public:
  Event();

  // Signals the event.
  void Signal();

  // Blocks until the event is signalled.
  void Wait();
  // Blocks until the event is signalled or until the specified time limit
  // expires. Returns true if the event was signalled.
  bool WaitWithTimeout(int timeout_milliseconds);

 private:
  Mutex mutex_;
  bool signal_;
  CondVar condition_;
  DISALLOW_EVIL_CONSTRUCTORS(Event);
};

#endif // GEARS_BASE_COMMON_EVENT_H__
