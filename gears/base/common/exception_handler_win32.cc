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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <assert.h>
#include <shellapi.h>
#include <time.h>

#include "gears/base/common/exception_handler.h"

#include "gears/base/common/paths.h"

#include "client/windows/handler/exception_handler.h"  // from breakpad/src


// Product-specific constants.  MODIFY THESE TO SUIT YOUR PROJECT.
#include "genfiles/product_constants.h"
const wchar_t *kCrashReportProductName = L"Google_Gears";  // [naming]
const wchar_t *kCrashReportProductVersion = PRODUCT_VERSION_STRING
#if BROWSER_FF
                                            L" (win32 firefox"
#elif BROWSER_IE
                                            L" (win32 ie"
#elif BROWSER_NPAPI
                                            L" (win32 npapi"
#elif BROWSER_NONE
                                            L" (win32"
#endif
#ifdef DEBUG
                                            L" dbg"
#endif
                                            L")";

using namespace google_breakpad;


ExceptionManager* ExceptionManager::instance_ = NULL;

ExceptionManager::ExceptionManager(bool catch_entire_process)
    : catch_entire_process_(catch_entire_process),
      exception_handler_(NULL) {
  assert(!instance_);
  instance_ = this;
}

ExceptionManager::~ExceptionManager() {
  if (exception_handler_)
    delete exception_handler_;
  assert(instance_ == this);
  instance_ = NULL;
}

static HMODULE GetModuleHandleFromAddress(void *address) {
  MEMORY_BASIC_INFORMATION mbi;
  SIZE_T result = VirtualQuery(address, &mbi, sizeof(mbi));
  return static_cast<HMODULE>(mbi.AllocationBase);
}

// Gets the handle to the currently executing module.
static HMODULE GetCurrentModuleHandle() {
  // pass a pointer to the current function
  return GetModuleHandleFromAddress(GetCurrentModuleHandle);
}

static bool IsAddressInCurrentModule(void *address) {
  return GetCurrentModuleHandle() == GetModuleHandleFromAddress(address);
}

static bool FilterCallback(void *context,
                           EXCEPTION_POINTERS *exinfo,
                           MDRawAssertionInfo *assertion) {
  ExceptionManager* this_ptr = reinterpret_cast<ExceptionManager*>(context);
  if (this_ptr->catch_entire_process())
    return true;

  if (!exinfo)
    return true;

  return IsAddressInCurrentModule(exinfo->ExceptionRecord->ExceptionAddress);
}

// Is called by Breakpad when an exception occurs and a minidump has been
// written to disk.
static bool MinidumpCallback(const wchar_t *minidump_folder,
                             const wchar_t *minidump_id,
                             void *context,
                             EXCEPTION_POINTERS *exinfo,
                             MDRawAssertionInfo *assertion,
                             bool succeeded) {
  bool handled_exception;
  ExceptionManager *this_ptr = reinterpret_cast<ExceptionManager*>(context);
  if (this_ptr->catch_entire_process()) {
    // Tell Windows we handled the exception so that the user doesn't see a
    // crash dialog.
    handled_exception = true;
  } else {
    // Returning false makes Breakpad behave as though it didn't handle the
    // exception.  This allows the browser to display its crash dialog instead
    // of abruptly terminating.  This is what some Google apps do when they're
    // inside a browser.
    handled_exception = false;
  }

  // get the full path to the minidump
  wchar_t minidump_path[MAX_PATH];
  _snwprintf(minidump_path, sizeof(minidump_path), L"%s\\%s.dmp",
             minidump_folder, minidump_id);

  // create the command line to start the crash sender process
  std::string16 install_directory;
  if (!GetInstallDirectory(&install_directory)) {
    return handled_exception;
  }

  std::string16 command_line;
  command_line += STRING16(L"\"");
  command_line += install_directory;
  command_line += STRING16(L"\\crash_sender.exe\" \"");
  command_line += minidump_path;
  command_line += L"\" \"";
  command_line += kCrashReportProductName;
  command_line += L"\" \"";
  command_line += kCrashReportProductVersion;
  command_line += L"\"";

  // execute the process
  STARTUPINFO startup_info = {0};
  startup_info.cb = sizeof(startup_info);
  PROCESS_INFORMATION process_info = {0};
  CreateProcessW(NULL,  // application name (NULL to get from command line)
                 const_cast<char16 *>(command_line.c_str()),
                 NULL,  // process attributes (NULL means process handle not
                        // inheritable)
                 NULL,  // thread attributes (NULL means thread handle not
                        // inheritable)
                 FALSE, // inherit handles
                 0,     // creation flags
                 NULL,  // environment block (NULL to use parent's)
                 NULL,  // starting block (NULL to use parent's)
                 &startup_info,
                 &process_info);
  CloseHandle(process_info.hProcess);
  CloseHandle(process_info.hThread);
    
  return handled_exception;
}

void ExceptionManager::StartMonitoring() {
  if (exception_handler_) { return; }  // don't init more than once

  wchar_t temp_path[MAX_PATH];
  if (!GetTempPathW(MAX_PATH, temp_path)) { return; }

  exception_handler_ = new google_breakpad::ExceptionHandler(temp_path,
                                                             FilterCallback,
                                                             MinidumpCallback,
                                                             this, true);
}

void ExceptionManager::AddMemoryRange(void *address, int length) {
  assert(exception_handler_);
  exception_handler_->AddMemoryRange(address, length);
}

void ExceptionManager::ClearMemoryRanges() {
  assert(exception_handler_);
  exception_handler_->ClearMemoryRanges();
}

// static
bool ExceptionManager::ReportAndContinue() {
  if (!instance_ || !instance_->exception_handler_) {
    return false;
  }

  // Pass parameters to WriteMinidump so the reported call stack ends here,
  // instead of including all frames down to where the dump file gets written.
  //
  // This requires a valid EXCEPTION_POINTERS struct.  GetExceptionInformation()
  // can generate one for us.  But that function can only be used in an __except
  // filter statement.  And the value returned only appears to be valid for the
  // lifetime of the filter statement.  Hence the comma-separated statement
  // below, which is actually common practice.
  bool retval;
  google_breakpad::ExceptionHandler *h = instance_->exception_handler_;

  __try {
    int *null_pointer = NULL;
    *null_pointer = 1;
  } __except (retval = h->WriteMinidump(GetExceptionInformation(), NULL),
              EXCEPTION_EXECUTE_HANDLER) {
    // EXCEPTION_EXECUTE_HANDLER causes execution to continue here.
    // We have nothing more to do, so just continue normally.
  }
  return retval;
}
