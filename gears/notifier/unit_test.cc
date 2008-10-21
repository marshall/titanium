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

#if defined(OFFICIAL_BUILD) || !defined(USING_CCTESTS)
// The notification API has not been finalized for official builds.
int main(int, char **) {
  return 0;
}
#else
#include "gears/notifier/unit_test.h"

#include "gears/base/common/string16.h"
#include "gears/base/common/timed_call_test.h"
#include "third_party/gtest/include/gtest/gtest.h"

TEST(TimedCall, BasicCallback) {
  std::string16 error;
  bool success = TestTimedCallbackAll(&error);

  if (!success) {
    FAIL() << error.c_str();
  }
}

int RunTests(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// For OSX, 'main' is defined in unit_test_osx.mm.
#ifndef OS_MACOSX
int main(int argc, char **argv) {
  return RunTests(argc, argv);
}
#endif  // OS_MACOSX
#endif  // defined(OFFICIAL_BUILD) || !defined(USING_CCTESTS)
