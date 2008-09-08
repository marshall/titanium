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

#ifndef GEARS_WORKERPOOL_COMMON_WORKERPOOL_UTILS_H__
#define GEARS_WORKERPOOL_COMMON_WORKERPOOL_UTILS_H__

#include "gears/base/common/js_runner.h"
#include "gears/base/common/string16.h"

extern const char16 *kWorkerInsertedFactoryName;
extern const char16 *kWorkerInsertedWorkerPoolName;

extern const char *kWorkerInsertedFactoryNameAscii;
extern const char *kWorkerInsertedWorkerPoolNameAscii;

extern const char16 *kWorkerInsertedPreamble;

// The "owning" worker is the first worker that creates the workerpool and
// whose deletion causes the workerpool to shutdown.
extern const int kOwningWorkerId;
extern const int kInvalidWorkerId;

// Creates an error message for display in the parent when a child worker has an
// unhandled exception. The line number is only included in the message if it is
// non-zero.
void FormatWorkerPoolErrorMessage(const JsErrorInfo &error_info,
                                  int src_worker_id,
                                  std::string16 *message);

#endif  // GEARS_WORKERPOOL_COMMON_WORKERPOOL_UTILS_H__
