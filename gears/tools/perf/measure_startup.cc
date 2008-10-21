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

// Helps measure browser startup time.  The sequence is:
// * This tool gets the current time just before starting the browser process.
// * The browser later gets the current time -- using the SAME API -- at
//   whatever event is being measured.
// * The elapsed time is computed manually, by comparing the values.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
 #ifdef WINCE
  #define execv(A,B)  printf("WinCE doesn't have execv!\n")
 #else
  #include <process.h>
  #define execv _execv
 #endif // WINCE
  #define PRINTF_64BIT_SPECIFIER "I64"
#else
  #include <unistd.h>
  #define PRINTF_64BIT_SPECIFIER "ll"
#endif

#include "gears/base/common/basictypes.h"
int64 GetCurrentTimeMillis();

// TODO(cprince): Build against stopwatch.* instead, when it is OS-specific.
int64 GetTickValueMicros() {
  return 1000 * GetCurrentTimeMillis();
}
int64 GetCurrentTimeMillis() {
  static int64 i = 0;
  return ++i;
}  


// Returns the granularity successive timer reads.
// The value is often greater than 1 unit!!
int GetTimerGranularity() {
  const int kNumSamples = 4;
  int64 samples[kNumSamples];
  int num_dupes[kNumSamples] = {0};  // #times we saw same value
  samples[0] = GetTickValueMicros();

  for (int i = 1; i < kNumSamples; ++i) {
    int64 prev_value = samples[i - 1];
    int64 value = prev_value;
    while (value == prev_value) {
      ++num_dupes[i];
      value = GetTickValueMicros();
    }
    samples[i] = value;
  }

  //// If you need more detailed diagnostics, uncomment this block.
  //printf("Sequential timing samples:\n");
  //for (int i = 0; i < kNumSamples; ++i) {
  //  printf("  %" PRINTF_64BIT_SPECIFIER "d (%d dupes)\n",
  //         samples[i], num_dupes[i]);
  //}

  int min_delta = INT_MAX;
  for (int i = 1; i < kNumSamples; ++i) {
    int delta = static_cast<int>(samples[i] - samples[i - 1]);
    if (delta < min_delta) {
      min_delta = delta;
    }
  }
  return min_delta;
}

int main(int argc, char** argv) {
  // Display the timer granularity.
  printf("Timer granularity: %d units.\n", GetTimerGranularity());

  // Check arguments.
  const int kRequiredArguments = 1;  // "real" arguments, excluding argv[0]
  if (argc < (1 + kRequiredArguments)) {
    printf("USAGE: %s PROCESS_NAME [ARG1 [ARG2...]]", argv[0]);
    exit(1);
  }

  // Echo the process being executed.
  printf("COMMAND LINE:");
  for (int i = 1; i < argc; ++i) {
    printf(" %s", argv[i]);
  }
  printf("\n");

  // Shift all args by one position, because exec() takes an array argument
  // that must contain a terminating NULL.
  for (int i = 1; i < argc; ++i) {
    argv[i - 1] = argv[i];
  }
  argv[argc - 1] = NULL;

  // Get the current time, and start the process.
  int64 start = GetTickValueMicros();
  // TODO(cprince): Should print time *after* starting child process, to avoid
  // introducing delay.  But that requires a function other than fork/exec.
  printf("START TIMESTAMP: %" PRINTF_64BIT_SPECIFIER "d\n", start);
  execv(argv[0], &argv[0]);
  // On success, exec doesn't return; it *replaces* the current process!
  printf("\n");
  printf("ERROR: failed to launch process!  Did you provide the full path?\n");

  return 0;
}
