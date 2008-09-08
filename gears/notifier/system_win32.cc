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
#if WIN32

#include "gears/notifier/system.h"

#include <string>
#include <assert.h>
#include <atlbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <windows.h>

#include "gears/base/common/basictypes.h"
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/notifier/notifier_utils_win32.h"
#include "third_party/glint/include/platform.h"
#include "third_party/glint/include/rectangle.h"
#include "third_party/glint/include/root_ui.h"

// Constants.
static const char16 *kRegKeyStartMenuInternet =
    L"SOFTWARE\\Clients\\StartMenuInternet";
static const char16 *kRegKeyShellOpenCommand = L"\\shell\\open\\command";
static const char16 *kIeExeName = L"iexplore.exe";
static const char16 *kRegKeyIeClass =
    L"CLSID\\{0002DF01-0000-0000-C000-000000000046}\\LocalServer32";
static const char16 *kRegValueIeClass = L"";

// Get the current module path. This is the path where the module of the
// currently running code sits.
static std::string16 GetCurrentModuleFilename() {
  // Get the handle to the module containing the given executing address
  MEMORY_BASIC_INFORMATION mbi = {0};
  DWORD result = ::VirtualQuery(&GetCurrentModuleFilename, &mbi, sizeof(mbi));
  assert(result == sizeof(mbi));
  HMODULE module_handle = reinterpret_cast<HMODULE>(mbi.AllocationBase);

  // Get the path of the loaded module
  wchar_t buffer[MAX_PATH + 1] = {0};
  ::GetModuleFileName(module_handle, buffer, ARRAYSIZE(buffer));
  return std::string16(buffer);
}

std::string System::GetResourcePath() {
  std::string16 path = GetCurrentModuleFilename();
  size_t idx = path.rfind(L'\\');
  if (idx != std::string16::npos) {
    path = path.erase(idx);
  }
  path += L"\\";

  std::string result;
  String16ToUTF8(path.c_str(), &result);
  return result;
}

bool GetSpecialFolder(DWORD csidl, std::string16 *folder_path) {
  assert(folder_path);

  folder_path->clear();

  // Get the path of a special folder as an ITEMIDLIST structure.
  ITEMIDLIST *folder_location = NULL;
  if (FAILED(::SHGetFolderLocation(NULL, csidl, NULL, 0, &folder_location))) {
    return false;
  }

  // Get an interface to the desktop folder.
  CComPtr<IShellFolder> desktop_folder;
  if (SUCCEEDED(::SHGetDesktopFolder(&desktop_folder))) {
    assert(desktop_folder);

    // Ask the desktop for the display name of the special folder.
    STRRET str_ret = {0};
    str_ret.uType = STRRET_WSTR;
    if (SUCCEEDED(desktop_folder->GetDisplayNameOf(folder_location,
                                                   SHGDN_FORPARSING,
                                                   &str_ret))) {
      wchar_t *folder_name = NULL;
      if (SUCCEEDED(::StrRetToStr(&str_ret, folder_location, &folder_name))) {
        *folder_path = folder_name;
      }
      ::CoTaskMemFree(folder_name);
    }
  }

  ::CoTaskMemFree(folder_location);

  return !folder_path->empty();
}

bool System::GetUserDataLocation(std::string16 *path, bool create_if_missing) {
  assert(path);

  // Get the path to the special folder holding application-specific data.
  int folder_id = CSIDL_APPDATA;
  if (create_if_missing) {
    folder_id |= CSIDL_FLAG_CREATE;
  }
  if (!GetSpecialFolder(folder_id, path)) {
    return false;
  }

  // Add the directory for our product. If the directory does not exist, create
  // it.
  *path += kPathSeparator;
  *path += STRING16(PRODUCT_SHORT_NAME);

  if (create_if_missing && !File::DirectoryExists(path->c_str())) {
    if (!File::RecursivelyCreateDir(path->c_str())) {
      return false;
    }
  }

  return true;
}

void System::GetMainScreenWorkArea(glint::Rectangle *bounds) {
  assert(bounds);
  RECT work_area = {0};
  if (::SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0)) {
    bounds->Set(work_area.left,
                work_area.top,
                work_area.right,
                work_area.bottom);
  } else {
    // If failed to call SystemParametersInfo for some reason, we simply get the
    // full screen size as an alternative.
    bounds->Set(0,
                0,
                ::GetSystemMetrics(SM_CXSCREEN) - 1,
                ::GetSystemMetrics(SM_CYSCREEN) - 1);
  }
}

// Used in System::GetSystemFontScaleFactor.
struct FontScaleMapping {
  const wchar_t *size_name;
  double scale_factor;
};

