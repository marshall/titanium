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
#ifdef OS_MACOSX

#import "gears/notifier/notifier_process.h"

#import <Cocoa/Cocoa.h>

#import "gears/base/common/event.h"
#import "gears/base/common/common_osx.h"
#import "gears/notifier/notifier_process_posix.h"

static int kStartTimeout = 10000;  // 10 seconds
static int kWaitPeriod = 200;      // wait for 200ms at a time

bool NotifierProcess::StartProcess(const char16 *cmd_line_options,
                                   Event *stop_event,
                                   bool async) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  if (NotifierPosixUtils::FindNotifierProcess() != 0)
    return true;

  NSWorkspace *ws = [NSWorkspace sharedWorkspace];
#ifdef BROWSER_NONE
  // TODO(dimich): Add logic finding the executable in 'hot upgrade' scenario.
  assert(false && "Not implemented.");
  NSString *path = @"no path";
#else
  std::string resource_path = ModuleResourcesDirectory();
  NSString *path = [NSString stringWithUTF8String:resource_path.c_str()];
  path = [path stringByAppendingPathComponent:@"Notifier.app"];
#endif
  BOOL launched = [ws launchApplication:path];

  // If sync, lets wait, periodically asking for PID. Have a timeout.
  // Launching of the app is a long process with potentially several
  // 'control points' - creating a process, loading code, running first method,
  // starting message loop, getting to idle first time. While documentation for
  // NSWorkspace implies 'launchApplication' is synchronous, it's not clear
  // what they mean by that.
  // We need precise control over when we can start doing IPC which requires
  // some initialization to be done. We also need to know when we should
  // stop sending IPC messages to one process and start sending them to
  // another in case of 'hot upgrade'. So we rely on an explicit mechanism
  // with lock file and PID written into it. FindNotifierProcess() attempts to
  // read a valid PID from the lock file.
  if (!async) {
    int loops = kStartTimeout / kWaitPeriod;
    while (!stop_event->WaitWithTimeout(kWaitPeriod) && (--loops > 0)) {
      if (NotifierPosixUtils::FindNotifierProcess()) {
        launched = true;
        break;
      }
    }
  }
  [pool release];
  return launched ? true : false;  // because BOOL is not bool.
}

pid_t NotifierProcess::FindProcess() {
  return NotifierPosixUtils::FindNotifierProcess();
}

#endif  // OS_MACOSX
#endif  // OFFICIAL_BUILD
