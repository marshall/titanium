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

// This file contains workarounds for issues with WebKit and OSX that affect 
// Gears.  See source file for full details.

#ifndef GEARS_BASE_SAFARI_SAFARI_WORKAROUNDS_H__
#define GEARS_BASE_SAFARI_SAFARI_WORKAROUNDS_H__

#if defined(__cplusplus)
extern "C" {
#endif

// Enables timers in nested runloops, see comments in .m file for full details.
void EnableWebkitTimersForNestedRunloop();
#if defined(__cplusplus)
}
#endif

// Functions defined in tools/osx/libleopard_support.a.

// Apply workaround for rdar://problem/5830581
void ApplyProtocolWorkaround();

// Workaround for rdar://problem/5817126 
NSURLResponse *LeopardCreateNSURLResponse(NSURL *url, int status_code,
                                          NSDictionary *headers);

#endif  // GEARS_BASE_SAFARI_SAFARI_WORKAROUNDS_H__
