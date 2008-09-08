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

#ifdef OS_MACOSX
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
int main(int argc, char *argv[]) {
  return 0;
}
#else
#import "gears/notifier/notifier.h"
#import <Cocoa/Cocoa.h>
#ifdef OFFICIAL_BUILD
#import "gears/base/common/exception_handler.h"
#endif  // OFFICIAL_BUILD
#import "gears/notifier/notifier_process_posix.h"

class MacNotifier : public Notifier {
 public:
  MacNotifier();
  virtual bool Initialize();
  virtual int Run();
  virtual void RequestQuit();
  virtual void Terminate();

 protected:
  virtual bool RegisterProcess();
  virtual bool UnregisterProcess();

 private:
  DISALLOW_EVIL_CONSTRUCTORS(MacNotifier);
};

MacNotifier::MacNotifier() {
}

bool MacNotifier::RegisterProcess() {
  return NotifierPosixUtils::RegisterIPC();
}

bool MacNotifier::UnregisterProcess() {
  NotifierPosixUtils::UnregisterIPC();
  return true;
}

bool MacNotifier::Initialize() {
  // Single instance check - *nix way, with a lock file.
  if (!NotifierPosixUtils::CreateLockFile()) {
    return false;
  }

  // This will setup IPC and call MacNotifier::RegisterProcess
  return Notifier::Initialize();
}

void MacNotifier::Terminate() {
  NotifierPosixUtils::DeleteLockFile();
  Notifier::Terminate();
}

int MacNotifier::Run() {
  [NSApp run];
  return 0;
}

void MacNotifier::RequestQuit() {
  [NSApp terminate:NSApp];
}

extern "C" {

int main(int argc, const char** argv) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  LOG(("Gears Notifier started.\n"));

#ifdef OFFICIAL_BUILD
  static ExceptionManager exception_manager(true);
  exception_manager.StartMonitoring();
  LOG(("Breakpad started.\n"));
#endif  // OFFICIAL_BUILD

  [NSApplication sharedApplication];

  MacNotifier notifier;
  if (notifier.Initialize()) {
    notifier.Run();
    notifier.Terminate();
  }

  LOG(("Gears Notifier terminated.\n"));
  [pool release];
  return 0;
}

}  // extern "C"

#endif  // OFFICIAL_BUILD
#endif  // OS_MACOSX

