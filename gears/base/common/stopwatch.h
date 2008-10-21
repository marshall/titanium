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

#ifndef GEARS_BASE_COMMON_STOPWATCH_H__
#define GEARS_BASE_COMMON_STOPWATCH_H__

#include "gears/base/common/basictypes.h"
#include "gears/base/common/mutex.h"

// Returns the current time in milliseconds since the epoch (midnight January 1,
// 1970 GMT).
int64 GetCurrentTimeMillis();

// Returns a monotonically incrementing counter.
int64 GetTicks();

// Returns the number of microseconds elapsed between the start and end tick
// counts.
int64 GetTickDeltaMicros(int64 start, int64 end);

// Simple perf timer. Supports nested calls.
class Stopwatch {
 public:
  Stopwatch() : start_ticks_(0), total_ticks_(0), nested_count_(0) {};
  void Start();
  void Stop();
  // Returns the number of milliseconds elapsed.
  int GetElapsed();

 private:
  Mutex mutex_;
  // The starting number and total number of ticks.
  int64 start_ticks_;
  int64 total_ticks_;
  int nested_count_;
};

// Times an individual block of code.
class ScopedStopwatch {
 public:
  ScopedStopwatch(Stopwatch *t);
  ~ScopedStopwatch();

 private:
  Stopwatch *t_;
};

#endif  // GEARS_BASE_COMMON_STOPWATCH_H__
