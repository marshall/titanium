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

#import "gears/base/common/detect_version_collision.h"
#import "gears/ui/safari/settings_menu.h"
#import "gears/ui/common/settings_dialog.h"

@implementation GearsSettingsMenuEnabler
+ (void)installSettingsMenu {
  LOG(("GearsSettingsMenuEnabler installSettingsMenu called"));
  // Allocate an instance of ourselves.
  GearsSettingsMenuEnabler *inst = [[GearsSettingsMenuEnabler alloc] init];
  if (!inst) {
    LOG(("GearsSettingsMenuEnabler: init failed"));
    return;
  }

  NSMenu *mainMenu = [NSApp mainMenu];
  NSMenuItem *app_menu = [mainMenu itemAtIndex:0];
  if (!app_menu) { 
    LOG(("GearsSettingsMenuEnabler: Couldn't find app menu"));
    return;
  }
  NSMenu *submenu = [app_menu submenu];
  NSMenuItem *new_menu = 
      [submenu insertItemWithTitle:@ PRODUCT_FRIENDLY_NAME_ASCII " Settings..." 
                            action:@selector(showSettingsDialog) 
                     keyEquivalent:@"" 
                           atIndex:4];
  [new_menu setTarget:inst];
  LOG(("GearsSettingsMenuEnabler: Added Gears settings menu OK"));
  LOG(("GearsSettingsMenuEnabler installSettingsMenu exited"));
}

- (void)showSettingsDialog {
  if (DetectedVersionCollision()) {
    NotifyUserOfVersionCollision();
    return;
  }
  
  if (!SettingsDialog::IsVisible()) {
    SettingsDialog::Run(NULL);
  }
}

- (BOOL)validateMenuItem:(NSMenuItem *)item {
  return !SettingsDialog::IsVisible();
}

@end
