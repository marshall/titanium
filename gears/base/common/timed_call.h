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
//

#ifndef GEARS_BASE_COMMON_TIMED_CALL_H__
#define GEARS_BASE_COMMON_TIMED_CALL_H__

// For DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/basictypes.h"

// This API requires the thread calling the constructor to execute the
// message processing system (DispatchMessage and friends on Windows)
// or the run loop (some variation of CFRunLoop/NSRunLoop on Mac)
// at some point for the timer expiration to be checked.
// On a UI thread, it is likely to be already running.

// Queues a callback to be called after some period of time.
// Uses one OS timer per thread.
class TimedCall {
 public:
  typedef void (*TimedCallback)(void *arg);

  // 'delay_millis' is how many milliseconds from now to fire the callback.
  // If 'delay_millis' is negative, it will be called the next time around
  // the message/run loop, even if created inside a callback.
  // 'repeat' indicates whether the callback should be called repeatedly.
  // 'callback' is the function to call.
  // 'callback_parameter' is what to pass the function when it is called back.
  TimedCall(int64 delay_millis, bool repeat,
            TimedCallback callback, void *callback_parameter);

  // 'callback_' will not be called after or during destructor.
  // TODO(chimene) test destructor supports being called inside a callback.
  // Do not call on a different thread from the constructor (crash on fire)
  // unless you have already destroyed the constructor's thread
  // TODO(chimene): Fix (and test) this.
  ~TimedCall();

  // Calls 'callback_', re-schedules if 'repeat' is true.
  // Do not call on a different thread from the constructor.
  // Called internally sometime after
  // (GetCurrentTimeMillis() < deadline_) becomes true.
  void Fire();

  // Accessor functions
  int64 deadline() const {
    return deadline_;
  }
  TimedCallback callback() const {
    return callback_;
  }
  void *callback_parameter() const {
    return callback_parameter_;
  }

 private:

  // Timer will go off (GetCurrentTimeMillis + delay_)
  // milliseconds after the constructor is called.
  int64 delay_;

  // If 'repeat_' is true the callback will be called again with the same delay,
  // calculated right before the callback is called.
  // Does not schedule for every 'delay_' ms, only re-schedules
  // the timer for 'delay_' ms in the future right before the timer goes off.
  bool repeat_;

  // Time relative to GetCurrentTimeMillis() when the callback should activate.
  int64 deadline_;

  // The function to be called.
  TimedCallback callback_;

  // Parameter to 'callback_'.
  void *callback_parameter_;

  // The callback will not be called after this is called.
  // Cancel() does not call the callback.
  void Cancel();

  DISALLOW_EVIL_CONSTRUCTORS(TimedCall);
};

#endif  // GEARS_BASE_COMMON_TIMED_CALLBACK_H__
