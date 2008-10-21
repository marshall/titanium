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

#ifndef GEARS_CRASH_SENDER_CRASH_SENDER_OSX__
#define GEARS_CRASH_SENDER_CRASH_SENDER_OSX__

// This component uses the HTTPMultipartUpload of the breakpad project to send
// the minidump and associated data to the Google crash reporting servers.
// It will perform throttling based on the parameters passed to it and will
// prompt the user to send the minidump.

#include <Foundation/Foundation.h>

#import "gears/base/common/exception_handler_osx/google_breakpad.h"

@interface Reporter : NSObject {
 @private
  int configFile_;                    // File descriptor for config file
  NSMutableDictionary *parameters_;   // Key value pairs of data (STRONG)
  NSData *minidumpContents_;          // The data in the minidump (STRONG)
}

@end
#endif  // GEARS_CRASH_SENDER_CRASH_SENDER_OSX__
