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

// This file is used by both Linux and OSX.
// TODO(cprince): remove platform-specific #ifdef guards when OS-specific
// sources (e.g. LINUX_CPPSRCS) are implemented
#if defined(LINUX) || defined(OS_MACOSX) || defined(ANDROID)

#include "gears/base/common/stopwatch.h"

#include <assert.h>
#include <sys/time.h>

// Returns the current time in milliseconds since the epoch (midnight January 1,
// 1970 GMT).
int64 GetCurrentTimeMillis() {
  return GetTicks() / 1000;
}

// Returns a monotonically incrementing counter.
int64 GetTicks() {
  // gettimeofday() has microsecond resolution, which is good enough, so we use
  // that for simplicity. Note that this isn't strictly monotonic because the
  // clock could be reset.
  struct timeval t;
  int ret = gettimeofday(&t, 0);
  return ret == 0 ? (t.tv_sec * 1000000LL) + t.tv_usec : 0;
}

// Returns the number of microseconds elapsed between the start and end tick
// counts.
int64 GetTickDeltaMicros(int64 start, int64 end) {
  // Ticks are always microseconds.
  return end - start;
}

#endif
