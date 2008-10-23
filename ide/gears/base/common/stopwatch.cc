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

#include "gears/base/common/stopwatch.h"

#include <assert.h>

// Implementation of Stopwatch. Implementations of GetCurrentTimeMillis(),
// GetTicks() and GetTickDeltaMicros() are platform dependent are implemented in
// stopwatch_xxx.cc.

void Stopwatch::Start() {
  MutexLock lock(&mutex_);

  if (nested_count_ == 0) {
    start_ticks_ = GetTicks();
  }

  nested_count_++;
}

void Stopwatch::Stop() {
  MutexLock lock(&mutex_);

  // You shouldn't call stop() before ever calling start; that would be silly.
  assert(nested_count_ > 0);

  nested_count_--;
  if (nested_count_ == 0) {
    total_ticks_ += GetTicks() - start_ticks_;
  }
}

int Stopwatch::GetElapsed() {
  // Return the number of milliseconds elapsed.
  return static_cast<int>(GetTickDeltaMicros(0, total_ticks_) / 1000);
}


ScopedStopwatch::ScopedStopwatch(Stopwatch *t) {
  assert(t);
  t_ = t;
  t_->Start();
}

ScopedStopwatch::~ScopedStopwatch() {
  assert(t_);
  t_->Stop();
}
