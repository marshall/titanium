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

#if defined(LINUX) && !defined(OS_MACOSX)
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
int main(int argc, char *argv[]) {
  return -1;
}
#else
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include "gears/notifier/notifier.h"
#include "gears/notifier/notifier_process_posix.h"
#include "third_party/glint/include/platform.h"

class LinuxNotifier : public Notifier {
 public:
  LinuxNotifier(bool debug);
  virtual bool Initialize();
  virtual int Run();
  virtual void Terminate();
  virtual void RequestQuit();

 private:
  bool StartAsDaemon();
  virtual bool RegisterProcess();
  virtual bool UnregisterProcess();

  bool debug_;

  DISALLOW_EVIL_CONSTRUCTORS(LinuxNotifier);
};

LinuxNotifier::LinuxNotifier(bool debug) : debug_(debug) {
}

bool LinuxNotifier::RegisterProcess() {
  NotifierPosixUtils::RegisterIPC();
  return true;
}

bool LinuxNotifier::UnregisterProcess() {
  NotifierPosixUtils::UnregisterIPC();
  return true;
}

// Start the instance as a background daemon process.
// TODO(dimich): Consider replacing this code with daemon() API, which exists
// on Linux.
bool LinuxNotifier::StartAsDaemon() {
  // Clear file creation mask.
  umask(0);

  // Get maximum number of file descriptors.
  rlimit rl = { 0 };
  if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
    LOG(("getrlimit failed with errno=%d\n", errno));
    return false;
  }

  // Fork off the parent process in order to become a session leader to lose
  // controlling TTY.
  pid_t pid = fork();
  if (pid < 0 ) {
    LOG(("fork failed with errno=%d\n", errno));
    return false;
  } else if (pid > 0) {
    exit(EXIT_SUCCESS);   // parent process
  }

  // Create a new sid for the child process.
  if (setsid() < 0) {
    LOG(("setsid failed with errno=%d\n", errno));
    return false;
  }

  // Ensure future open won't allocate controlling TTYs.
  struct sigaction sig_action = { 0 };
  sig_action.sa_handler = SIG_IGN;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = 0;
  if (sigaction(SIGHUP, &sig_action, NULL) < 0) {
    LOG(("sigaction failed with errno=%d\n", errno));
  }
  pid = fork();
  if (pid < 0) {
    LOG(("fork failed with errno=%d\n", errno));
    return false;
  } else if (pid > 0) {
    exit(EXIT_SUCCESS);   // parent process
  }

  // Change the current working directory to the root so we won't prevent file
  // systems from being unmounted.
  if (chdir("/") < 0) {
    LOG(("chdir failed with errno=%d\n", errno));
    return false;
  }

  // Close all open file descriptors.
  if (rl.rlim_max == RLIM_INFINITY) {
    rl.rlim_max = 1024;
  }
  for (int i = 0; i < static_cast<int>(rl.rlim_max); ++i) {
    close(i);
  }

  // Attach standard file descriptors to /dev/null.
  open("/dev/null", O_RDWR);
  dup(0);
  dup(0);

  return true;
}

bool LinuxNotifier::Initialize() {
  // Check if the Notifier process is already running.
  if (NotifierPosixUtils::FindNotifierProcess() != 0) {
    return false;
  }

  // Start as a daemon process. Do this only if debug option is not present.
  if (!debug_) {
    if (!StartAsDaemon()) {
      return false;
    }
  }

  // Single instance check - *nix way, with a lock file.
  if (!NotifierPosixUtils::CreateLockFile()) {
    return false;
  }

  // This will setup IPC and call LinuxNotifier::RegisterProcess
  return Notifier::Initialize();
}

void LinuxNotifier::Terminate() {
  NotifierPosixUtils::DeleteLockFile();
  Notifier::Terminate();
}

void LinuxNotifier::RequestQuit() {
  running_ = false;
  // TODO(levin): Should this be part of glint::platform,
  // since it relies on knowing what loop is running there?
  gtk_main_quit();
}

int LinuxNotifier::Run() {
  running_ = true;
  while (running_) {
    glint::platform()->RunMessageLoop();
  }
  return 0;
}

int main(int argc, char *argv[]) {
  LOG(("Gears Notifier started.\n"));

  bool debug = false;
#ifdef DEBUG
  if (argc > 1 && strcmp(argv[1], "-debug") == 0) {
    debug = true;
  }
#endif  // DEBUG

  LinuxNotifier notifier(debug);

  int retval = -1;
  if (notifier.Initialize()) {
    retval = notifier.Run();
    notifier.Terminate();
  }

  LOG(("Gears Notifier terminated.\n"));
  return retval;
}

#endif  // OFFICIAL_BUILD
#endif  // defined(LINUX) && !defined(OS_MACOSX)
