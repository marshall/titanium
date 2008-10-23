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
#if defined(LINUX) && !defined(OS_MACOSX)

#include "gears/notifier/notifier_process.h"

#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#include "gears/base/common/common.h"
#include "gears/base/common/string_utils.h"
#include "gears/notifier/notifier_process_posix.h"

#if BROWSER_FF
#include "gears/base/common/paths.h"
#elif BROWSER_NONE
#include "gears/notifier/system_linux.h"
#else
#error Unsupported.
#endif

bool NotifierProcess::StartProcess(const char16 *cmd_line_options,
                                   Event *stop_event,
                                   bool async) {
  // Command line options are not supported yet.
  assert(!cmd_line_options);

#if BROWSER_FF
  std::string16 notifier_path16;
  if (!GetComponentDirectory(&notifier_path16)) {
    return false;
  }
#elif BROWSER_NONE
  std::string16 notifier_path16 = GetCurrentModulePath();
#else
#error Unsupported.
#endif
  std::string notifier_path;
  String16ToUTF8(notifier_path16.c_str(), &notifier_path);
  notifier_path.append("/notifier");

  pid_t pid = fork();
  if (pid < 0 ) {
    LOG(("fork failed with errno=%d\n", errno));
    return false;
  } else if (pid > 0) {
    int status = 0;
    waitpid(pid, &status, 0);
    if (async) {
      return true;
    }
    return NotifierPosixUtils::WaitForNotifierProcess(stop_event);
  }

  execl(notifier_path.c_str(), "notifier", static_cast<char*>(0));
  // We never go here due to execl.
  return false;
}

pid_t NotifierProcess::FindProcess() {
  return NotifierPosixUtils::FindNotifierProcess();
}

#endif  // defined(LINUX) && !defined(OS_MACOSX)
#endif  // OFFICIAL_BUILD