// Get the scale factor of current system font to normal system font
// (i.e. DPI has been changed)
double System::GetSystemFontScaleFactor() {
  double factor = 1.0;

  // Count in the scaling due to DPI change
  const double kDefaultDPI = 96.0;
  HDC hdc = ::GetDC(NULL);
  factor *= ::GetDeviceCaps(hdc, LOGPIXELSY) / kDefaultDPI;
  ::ReleaseDC(NULL, hdc);

  const wchar_t kKeyName[] =
      L"Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager";
  HKEY key;
  if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CURRENT_USER,
                                      kKeyName,
                                      0,
                                      KEY_READ,
                                      &key)) {
    return factor;
  }

  // Count in the scaling due to font size change. Windows stores the
  // current 'font size name' in registry and AFAIK there can be 3 string
  // values for which we should check.
  const int kBufferSizeBytes = 64;
  BYTE buffer[kBufferSizeBytes];
  DWORD buffer_actual_bytes = kBufferSizeBytes;
  if (ERROR_SUCCESS == ::RegQueryValueEx(key,
                                         L"SizeName",
                                         NULL,
                                         NULL,
                                         buffer,
                                         &buffer_actual_bytes)) {
    const FontScaleMapping mappings[] = {
      { L"NormalSize",      1.0 },
      { L"LargeFonts",      13.0 / 11.0 },
      { L"ExtraLargeFonts", 16.0 / 11.0 },
    };
    wchar_t *font_size_name = reinterpret_cast<wchar_t*>(buffer);
    int font_size_name_length = buffer_actual_bytes / sizeof(font_size_name[0]);

    // RegQueryValueEx does not guarantee terminating \0. So check and if there
    // is one, don't include it in string.
    if (font_size_name_length >= 1 &&
        font_size_name[font_size_name_length - 1] == 0) {
      --font_size_name_length;
    }

    std::string16 size_name(font_size_name, font_size_name_length);
    for (size_t i = 0; i < ARRAYSIZE(mappings); ++i) {
      if (size_name == mappings[i].size_name) {
        factor *= mappings[i].scale_factor;
        break;
      }
    }
  }
  return factor;
}

class ScopedMenuHandle {
public:
  ScopedMenuHandle() {
    menu_ = ::CreatePopupMenu();
   }
  ~ScopedMenuHandle() {
    if (menu_) {
      ::DestroyMenu(menu_);
    }
  }
  bool isValid() { return menu_ != NULL; }
  HMENU get() { return menu_; }
private:
  HMENU menu_;
};

int System::ShowContextMenu(const MenuItem *menu_items,
                            size_t menu_items_count,
                            glint::RootUI *root_ui) {

  ScopedMenuHandle menu;
  if (!menu.isValid())
    return -1;

  // Build up the menu.
  for (size_t i = 0; i < menu_items_count; ++i) {
    if (menu_items[i].title == "-") {  // Insert a separator..
      if (!::AppendMenu(menu.get(),
                        MF_SEPARATOR,
                        menu_items[i].command_id,
                        NULL)) {
        return -1;
      }
    } else {                           // Or insert a textual menu item.
      std::string16 title;
      if (!UTF8ToString16(menu_items[i].title, &title))
        return -1;
      int command_id = menu_items[i].command_id;
      assert(command_id >= 0);
      ++command_id;  // To avoid id = 0, which is not valid as command id.

      bool isEnabled = menu_items[i].enabled;
      bool isChecked = menu_items[i].checked;

      if (!::AppendMenu(menu.get(), MF_STRING, command_id, title.c_str()))
        return -1;
      // New menu items are enabled by default, disable if needed.
      if (!isEnabled) {
        UINT flag = MF_BYCOMMAND | MF_GRAYED;
        if (::EnableMenuItem(menu.get(), command_id, flag) == -1)
          return -1;
      }
      // New menu items are unchecked by default, check if needed.
      if (isChecked) {
        UINT flag = MF_BYCOMMAND | MF_CHECKED;
        if (::CheckMenuItem(menu.get(), command_id, flag) == -1)
          return -1;
      }
    }
  }

  // Show the menu.
  HWND window_handle = reinterpret_cast<HWND>(
      glint::platform()->GetWindowNativeHandle(root_ui->GetPlatformWindow()));
  assert(window_handle);

  // If we don't set it to be foreground, it will not stop tracking even
  // if we click outside of menu.
  ::SetForegroundWindow(window_handle);

  POINT point = {0, 0};
  ::GetCursorPos(&point);
  // This returns 0 if menu is dismissed w/o selection made.
  int command_id = ::TrackPopupMenuEx(menu.get(),
                                      TPM_LEFTALIGN | TPM_RETURNCMD |
                                      TPM_NONOTIFY | TPM_LEFTBUTTON |
                                      TPM_VERTICAL,
                                      point.x,
                                      point.y,
                                      window_handle,
                                      NULL);

  // Return -1 in case the menu was simply dismissed, or command_id otherwise.
  // Increment/decrement is needed to work around the fact that 0 is not a
  // valid command id.
  return command_id - 1;
}

