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
#if defined(LINUX) || defined(ANDROID) || defined(OS_MACOSX)

#include "gears/notifier/notifier_process_posix.h"

#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

#include <string>

#include "gears/base/common/common.h"
#include "gears/base/common/common_osx.h"
#include "gears/base/common/event.h"
#include "gears/base/common/file.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"

// Constants relating to waiting for a new notifier process to start.
static int kStartTimeout = 10000;  // 10 seconds
static int kWaitPeriodMs = 200;    // wait for 200ms at a time

// Process-wide global file descriptor for a lock file.
static int single_instance_locking_fd = -1;

static std::string GetSingleInstanceLockFilePath() {
  static std::string s_lock_file_path;
  if (s_lock_file_path.empty()) {
#ifdef OS_MACOSX
    std::string lock_file_path = TempDirectoryForCurrentUser();
#else
    std::string16 lock_file_path16;
    std::string lock_file_path;
    if (!File::GetBaseTemporaryDirectory(&lock_file_path16) ||
        !String16ToUTF8(lock_file_path16, &lock_file_path)) {
      return std::string();
    }
#endif
    lock_file_path += "/" PRODUCT_SHORT_NAME_ASCII "_notifier_";
    lock_file_path += IntegerToString(getuid());
    lock_file_path += ".lock";
    s_lock_file_path = lock_file_path;
  }
  return s_lock_file_path;
}

pid_t NotifierPosixUtils::FindNotifierProcess() {
  pid_t pid = 0;

  std::string lock_file_path = GetSingleInstanceLockFilePath();
  if (lock_file_path.empty()) {
    return 0;
  }
  int lock_file_fd = open(lock_file_path.c_str(), O_RDONLY);
  if (lock_file_fd > 0) {
    char buf[16] = { 0 };
    int num = read(lock_file_fd, buf, sizeof(buf) - 1);
    if (0 < num && num < static_cast<int>(sizeof(buf) - 1)) {
      pid_t id = strtol(buf, NULL, 10);
      if (id > 0 && kill(id, 0) == 0) {
        pid = id;
      }
    }
    close(lock_file_fd);
  }

  return pid;
}

// Check if the instance has already been running. Create and lock the 'lock
// file'. If either create or lock didn't work - return false.
bool NotifierPosixUtils::CreateLockFile() {
  std::string lock_file_path = GetSingleInstanceLockFilePath();

  // TODO(jianli): Add file locking to File interface and use it instead.
  // We could use O_EXLOCK during open() to obtain the lock atomically.
  // However it only works on BSD and OSX, not on Linux. So we use flock().
  single_instance_locking_fd = open(
      lock_file_path.c_str(),
      O_RDWR | O_CREAT | O_NONBLOCK | O_TRUNC,
      S_IRUSR | S_IWUSR);
  if (single_instance_locking_fd < 0) {
    LOG(("opening lock file %s failed with errno=%d\n",
         lock_file_path.c_str(), errno));
    return false;
  }

  if (0 != flock(single_instance_locking_fd, LOCK_EX | LOCK_NB)) {
    single_instance_locking_fd = -1;
    LOG(("Notifier is already started.\n"));
    return false;
  }

  return true;
}

void NotifierPosixUtils::DeleteLockFile() {
  if (single_instance_locking_fd < 0)
    return;

  close(single_instance_locking_fd);
  std::string lock_file_path = GetSingleInstanceLockFilePath();
  unlink(lock_file_path.c_str());
  single_instance_locking_fd = -1;
}

bool NotifierPosixUtils::RegisterIPC() {
  if (single_instance_locking_fd < 0) {
    return true;
  }

  if (ftruncate(single_instance_locking_fd, 0) < 0) {
    LOG(("truncating lock file failed with errno=%d\n", errno));
    return false;
  }
  std::string pid_str = IntegerToString(static_cast<int32>(getpid()));
  if (write(single_instance_locking_fd,
            &pid_str[0],
            pid_str.length() + 1) < 0) {
    LOG(("writing lock file failed with errno=%d\n", errno));
    return false;
  }

  return true;
}

bool NotifierPosixUtils::UnregisterIPC() {
  if (single_instance_locking_fd < 0) {
    return true;
  }

  if (ftruncate(single_instance_locking_fd, 0) < 0) {
    LOG(("truncating lock file failed with errno=%d\n", errno));
    return false;
  }
  return true;
}

bool NotifierPosixUtils::WaitForNotifierProcess(Event *stop_event) {
  for (int loops = kStartTimeout / kWaitPeriodMs;
       loops > 0 && !stop_event->WaitWithTimeout(kWaitPeriodMs);
       --loops) {
    if (NotifierPosixUtils::FindNotifierProcess()) {
      return true;
    }
  }
  return false;
}

#endif  // defined(LINUX) || defined(ANDROID) || defined(OS_MACOSX)
#endif  // OFFICIAL_BUILD
