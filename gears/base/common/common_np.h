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

#ifndef GEARS_BASE_COMMON_COMMON_NPAPI_H__
#define GEARS_BASE_COMMON_COMMON_NPAPI_H__

//-----------------------------------------------------------------------------
// For the NPAPI on Android build target
//-----------------------------------------------------------------------------
#ifdef ANDROID

// Android logging support. Out-of-line to avoid polluting the
// namespace with Android defines.
extern "C" void android_log_helper(const char *tag,
                                   const char *file,
                                   unsigned line,
                                   const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));

#define LOG_INNER(fmt, args...) \
    do { \
      android_log_helper("Gears", \
                         __FILE__, \
                         __LINE__, \
                         fmt , \
                         ## args ); \
    } while (0)

#ifdef DEBUG

#define LOG(args) \
    do { LOG_INNER args ; } while (0)

#include <assert.h>
#include <pthread.h>

class CurrentThreadID {
 public:
  CurrentThreadID() {
    id_ = pthread_self();
  }
  pthread_t get() {
    return id_;
  }
 private:
  pthread_t id_;
};

#define DECL_SINGLE_THREAD \
    CurrentThreadID thread_id_;

#define ASSERT_SINGLE_THREAD() \
    assert(pthread_equal(pthread_self(), thread_id_.get()))

#else  // !DEBUG

#define LOG(args) do { } while (0)
#define DECL_SINGLE_THREAD
#define ASSERT_SINGLE_THREAD()

#endif

#else  // !ANDROID

//-----------------------------------------------------------------------------
// For the NPAPI on Win32 build target
//-----------------------------------------------------------------------------
#include <windows.h>  // for DWORD
#include "gears/base/ie/atl_headers.h" // TODO(cprince): change ATLASSERT to DCHECK

#define ENABLE_LOGGING
#if defined(DEBUG) && defined(ENABLE_LOGGING)
// ATLTRACE for Win32 can take either a wide or narrow string.
#define LOG(args) ATLTRACE args
#define LOG16(args) ATLTRACE args
#else  // defined(DEBUG) && defined(ENABLE_LOGGING)
#define LOG(args) __noop
#define LOG16(args) __noop
#endif  // defined(DEBUG) && defined(ENABLE_LOGGING)


// Debug only code to help us assert that class methods are restricted to a
// single thread.  To use, add a DECL_SINGLE_THREAD to your class declaration.
// Then, add ASSERT_SINGLE_THREAD() calls to the top of each class method.
#ifdef DEBUG

class CurrentThreadID {
 public:
  CurrentThreadID() {
    id_ = GetCurrentThreadId();
  }
  DWORD get() {
    return id_;
  }
 private:
  DWORD id_;
};

#define DECL_SINGLE_THREAD \
    CurrentThreadID thread_id_;

#define ASSERT_SINGLE_THREAD() \
    ATLASSERT(thread_id_.get() == GetCurrentThreadId())

#else  // !DEBUG
#define DECL_SINGLE_THREAD
#define ASSERT_SINGLE_THREAD()
#endif

#endif // !ANDROID

#endif // GEARS_BASE_COMMON_COMMON_NPAPI_H__
