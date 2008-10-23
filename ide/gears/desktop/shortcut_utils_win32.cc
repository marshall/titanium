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
//
// Shared utility for creating desktop shortcut files.

#ifdef WIN32

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>

#include "gears/desktop/desktop.h"
#include "gears/desktop/shortcut_utils_win32.h"

#include "gears/base/common/basictypes.h"
#include "gears/base/common/process_utils_win32.h"
#ifdef WINCE
#include "gears/base/common/file.h"
#endif
#include "gears/base/common/png_utils.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#ifdef WINCE
#include "gears/desktop/dll_data_wince.h"  // For kDllIconId
#endif
#include "gears/ui/ie/string_table.h"
#include "third_party/scoped_ptr/scoped_ptr.h"
#if BROWSER_CHROME
#include "gears/base/chrome/module_cr.h"
#endif

static bool CreateShellLink(const char16 *link_path,
                            const char16 *icon_path,
                            const char16 *object_path,
                            const char16 *arguments) {
#ifdef WINCE
  // IShellLink does not exist on WinCE and the alternative (SHCreateShortcut)
  // does not allow us to set a custom icon. So we write the shortcut by hand.
  // The format is ...
  // <n>#"<target>" [[-]<argument>][?<module>,<icon index>]
  // where ...
  // <> = variable
  // [] = optional
  // n = number of characters after the hash, up to and including the last comma
  // eg. 63#\Windows\iexplore.exe -http://www.google.com?\Windows\test.exe,101
  if (!object_path) {
    return false;
  }
  std::string16 shortcut = STRING16(L"\"");
  shortcut += object_path;
  shortcut += STRING16(L"\"");
  if (arguments) {
    shortcut += STRING16(L" ");
    shortcut += arguments;
  }
  if (icon_path) {
    shortcut += STRING16(L"?");
    shortcut += icon_path;
    shortcut += STRING16(L",");
  }
  shortcut = IntegerToString16(shortcut.size()) + STRING16(L"#") + shortcut;
  if (icon_path) {
    shortcut += IntegerToString16(kDllIconId);
  }
  std::string shortcut_utf8;
  if (!String16ToUTF8(shortcut.c_str(), &shortcut_utf8)) {
    return false;
  }
  // If the file already exists, this will fail, but that's OK.
  File::CreateNewFile(link_path);
  if (!File::WriteBytesToFile(
           link_path,
           reinterpret_cast<const uint8*>(shortcut_utf8.c_str()),
           shortcut_utf8.size())) {
    return false;
  }
  return true;
#else
  HRESULT result;
  IShellLink* shell_link;

  result = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*)&shell_link);
  if (SUCCEEDED(result)) {
    shell_link->SetPath(object_path);
    shell_link->SetArguments(arguments);
    shell_link->SetIconLocation(icon_path, 0);

    IPersistFile* persist_file;
    result = shell_link->QueryInterface(IID_IPersistFile,
        reinterpret_cast<void**>(&persist_file));

    if (SUCCEEDED(result)) {
      result = persist_file->Save(link_path, TRUE);
      persist_file->Release();
    }
    shell_link->Release();
  }
  return SUCCEEDED(result);
#endif
}

static bool ReadShellLink(const char16 *link_path,
                          std::string16 *icon_path,
                          std::string16 *object_path,
                          std::string16 *arguments) {
#ifdef WINCE
  char16 target[CHAR_MAX];
  if (SHGetShortcutTarget(link_path, target, CHAR_MAX) == FALSE) {
    return false;
  }
  if (icon_path) {
    std::vector<uint8> data;
    if (!File::ReadFileToVector(link_path, &data)) {
      return false;
    }
    std::string16 shortcut;
    if (!UTF8ToString16(reinterpret_cast<char*>(&data[0]), &shortcut)) {
      return false;
    }
    // If an icon is specified, its module file must be preceeded by "?" and
    // followed by ",".
    int start_index = shortcut.find(STRING16(L"?"));
    int end_index = shortcut.find(STRING16(L","));
    if (std::string16::npos != start_index &&
        std::string16::npos != end_index &&
        start_index < end_index) {
      *icon_path = shortcut.substr(start_index + 1,
                                   end_index - start_index - 1);
    } else {
      *icon_path = STRING16(L"");
    }
  }
  return true;
#else
  HRESULT result;
  IShellLink* shell_link;

  result = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID*)&shell_link);
  if (SUCCEEDED(result)) {
    IPersistFile* persist_file;
    result = shell_link->QueryInterface(
                             IID_IPersistFile,
                             reinterpret_cast<void**>(&persist_file));

    if (SUCCEEDED(result)) {
      result = persist_file->Load(link_path, TRUE);

      char16 icon_buffer[MAX_PATH];
      if (SUCCEEDED(result) && icon_path) {
        int icon_id;
        result = shell_link->GetIconLocation(icon_buffer, MAX_PATH, &icon_id);
      }
      char16 object_buffer[MAX_PATH];
      if (SUCCEEDED(result) && object_path) {
        result = shell_link->GetPath(object_buffer, MAX_PATH, NULL, 0);
      }
      char16 arg_buffer[MAX_PATH];
      if (SUCCEEDED(result) && arguments) {
        result = shell_link->GetArguments(arg_buffer, MAX_PATH);
      }

      if (SUCCEEDED(result)) {
        // Only set the return values when the function succeeded.
        if (icon_path)
          *icon_path = icon_buffer;
        if (object_path)
          *object_path = object_buffer;
        if (arguments)
          *arguments = arg_buffer;
      }
      persist_file->Release();
    }
    shell_link->Release();
  }
  return SUCCEEDED(result);
