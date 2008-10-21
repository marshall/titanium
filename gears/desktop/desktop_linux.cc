// Copyright 2007, Google Inc.
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
//
// Linux-specific implementation of desktop shortcut creation.

// TODO(cprince): remove platform-specific #ifdef guards when OS-specific
// sources (e.g. LINUX_CPPSRCS) are implemented
#if defined(LINUX) && !defined(OS_MACOSX)
#include <errno.h>

#include "gears/desktop/desktop.h"

#include "gears/base/common/basictypes.h"
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "gears/desktop/desktop.h"
#include "gears/localserver/common/http_constants.h"


static bool GetIconPath(const SecurityOrigin &origin,
                        const std::string16 link_name,
                        std::string16 *icon_path,
                        std::string16 *error) {
  if (!GetDataDirectory(origin, icon_path)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  AppendDataName(STRING16(L"icons"), kDataSuffixForDesktop, icon_path);
  *icon_path += kPathSeparator;
  *icon_path += link_name;
  *icon_path += STRING16(L"_linux");
  *icon_path += STRING16(L".png");

  return true;
}

static bool WriteIconFile(const Desktop::ShortcutInfo &shortcut,
                          const std::string16 &icon_path,
                          std::string16 *error) {
  const Desktop::IconData *chosen_icon = NULL;

  // Try to pick the best icon size of the available choices.
  if (!shortcut.icon48x48.png_data.empty()) { // 48 is default size for gnome
    chosen_icon = &shortcut.icon48x48;
  } else if (!shortcut.icon128x128.png_data.empty()) { // better to be too big
    chosen_icon = &shortcut.icon128x128;
  } else if (!shortcut.icon32x32.png_data.empty()) {
    chosen_icon = &shortcut.icon32x32;
  } else if (!shortcut.icon16x16.png_data.empty()) {
    chosen_icon = &shortcut.icon16x16;
  } else {
    // Caller should have verified that we had at least one icon
    assert(false);
  }

  // (over)write the icon file
  File::CreateNewFile(icon_path.c_str());
  if (!File::WriteVectorToFile(icon_path.c_str(), &chosen_icon->png_data)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  return true;
}

// We only allow writing the shortcut if no other file exists in that spot.
static bool CanWriteShortcut(const Desktop::ShortcutInfo &shortcut) {
  std::string home_path = getenv("HOME");
  std::string16 shortcut_path;
  if (!UTF8ToString16(home_path.c_str(), home_path.length(), &shortcut_path)) {
    return false;
  }

  shortcut_path += STRING16(L"/Desktop/");
  shortcut_path += shortcut.app_name;
  shortcut_path += STRING16(L".desktop");

  return !File::Exists(shortcut_path.c_str()) &&
    !File::DirectoryExists(shortcut_path.c_str());
}

bool Desktop::CreateShortcutPlatformImpl(
                       const SecurityOrigin &origin,
                       const Desktop::ShortcutInfo &shortcut,
                       uint32 locations,
                       std::string16 *error) {
  // Return immediately if shortcut already exists.
  if (!CanWriteShortcut(shortcut)) {
    return true;
  }

  // Note: We assume that link_name has already been validated by the caller to
  // have only legal filename characters and that launch_url has already been
  // resolved to an absolute URL.

  std::string16 icon_path;
  if (!GetIconPath(origin, shortcut.app_name, &icon_path, error)) {
    return false;
  }

  if (!WriteIconFile(shortcut, icon_path, error)) {
    return false;
  }

  // Execute 'which' to determine where the firefox launch script is.
  FILE *binary_location = popen("which firefox", "r");
  if (binary_location == NULL ) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  std::string browser_path_utf8("\"");
  int input = fgetc(binary_location);
  while (input != EOF) {
    // Ignore the carrage return.
    if (input != '\n') {
      browser_path_utf8.push_back(input);
    }
    input = fgetc(binary_location);
  }
  browser_path_utf8.push_back('\"');

  pclose(binary_location);

  std::string16 browser_path;
  if (!UTF8ToString16(browser_path_utf8.c_str(), browser_path_utf8.length(),
                      &browser_path)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  std::string link_name_utf8;
  if (!String16ToUTF8(shortcut.app_name.c_str(), shortcut.app_name.length(),
                      &link_name_utf8)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  std::string shortcut_path(getenv("HOME"));
  shortcut_path += "/Desktop/";
  shortcut_path += link_name_utf8;
  shortcut_path += ".desktop";

  FILE *f = fopen(shortcut_path.c_str(), "w");
  if (f == NULL) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  // See http://standards.freedesktop.org/desktop-entry-spec/latest/ for
  // format documentation.
  std::string16 shortcut_contents(
      STRING16(L"[Desktop Entry]\nType=Application\nVersion=1.0"));
  shortcut_contents += STRING16(L"\nName=");
  shortcut_contents += shortcut.app_name;
  shortcut_contents += STRING16(L"\nIcon=");
  shortcut_contents += icon_path;
  shortcut_contents += STRING16(L"\nExec=");
  shortcut_contents += browser_path;
  shortcut_contents += STRING16(L" '");
  shortcut_contents += shortcut.app_url;
  shortcut_contents += STRING16(L"'\n");

  std::string shortcut_contents_utf8;
  if (String16ToUTF8(shortcut_contents.c_str(), shortcut_contents.size(),
                     &shortcut_contents_utf8)) {
    fputs(shortcut_contents_utf8.c_str(), f);
  }

  fclose(f);
  return true;
}
 
#endif  // #if defined(LINUX) && !defined(OS_MACOSX)