void System::ShowNotifierPreferences() {
  // TODO: Implement this.
  assert(false);
}

void GetDefaultBrowserName(std::string16 *name) {
  assert(name);
  name->erase();

  // Get the default browser name from current user registry.
  bool res = GetRegStringValue(HKEY_CURRENT_USER,
                               kRegKeyStartMenuInternet,
                               NULL,
                               name);

  // If failed, try to get from local machine registry.
  if (!res || name->empty()) {
    res = GetRegStringValue(HKEY_LOCAL_MACHINE,
                            kRegKeyStartMenuInternet,
                            NULL,
                            name);
  }

  // If still failed, we don't want to break in this case so
  // we default to IEXPLORE.EXE. A scenario where this can happen is
  // when the user installs a browser that configures itself to be default.
  // Then the user uninstalls or somehow removes that browser and the
  // registry is not updated correctly.
  if (!res || name->empty()) {
    name->assign(kIeExeName);
  }
}

// Removes things like %1 from browser's 'open' commmand.
void CleanBrowserPath(std::string16 *path) {
  assert(path);

  // Find one of '-nohome', '"%1"', '%1' and chop it off.
  std::string16::size_type pos = path->find(L"-nohome", 0);
  if (pos == std::string16::npos) {
    pos = path->find(L"\"%1\"", 0);
  }
  if (pos == std::string16::npos) {
    pos = path->find(L"%1", 0);
  }
  if (pos != std::string16::npos) {
    *path = path->substr(0, pos);
  }
}

bool GetDefaultBrowserPath(std::string16 *path) {
  assert(path);
  path->erase();

  // Get the default browser name.
  std::string16 name;
  GetDefaultBrowserName(&name);

  // Read the path corresponding to it.
  std::string16 browser_key(kRegKeyStartMenuInternet);
  browser_key += L"\\";
  browser_key += name;
  browser_key += kRegKeyShellOpenCommand;
  bool res = GetRegStringValue(HKEY_LOCAL_MACHINE,
                               browser_key.c_str(),
                               NULL,
                               path);

  // If failed and the default browser is not IE, try IE once again.
  if (!res || path->empty()) {
    browser_key = kRegKeyStartMenuInternet;
    browser_key += L"\\";
    browser_key += kIeExeName;
    browser_key += kRegKeyShellOpenCommand;
    if (GetRegStringValue(HKEY_LOCAL_MACHINE,
                          browser_key.c_str(),
                          NULL,
                          path)) {
      return false;
    }
  }

  CleanBrowserPath(path);
  return !path->empty();
}

bool DoExecute(const char16 *file, const char16 *cmd_line_options) {
  assert(file && * file);

  std::string16 cmd_line(file);
  std::string16::size_type idx = cmd_line.find(L"%1");
  if (idx != std::string16::npos) {
    cmd_line.replace(idx, 2, cmd_line_options);
  } else {
    if (cmd_line[0] != L'"' && cmd_line[cmd_line.length() - 1] != L'"') {
      cmd_line.insert(0, 1, L'"');
      cmd_line.append(1, L'"');
    }
    if (cmd_line_options && *cmd_line_options) {
      cmd_line.append(1, L' ');
      cmd_line.append(cmd_line_options);
    }
  }

  STARTUPINFO si = { sizeof(si) };
  PROCESS_INFORMATION pi = {0};
  if (!::CreateProcess(NULL,
                       const_cast<LPWSTR>(cmd_line.c_str()),
                       NULL,
                       NULL,
                       false,
                       0,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {
    return false;
  }

  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);

  return true;
}

bool System::OpenUrlInBrowser(const char16 *url) {
  assert(url && *url);

  // Try to open with default browser.
  std::string16 browser_path;
  bool found_browser_path = GetDefaultBrowserPath(&browser_path);
  if (found_browser_path &&
      !browser_path.empty() &&
      DoExecute(browser_path.c_str(), url)) {
    return true;
  }

  // Try to open with IE if can't open with default browser
  if (GetRegStringValue(HKEY_CLASSES_ROOT,
                        kRegKeyIeClass,
                        kRegValueIeClass,
                        &browser_path) &&
      DoExecute(browser_path.c_str(), url)) {
    return true;
  }

  // ShellExecute the url directly as a last resort
  return DoExecute(url, L"");
}

#endif  // WIN32
#endif  // OFFICIAL_BUILD
