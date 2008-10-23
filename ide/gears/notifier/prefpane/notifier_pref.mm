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

#import "notifier_pref.h"

#import "gears/base/common/common_sf.h"  // for IsLeopardOrGreater
#import "gears/notifier/notifier_pref_common.h"
#import "third_party/growl/GrowlApplicationBridge.h"

@implementation ComGoogleGearsNotifierPref

// Called when the user clicks on the "Gears Notifier" icon and our prefpane
// is about to pop into view. There's another method initWithBundle which is
// called when the prefpane bundle is loaded but before the nib file is loaded.
- (void)mainViewDidLoad {
  // This changes the size of the view to work around changes to prefpane size
  // in Leopard.  System Preferences doesn't resize the view, it just sticks
  // it in, so we have to resize it so the struts and springs of the nib get
  // evaluated.
  SInt32 os_version = 0;
  if (Gestalt(gestaltSystemVersion, &os_version) != noErr) {
    // Do nothing if an error occured.
  }
  NSLog(@"Gestalt: %x", os_version);
  if (os_version <= 0x1050) {
    // Getting the window from the nib crashes, so we get the window elsewhere.
    NSSize currentSize = [[growlCheckbox superview] frame].size;
    // Tiger's pane width.
    currentSize.width = 591.0;
    [[growlCheckbox superview] setFrameSize:currentSize];
  }

  CFPreferencesAppSynchronize(kGearsNotifierAppID);

  bool state = ShouldUseGrowlPref();
  [growlCheckbox setState:state];

  BOOL isGrowlInstalled = [GrowlApplicationBridge isGrowlInstalled];
  [growlCheckbox setEnabled:isGrowlInstalled];

  if (isGrowlInstalled) {
    [growlCheckbox setToolTip:nil];
  } else {
    // TODO(chimene): Internationalize this with Notifier's localization code
    // Don't use strings files as it could crash prefpane if it's updated
    // while the bundle is loaded. (says jgm)
    NSString *localizedTooltip = NSLocalizedStringFromTableInBundle(
        @"GrowlNotInstalled",
        nil,
        [NSBundle bundleForClass:[self class]],
        "Tooltip for disabled Growl checkbox when Growl isn't installed");

    [growlCheckbox setToolTip:localizedTooltip];
  }
}

- (IBAction)growlCheckboxClicked:(id)sender {
  if ([sender state]) {
    CFPreferencesSetAppValue(kShouldUseGrowlPrefString, kCFBooleanTrue,
                             kGearsNotifierAppID);
  } else {
    CFPreferencesSetAppValue(kShouldUseGrowlPrefString, kCFBooleanFalse,
                             kGearsNotifierAppID);
  }
  [self notifyPrefChanged];
}

- (IBAction)licenseButtonClicked:(id)sender {
  // Is this sufficient for BSD requirements, or do we need a local copy?
  NSURL *licenseURL =
      [NSURL URLWithString:@"http://growl.info/documentation/developer/bsd-license.txt"];
  [[NSWorkspace sharedWorkspace] openURL:licenseURL];
}

- (void)notifyPrefChanged {
  CFPreferencesAppSynchronize(kGearsNotifierAppID);

  CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
  CFNotificationCenterPostNotification(center,
                                       kGearsNotifierPrefChangedNotification,
                                       kGearsNotifierAppID, NULL, TRUE);
}
@end
