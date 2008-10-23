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

#include "gears/workerpool/common/workerpool_utils.h"

#include "gears/base/common/js_runner.h"
#include "gears/base/common/string_utils.h"

const char16 *kWorkerInsertedFactoryName    = STRING16(L"gearsFactory");
const char16 *kWorkerInsertedWorkerPoolName = STRING16(L"gearsWorkerPool");

const char *kWorkerInsertedFactoryNameAscii    = "gearsFactory";
const char *kWorkerInsertedWorkerPoolNameAscii = "gearsWorkerPool";

const char16 *kWorkerInsertedPreamble = STRING16(
  L"var google = {};"
  L"google.gears = {};"
  L"google.gears.factory = gearsFactory;"
  L"google.gears.workerPool = gearsWorkerPool;");

// The "owning" worker is the first worker that creates the workerpool and
// whose deletion causes the workerpool to shutdown.
const int kOwningWorkerId = 0;
const int kInvalidWorkerId = -1;

void FormatWorkerPoolErrorMessage(const JsErrorInfo &error_info,
                                  int src_worker_id,
                                  std::string16 *message) {
  *message = STRING16(L"Error in worker ");
  *message += IntegerToString16(src_worker_id);

  if (error_info.line != 0) {
    *message += STRING16(L" at line ");
    *message += IntegerToString16(error_info.line);
  }

  *message += STRING16(L". ");
  *message += error_info.message;
}
