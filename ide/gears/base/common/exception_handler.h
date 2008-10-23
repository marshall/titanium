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
//
// Wrapper class for using the Breakpad crash reporting system.
//
// Be sure to modify kCrashReportProduct* in the .cc file to suit your project.
//
// To use this wrapper, you must also add this to your include path:
//     /breakpad/src
// And link with these files:
//     /breakpad/src/client/windows/handler/exception_handler.cc
//     /breakpad/src/client/windows/sender/crash_report_sender.cc
//     /breakpad/src/common/windows/guid_string.cc
//     /breakpad/src/common/windows/http_upload.cc

#ifndef GEARS_BASE_COMMON_EXCEPTION_HANDLER_H__
#define GEARS_BASE_COMMON_EXCEPTION_HANDLER_H__

#if (defined(WIN32) && !defined(WINCE)) || \
    (defined(OS_MACOSX) && (defined(BROWSER_WEBKIT) || defined(BROWSER_NONE)))

namespace google_breakpad {
  class ExceptionHandler;
};

// Sample usage:
//   int main(void) {
//     static ExceptionManager exception_manager(false);
//     exception_manager.StartMonitoring();
//     ...
//   }
class ExceptionManager {
 public:
  // If catch_entire_process is true, then all minidumps are captured.
  // Otherwise, only crashes in this module are captured.
  // Use the latter when running inside IE or Firefox.
  // StartMonitoring needs to be called before any minidumps are captured.
  ExceptionManager(bool catch_entire_process);
  ~ExceptionManager();

  // Starts monitoring for crashes. When a crash occurs a minidump will
  // automatically be captured and sent.
  void StartMonitoring();

  // Specifies additional memory ranges for inclusion in minidumps.
  // Requires a Breakpad instance, so must be called after StartMonitoring().
  void AddMemoryRange(void *address, int length);
  void ClearMemoryRanges();

  // Manually captures and sends a minidump, returns true on success.
  // If StartMonitoring has not been called, no minidump is sent and
  // false is returned.
  static bool ReportAndContinue();

// None of this is used on OS X.
#if (defined(WIN32) && !defined(WINCE))
  // TODO(michaeln): Cleanup. The following should not be called
  // directly, ideally these should be private methods.
  bool catch_entire_process() { return catch_entire_process_; }

 private:
  static ExceptionManager *instance_;

  bool catch_entire_process_;
  google_breakpad::ExceptionHandler *exception_handler_;
#endif
};
#else

// Stub to allow compilation on OS'es for which we do not yet implement crash
// reporting.  (Only the static methods need to be listed here.)
class ExceptionManager {
 public:
  static bool ReportAndContinue() { return false; }
};

#endif  // (WIN32 && !WINCE) || (OS_MACOSX && (BROWSER_WEBKIT || BROWSER_NONE))
#endif  // GEARS_BASE_COMMON_EXCEPTION_HANDLER_H__
