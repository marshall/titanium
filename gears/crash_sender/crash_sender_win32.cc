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

#include <map>
#include <windows.h>
#include <assert.h>
#include <shellapi.h>
#include <tchar.h>
#include <time.h>

#include "client/windows/sender/crash_report_sender.h"  // from breakpad/src


const wchar_t *kCrashReportUrl = L"http://www.google.com/cr/report";
const wchar_t *kCrashReportThrottlingRegKey = L"Software\\Google\\Breakpad\\Throttling";
const wchar_t *kCrashReportProductParam = L"prod";
const wchar_t *kCrashReportVersionParam = L"ver";

const int kCrashReportAttempts         = 3;
const int kCrashReportResendPeriodMs   = (1 * 60 * 60 * 1000);
const int kCrashReportsMaxPerInterval  = 5;
const int kCrashReportsIntervalSeconds = (24 * 60  * 60);

using namespace google_breakpad;


bool CanSendMinidump(const wchar_t *product_name);
void SendMinidump(const wchar_t *minidump_filename,
                  const wchar_t *product_name,
                  const wchar_t *product_version);


int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPTSTR command_line, int) {
  int num_args;
  wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &num_args);
  assert(argv);
  assert(num_args == 4);  // first argument is program path + filename

  SendMinidump(argv[1], argv[2], argv[3]);

  return 0;
}

bool CanSendMinidump(const wchar_t *product_name) {
  bool can_send = false;

  time_t current_time;
  time(&current_time);

  // For throttling, we remember when the last N minidumps were sent.

  time_t past_send_times[kCrashReportsMaxPerInterval];
  DWORD bytes = sizeof(past_send_times);
  memset(&past_send_times, 0, bytes);

  HKEY reg_key;
  DWORD create_type;
  if (ERROR_SUCCESS != RegCreateKeyExW(HKEY_CURRENT_USER,
                                       kCrashReportThrottlingRegKey, 0, NULL, 0,
                                       KEY_READ | KEY_WRITE, NULL,
                                       &reg_key, &create_type)) {
    return false;  // this should never happen, but just in case
  }

  if (ERROR_SUCCESS != RegQueryValueEx(reg_key, product_name, NULL,
                                       NULL,
                                       reinterpret_cast<BYTE*>(past_send_times),
                                       &bytes)) {
    // this product hasn't sent any crash reports yet
    can_send = true;
  } else {
    // find crash reports within the last interval
    int crashes_in_last_interval = 0;
    for (int i = 0; i < kCrashReportsMaxPerInterval; ++i) {
      if (current_time - past_send_times[i] < kCrashReportsIntervalSeconds) {
        ++crashes_in_last_interval;
      }
    }

    can_send = crashes_in_last_interval < kCrashReportsMaxPerInterval;
  }

  if (can_send) {
    memmove(&past_send_times[1],
            &past_send_times[0],
            sizeof(time_t) * (kCrashReportsMaxPerInterval - 1));
    past_send_times[0] = current_time;
  }

  RegSetValueEx(reg_key, product_name, 0, REG_BINARY,
                reinterpret_cast<BYTE*>(past_send_times),
                sizeof(past_send_times));

  return can_send;
}

void SendMinidump(const wchar_t *minidump_filename, const wchar_t *product_name,
                  const wchar_t *product_version) {
  if (CanSendMinidump(product_name)) {
    map<std::wstring, std::wstring> parameters;
    parameters[kCrashReportProductParam] = product_name;
    parameters[kCrashReportVersionParam] = std::wstring(product_version);

    std::wstring minidump_wstr(minidump_filename);

    for (int i = 0; i < kCrashReportAttempts; ++i) {
      ReportResult result = CrashReportSender::SendCrashReport(kCrashReportUrl,
                                                               parameters,
                                                               minidump_wstr,
                                                               NULL);
      if (result == RESULT_FAILED) {
        Sleep(kCrashReportResendPeriodMs);
      } else {
        // RESULT_SUCCEEDED or RESULT_REJECTED
        break;
      }
    }
  }

  DeleteFileW(minidump_filename);
}
