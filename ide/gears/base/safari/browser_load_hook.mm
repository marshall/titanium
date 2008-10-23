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

#import "gears/base/common/common.h"
#import "gears/base/common/detect_version_collision.h"
#import "gears/base/common/exception_handler.h"
#import "gears/base/common/message_queue.h"
#import "gears/base/safari/browser_load_hook.h"
#import "gears/base/safari/safari_workarounds.h"
#import "gears/localserver/safari/http_handler.h"
#import "gears/localserver/safari/ui_thread.h"
#import "gears/ui/safari/settings_menu.h"
#import "gears/ui/safari/native_dialogs_osx.h"

static ExceptionManager exception_manager(true);

@implementation GearsBrowserLoadHook
+ (BOOL)installHook {
#ifdef OFFICIAL_BUILD
  // Init Breakpad.
  LOG(("Gears: Starting breakpad"));
  exception_manager.StartMonitoring();
  LOG(("Gears: Breakpad started"));
#endif
  SetUiThread();
  if (!ThreadMessageQueue::GetInstance()->InitThreadMessageQueue()) {
    LOG(("Gears: MessageQueue Init failed"));
  }

  // If there is a version collision, don't register HTTP interception hoooks.  
  if (DetectedVersionCollision()) {
    LOG(("Gears: Version collision detected"));
  } else {
    // Register HTTP intercept hook.
    if (![GearsHTTPHandler registerHandler]) {
      LOG(("GearsHTTPHandler registerHandler failed"));
      return NO;
    }
    
    // Install workarounds.
    LOG(("Gears: Installing protocol workaround"));
    ApplyProtocolWorkaround();
    LOG(("Gears: protocol workaround installed"));
  }
  
  // Register an applicationDidFinishLaunching delegeate
  GearsBrowserLoadHook *inst = [[GearsBrowserLoadHook alloc] init];
  if (!inst) {
    LOG(("GearsBrowserLoadHook init failed"));
    return NO;
  }
  LOG(("GearsBrowserLoadHook object created"));
  
  NSApplication *app = [NSApplication sharedApplication];
  if (!app) {
    LOG(("NSApplication sharedApplication returned NULL"));
    return NO;
  }

  // Make sure we don't try to add our Notification listener more than once.
  static bool notification_hook_installed = false;
  assert(!notification_hook_installed);
  if (!notification_hook_installed) {
    LOG(("Gears: adding finishlaunching notification hook"));
    [[NSNotificationCenter defaultCenter] 
        addObserver:inst 
           selector:@selector(applicationDidFinishLaunching:) 
               name:NSApplicationDidFinishLaunchingNotification
             object:app];
    LOG(("Gears: added finishlaunching notification hook"));
    
    LOG(("Gears: adding willterminate notification hook"));
    [[NSNotificationCenter defaultCenter] 
        addObserver:inst 
           selector:@selector(applicationWillTerminate:) 
               name:NSApplicationWillTerminateNotification
             object:app];
    LOG(("Gears: added willterminate notification hook"));

    LOG(("Gears: Loading Nibs"));
    PreloadNibFiles();
    LOG(("Gears: Nibs Loaded"));

    notification_hook_installed = true;
  } else {
    LOG(("Gears: notification hook already installed"));
  }
  
  // If we got here then we loaded OK
  LOG(("Loaded Gears version: " PRODUCT_VERSION_STRING_ASCII "\n" ));
  return YES;
}

// Called at the end of the application's launch, when NIBs are loaded and
// the menu bar is ready.
// IMPORTANT: Note that if a 3rd party application manually loads Gears, they
// may do so after this notification has fired. Therefore this handler should
// NOT run any code that will cripple Gears' normal operation.
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
  LOG(("Gears: finishlaunching hook called"));
  // Install Settings Menu.
  [GearsSettingsMenuEnabler installSettingsMenu];
  LOG(("Gears: finishlaunching hook exited"));
}

// Called just before the application quits.
- (void)applicationWillTerminate:(NSNotification*)aNotification {
  LOG(("Gears: applicationWillTerminate hook called"));
  UnloadNibFiles();
  LOG(("Gears: applicationWillTerminate hook exited"));
}
@end
