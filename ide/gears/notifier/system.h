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

#ifndef GEARS_NOTIFIER_SYSTEM_H__
#define GEARS_NOTIFIER_SYSTEM_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include <string>
#include "gears/base/common/string16.h"

namespace glint {
class Rectangle;
class RootUI;
}

class System {
 public:
  static std::string GetResourcePath();

  // Returns the path to store the application-specific data for the user.
  // There is no trailing path separator.
  static bool GetUserDataLocation(std::string16 *path, bool create_if_missing);

  // Returns bounds suitable for showing UI (does not include system menu,
  // taskbar or sidebars that occupy the space on a side).
  static void GetMainScreenWorkArea(glint::Rectangle *bounds);

  // If the system supports "large fonts" setting by scaling fonts behind the
  // scene (as Windows does when it scales 10pt font to look like 18pt), this
  // will report the 'font multiplication factor" for our UI to follow.
  static double GetSystemFontScaleFactor();

  // Menu Item descriptor. Not full-featured, only what we need in Notifier.
  // If title == "-" (single dash), the separator menu item is created.
  // command_id should be >= 0 since we return -1 as "nothing selected" result.
  struct MenuItem {
    std::string title;
    int command_id;
    bool enabled;
    bool checked;
  };

  // Pops up a context menu with items specified in 'menu_items' array.
  // When menu is closed (by user selecting an item or somehow else) it returns
  // with a 'command_id' of the selected item or -1 if no item was selected.
  // It is assumed that the call is made in context of processing some mouse
  // click event (so the platform-specific implementation can pick up the mouse
  // location to position the context menu properly).
  static int ShowContextMenu(const MenuItem *menu_items,
                             size_t menu_items_count,
                             glint::RootUI *root_ui);

  // Opens a UI panel with local Notifier preferences.
  static void ShowNotifierPreferences();

  // Opens an URL in the default browser.
  static bool OpenUrlInBrowser(const char16 *url);

 private:
  System() {}  // Static class.
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_SYSTEM_H__
