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
#import "gears/notifier/system.h"

#import <Cocoa/Cocoa.h>
#import <string>

#import "gears/base/common/common.h"
#import "gears/base/common/file.h"
#import "gears/base/common/string_utils.h"
#import "gears/notifier/notifier_pref_common.h"
#import "third_party/glint/include/platform.h"
#import "third_party/glint/include/rectangle.h"
#import "third_party/glint/include/root_ui.h"

std::string System::GetResourcePath() {
  return [[[NSBundle mainBundle] resourcePath] fileSystemRepresentation];
}

bool System::GetUserDataLocation(std::string16 *path, bool create_if_missing) {
  assert(path);

  NSArray *paths = NSSearchPathForDirectoriesInDomains(
                      NSApplicationSupportDirectory, NSUserDomainMask, YES);
  if ([paths count] < 1) {
    return false;
  }
  assert([paths count] == 1);

  NSString *path_ns = [paths objectAtIndex:0];
  path_ns = [path_ns stringByAppendingPathComponent:@PRODUCT_SHORT_NAME_ASCII];

  std::string narrow_path = [path_ns fileSystemRepresentation];
  if (!UTF8ToString16(narrow_path.c_str(), path)) {
    return false;
  }

  if (create_if_missing && !File::DirectoryExists(path->c_str())) {
    if (!File::RecursivelyCreateDir(path->c_str())) {
      return false;
    }
  }

  return true;
}

void System::GetMainScreenWorkArea(glint::Rectangle *bounds) {
  assert(bounds);
  // Reset - makes rectangle 'empty'.
  bounds->Reset();

  NSArray *screens = [NSScreen screens];
  if ([screens count] == 0)
    return;
  // Get the screen with main menu.Should work even if screens is empty
  NSScreen *screen = [screens objectAtIndex:0];

  NSRect full_bounds = [screen frame];
  NSRect work_bounds = [screen visibleFrame];

  // We need to make an adjustment here. If we returned the work area as it is,
  // using visibleFrame, the caller will think the dock is on the top and the
  // menubar is at the bottom, because the caller uses top-left origin. So we
  // adjust the origin to reverse the dock and menubar constraints.
  int menubar_size = NSMaxY(full_bounds) - NSMaxY(work_bounds);
#ifdef DEBUG
  // only used in the assert below
  int dock_size = NSMinY(work_bounds) - NSMinY(full_bounds);
#endif

  work_bounds.origin.y = NSMinY(full_bounds) + menubar_size;

  // Both sizes have been switched; compare the asserts to the statements above.

  assert(NSMinY(work_bounds) - NSMinY(full_bounds) == menubar_size);
  assert(NSMaxY(full_bounds) - NSMaxY(work_bounds) == dock_size);

  bounds->set_left(static_cast<int>(NSMinX(work_bounds)));
  bounds->set_top(static_cast<int>(NSMinY(work_bounds)));
  bounds->set_right(static_cast<int>(NSMaxX(work_bounds)));
  bounds->set_bottom(static_cast<int>(NSMaxY(work_bounds)));
}

double System::GetSystemFontScaleFactor() {
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  id val = [defaults objectForKey:@"AppleDisplayScaleFactor"];
  if (val == nil) {
    return 1.0;
  }

  return [val doubleValue];
}

// Helper object to implement context menu tracking.
@interface MenuTarget : NSObject {
  int selectedTag_;
}

- (id)init;
- (void)menuItemAction:(id)sender;
- (int)selectedTag;

@end

@implementation MenuTarget

- (id)init {
  self = [super init];
  if (self) {
    selectedTag_ = -1;
  }
  return self;
}

- (void)menuItemAction:(id)sender {
  selectedTag_ = [sender tag];
}

- (int)selectedTag {
  return selectedTag_;
}

@end

int System::ShowContextMenu(const MenuItem *menu_items,
                            size_t menu_items_count,
                            glint::RootUI *root_ui) {
  assert(menu_items && root_ui);
  if (menu_items_count == 0)
    return -1;
  MenuTarget *target = [[[MenuTarget alloc] init] autorelease];
  NSMenu *theMenu =
      [[[NSMenu alloc] initWithTitle:@"Notifications Settings"] autorelease];

  for (size_t i = 0; i < menu_items_count; ++i) {
    NSString *title =
        [NSString stringWithUTF8String:(menu_items[i].title.c_str())];

    // Check if the item should be a separator.
    BOOL isSeparator = [title isEqualToString:@"-"];

    NSMenuItem *newItem = (isSeparator ?
        [NSMenuItem separatorItem] :
        [[[NSMenuItem alloc] initWithTitle:title
                                    action:@selector(menuItemAction:)
                             keyEquivalent:@""] autorelease]);

    [newItem setTag:menu_items[i].command_id];
    [newItem setTarget:target];

    BOOL isEnabled = menu_items[i].enabled ? YES : NO;
    [newItem setEnabled:isEnabled];

    int checkedState = menu_items[i].checked ? NSOnState : NSOffState;
    [newItem setState:checkedState];
    [theMenu addItem:newItem];
  }

  NSWindow *window = static_cast<NSWindow*>(
      glint::platform()->GetWindowNativeHandle(root_ui->GetPlatformWindow()));
  assert(window);

  [NSMenu popUpContextMenu:theMenu
                 withEvent:[window currentEvent]
                   forView:[window contentView]];

  return [target selectedTag];
}


void System::ShowNotifierPreferences() {
  NSString *script_source =
      @"tell application \"System Preferences\" \n"
      @" activate \n"
      @" set current pane to pane \"%@\" \n"
      @"end tell\n";

  NSString *script_source_formatted =
    [NSString stringWithFormat:script_source, (NSString *)kGearsNotifierPrefPaneID];

  NSAppleScript *script =
      [[[NSAppleScript alloc] initWithSource:script_source_formatted] autorelease];

  NSDictionary *dictionary = nil;
  NSAppleEventDescriptor *result = [script executeAndReturnError:&dictionary];
  if (result == nil) {
    NSLog(@"ShowNotifierPreferences error: %@", [dictionary description]);
  }
}

bool System::OpenUrlInBrowser(const char16 *wide_url) {
  assert(wide_url && *wide_url);
  
  // We always open the url in Safari browser.
  
  std::string url;
  if (!String16ToUTF8(wide_url, &url)) {
    return false;
  }

  NSString *url_string = [NSString stringWithUTF8String:url.c_str()];
  if (!url_string) {
    return false;
  }
  NSURL *ns_url = [NSURL URLWithString:url_string];
  if (!ns_url) {
    return false;
  }
  return [[NSWorkspace sharedWorkspace] openURL:ns_url];
}
#endif  // OFFICIAL_BUILD
