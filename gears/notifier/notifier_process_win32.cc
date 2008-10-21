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

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef WIN32

#include "gears/notifier/notifier_process.h"

#include "gears/base/common/common.h"
#include "gears/base/common/event.h"
#include "gears/base/common/string16.h"
#include "gears/notifier/const_notifier.h"
#include "gears/notifier/notifier_sync_win32.h"
#include "gears/notifier/notifier_utils_win32.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// Work around the header including errors.
#include <windows.h>

static const int kNotifierStartUpSyncTimeoutMs = 5000;    // 5s

bool NotifierProcess::StartProcess(const char16 *cmd_line_options,
                                   Event *stop_event,
                                   bool async) {
  std::string16 notifier_path;
  if (!GetNotifierPath(&notifier_path)) {
    return false;
  }

  std::string16 cmd_line(L"\"");
  cmd_line += notifier_path;
  cmd_line += L"\"";
  if (cmd_line_options) {
    cmd_line += L" ";
    cmd_line += cmd_line_options;
  }

  STARTUPINFO startup_info = {0};
  startup_info.cb = sizeof(startup_info);
  PROCESS_INFORMATION process_info = {0};
  if (!::CreateProcess(NULL,
                       &cmd_line.at(0),
                       NULL,
                       NULL,
                       FALSE,
                       0,
                       NULL,
                       NULL,
                       &startup_info,
                       &process_info)) {
    return false;
  }
  ::CloseHandle(process_info.hProcess);
  ::CloseHandle(process_info.hThread);

  if (async) {
    return true;
  } else {
    NotifierSyncGate gate(kNotifierStartUpSyncGateName);
    return gate.Wait(stop_event, kNotifierStartUpSyncTimeoutMs);
  }
}

unsigned int NotifierProcess::FindProcess() {
  unsigned int process_id = 0;
  HWND window = ::FindWindow(kNotifierDummyWndClassName, NULL);
  if (window) {
    ::GetWindowThreadProcessId(window, reinterpret_cast<DWORD*>(&process_id));
  }

  return process_id;
}

#endif  // defined(WIN32)
#endif  // OFFICIAL_BUILD
