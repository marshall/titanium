// Copyright 2007, Google Inc.
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

#include "gears/base/common/mutex.h"
#include "gears/base/common/stopwatch.h"  // For GetCurrentTimeMillis()


void Mutex::Await(const Condition &cond) {
#ifdef DEBUG
  bool result = AwaitImpl(cond, 0);
  // We call AwaitImpl without a timeout, so it should always return with the
  // condition having become true.
  assert(result);
#else 
  AwaitImpl(cond, 0);
#endif
}


bool Mutex::AwaitWithTimeout(const Condition &cond, int timeout_milliseconds) {
  assert(timeout_milliseconds > 0);
  return AwaitImpl(cond, GetCurrentTimeMillis() + timeout_milliseconds);
}


bool Mutex::AwaitImpl(const Condition &cond, int64 end_time) {
  // end_time is milliseconds since the epoch. A value of zero indicates no
  // timeout.
  while (!cond.Eval() && (end_time == 0 || GetCurrentTimeMillis() < end_time)) {
    Unlock();

    // Yield the rest of our CPU timeslice before reacquiring the lock.
    // Otherwise we'll spin pointlessly here, hurting performance.
    // (The Condition cannot possibly change when no other thread runs.)
    ThreadYield();
    Lock();
  }
  return cond.Eval();
}