#endif
}

static bool GetShortcutLocationPath(std::string16 *shortcut_location_path,
                                    uint32 location) {
  assert(shortcut_location_path);
  char16 path_buf[MAX_PATH];

  // We use the old version of this function because the new version apparently
  // won't tell you the Desktop folder path.
#ifdef WINCE
  // On WinCE, we only support desktop.
  if (location != Desktop::SHORTCUT_LOCATION_STARTMENU) {
    assert(false);
    return false;
  }

  // On Pocket PC, we'd like to add to 'Start -> Programs'. On SmartPhone, this
  // location doesn't exist, so we'd like to add to 'Start'. Rather than brittle
  // phone detection, just try the former location and if it fails, try the
  // second.
  if (SHGetSpecialFolderPath(NULL, path_buf, CSIDL_PROGRAMS, FALSE) ||
      SHGetSpecialFolderPath(NULL, path_buf, CSIDL_STARTMENU, TRUE)) {
    *shortcut_location_path = path_buf;
    return true;
  }
#else

  switch (location) {
    case Desktop::SHORTCUT_LOCATION_DESKTOP:
      if (SHGetSpecialFolderPath(NULL, path_buf, CSIDL_DESKTOPDIRECTORY,
                                 TRUE)) {
        *shortcut_location_path = path_buf;
        return true;
      }
      break;
    case Desktop::SHORTCUT_LOCATION_QUICKLAUNCH:
      if (SHGetSpecialFolderPath(NULL, path_buf, CSIDL_APPDATA, TRUE)) {
        *shortcut_location_path = path_buf;
        *shortcut_location_path +=
            STRING16(L"\\Microsoft\\Internet Explorer\\Quick Launch");
        return true;
      }
      break;
    case Desktop::SHORTCUT_LOCATION_STARTMENU:
      if (SHGetSpecialFolderPath(NULL, path_buf, CSIDL_PROGRAMS, TRUE)) {
        *shortcut_location_path = path_buf;
        return true;
      }
      break;
  }
#endif
  return false;
}

// Returns the arguments string we should use to launch the browser with the
// given URL.
#if BROWSER_CHROME
static std::string16 GetArgumentsString(const std::string16 &url)  {
  std::string url_utf8;
  String16ToUTF8(url.c_str(), &url_utf8);
  char* arguments_utf8 = NULL;
  bool rv = g_cpbrowser_funcs.get_command_line_arguments(
      g_cpid, NULL, url_utf8.c_str(), &arguments_utf8) == CPERR_SUCCESS;

  std::string16 arguments;
  if (rv && arguments_utf8) {
    UTF8ToString16(arguments_utf8, &arguments);
    g_cpbrowser_funcs.free(arguments_utf8);
  }

  return arguments;
}
#else
static inline std::string16 GetArgumentsString(const std::string16 &url)  {
  return url;
}
#endif

bool CreateShortcutFileWin32(const std::string16 &name,
                             const std::string16 &browser_path,
                             const std::string16 &url,
                             const std::string16 &icons_path,
                             uint32 location,
                             std::string16 *error) {
  // Note: We assume that shortcut.app_name has been validated as a valid
  // filename and that the shortuct.app_url has been converted to absolute URL
  // by the caller.
  std::string16 link_path;
  if (!GetShortcutLocationPath(&link_path, location)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  link_path += STRING16(L"\\");
  link_path += name;
  link_path += STRING16(L".lnk");

  // Return immediately if shortcut already exists.
  if (ReadShellLink(link_path.c_str(), NULL, NULL, NULL)) {
    return true;
  }

  std::string16 arguments = GetArgumentsString(url);
  if (!CreateShellLink(link_path.c_str(), icons_path.c_str(),
                       browser_path.c_str(), arguments.c_str())) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  return true;
}
#endif  // #if defined(WIN32) && !defined(WINCE)
