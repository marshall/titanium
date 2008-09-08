// Copyright 2005, Google Inc.
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

#ifndef GEARS_BASE_COMMON_COMMON_FF_H__
#define GEARS_BASE_COMMON_COMMON_FF_H__

// This header file includes the more frequently used header files and defines
// logging primitives.

#define FORCE_PR_LOG

#include <gecko_sdk/include/prlog.h>
#include <gecko_sdk/include/nsStringAPI.h>
#include <gecko_sdk/include/nsServiceManagerUtils.h>
#include <gecko_sdk/include/nscore.h>
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsComponentManagerUtils.h>

#ifdef PR_LOGGING
extern PRLogModuleInfo *gLog;
#endif
// common_osx.h defines OSX-specific LOG, undef it here.
#undef LOG
#define LOG(args) PR_LOG(gLog, PR_LOG_DEBUG, args)
// Warning: never use %S in LOG(); it breaks on Linux (2- vs 4-byte wide chars)

//-----------------------------------------------------------------------------
// Debug only code to help us assert that class methods are restricted to a
// single thread.  To use, add a DECL_SINGLE_THREAD to your class declaration.
// Then, add ASSERT_SINGLE_THREAD() calls to the top of each class method.
#ifdef DEBUG
#include <gecko_sdk/include/prthread.h>
class CurrentThreadID {
 public:
  CurrentThreadID() {
    thr_ = PR_GetCurrentThread();
  }
  PRThread *get() {
    return thr_;
  }
 private:
  PRThread *thr_;
};
#define DECL_SINGLE_THREAD \
    CurrentThreadID thread_id_;
#define ASSERT_SINGLE_THREAD() \
    PR_ASSERT(thread_id_.get() == PR_GetCurrentThread())
#else
#define DECL_SINGLE_THREAD
#define ASSERT_SINGLE_THREAD()
#endif

#endif // GEARS_BASE_COMMON_COMMON_FF_H__
