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

// Utility functions for process management common for Linux and OSX.

#ifndef GEARS_NOTIFIER_NOTIFIER_PROCESS_POSIX_H__
#define GEARS_NOTIFIER_NOTIFIER_PROCESS_POSIX_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#include <string>

class Event;

class NotifierPosixUtils {
 public:
  // Called by Notifier process when it starts. If returns 'true', the calling
  // process is successfully registered as a single notifier process for the
  // current user (meaning it obtained a lock for the .lock file).
  // If this function returns 'false', the calling process should exit since
  // most likely the other process already is registered.
  static bool CreateLockFile();

  // Releases the single-instance status (by releasing the lock file).
  // After this, another process can immediately assume "the Notifier" status.
  // If current process did not have the lock file locked for itself, nothing
  // happens (so we don't try to delete someone else's lock file).
  static void DeleteLockFile();

  // Called by Notifier Process that has established IPC receiving port.
  // It writes the PID of the calling process into the lock file so other
  // processes can obtain it and start IPC communications. This is the final
  // step of Notifier initialization.
  static bool RegisterIPC();

  // Revokes the IPC channel by removing PID from the lock file. The sending
  // process will not be able to obtain the PID and thus will not be able
  // to send IPC. It will probably start another instance of the Notifier.
  static bool UnregisterIPC();

  // Called to find out if the Notifier process is running and if it is running,
  // returns its ID. It reads the ID from a lock file created by Notifier
  // process. If it returns 0, there is no Notifier yet running/ready.
  static pid_t FindNotifierProcess();

  // Waits for the notifier process to start and get to "idle".
  static bool WaitForNotifierProcess(Event *stop_event);

 private:
  NotifierPosixUtils() {  // static class - private ctor
  }
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_NOTIFIER_PROCESS_POSIX_H__
