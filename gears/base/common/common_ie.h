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

#ifndef GEARS_BASE_COMMON_COMMON_IE_H__
#define GEARS_BASE_COMMON_COMMON_IE_H__

#include <windows.h>  // for DWORD
#include <assert.h>
#include <stdio.h>
// TODO(cprince): change ATLASSERT to DCHECK
#include "gears/base/ie/atl_headers.h"


#ifdef WINCE
// Use of ATLTRACE (which is used by LOG and LOG16) may cause a stack fault on
// WinCE. See http://code.google.com/p/google-gears/issues/detail?id=342 for
// details. So we disable logging by default on WinCE.
#else
#define ENABLE_LOGGING
#endif

#if defined(DEBUG) && defined(ENABLE_LOGGING)
#if defined(WINCE)
// ATLTRACE for WinCE takes a wide string, so we can not call it directly.
// Instead we convert the message and call ATL::CTrace so that we can pass the
// the file name and line number from the call site.
//
// TODO(cprince): Remove this class as part of LOG() refactoring.
// Also note that LOG() calls should take string16, so the string conversion
// done below can go away at that time.
class GearsTrace {
 public:
  GearsTrace(const char* file_name, int line_no)
      : file_name_(file_name), line_no_(line_no) {}

  void operator() (const char* format, ...) const {
    // Print the message as a narrow string.
    //
    // The Windows implementation of (v)sn(w)printf() returns -1 if the output
    // is truncated. More sensible implementations return the number of
    // characters that would have been written, so the buffer can be
    // re-allocated to the correct size. Also, Windows does not provide
    // asnprintf(). The simplest option is to use (v)sn(w)printf() with a fixed
    // buffer size.
    //
    // (v)sn(w)printf() only null-terminates the string if there is space (ie
    // the string length is strictly less than the given buffer size). If the
    // string length is equal to the given buffer size, (v)sn(w)printf() will
    // not return a truncation error but the string will not be null-terminated.
    //
    // If the message is truncated we print it anyway. We can't distinguish
    // between truncation and other errors from (v)sn(w)printf() because WinCE
    // does not support errno, so we initialise the message buffer with an error
    // message.
    const int buffer_length = 256;
    char message_narrow[buffer_length];
    int error_len = _snprintf(message_narrow, buffer_length,
                              "Failed to print LOG message\n");
    assert(error_len > 0);
    va_list args;
    va_start(args, format);
    int narrow_len = _vsnprintf(message_narrow, buffer_length - 1, format,
                                args);
    va_end(args);
    // Null-terminate the string if it was truncated or if there was no space
    // for a terminator.
    if (-1 == narrow_len || buffer_length - 1 == narrow_len) {
      message_narrow[buffer_length - 1] = '\0';
    }
    // Convert to a wide string.
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, message_narrow, narrow_len,
                                       NULL, 0);
    if (wide_len > 0) {
      wchar_t* message_wide = new wchar_t[wide_len + 1];
      wide_len = MultiByteToWideChar(CP_UTF8, 0, message_narrow, narrow_len,
                                     message_wide, wide_len);
      if (wide_len > 0) {
        ATL::CTrace::s_trace.TraceV(file_name_, line_no_, atlTraceGeneral, 0,
                                    message_wide, NULL);
      }
      delete [] message_wide;
    }
  }
 private:
  GearsTrace& operator=(const GearsTrace& other);
  const char *const file_name_;
  const int line_no_;
};
#define LOG(args) GearsTrace(__FILE__, __LINE__) args
#define LOG16(args) ATLTRACE args
#else  // defined(WINCE)
// ATLTRACE for Win32 can take either a wide or narrow string.
#define LOG(args) ATLTRACE args
#define LOG16(args) ATLTRACE args
#endif  // defined(WINCE)
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

#else
#define DECL_SINGLE_THREAD
#define ASSERT_SINGLE_THREAD()
#endif

#ifdef DEBUG
std::string16 GetLastErrorString();
#endif

#endif  // GEARS_BASE_COMMON_COMMON_IE_H__
