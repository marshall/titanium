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

#ifdef WINCE
#include <windows.h>  // must be first
#include <winioctl.h>
#include "gears/installer/iemobile/process_restarter.h"
#include "gears/installer/iemobile/resource.h"  // for restart dialog strings

static const char16* kInternetExplorerExe = L"iexplore.exe";
// Note that this string does not appear to be localized by Microsoft.
static const char16* kInternetExplorerWindow = L"Internet Explorer";
static const char16* kGearsSite = L"http://gears.google.com/done.html";

HINSTANCE module_instance;
int _charmax = 255;

extern "C" {
BOOL WINAPI DllMain(HANDLE instance, DWORD reason, LPVOID reserved) {
  module_instance = static_cast<HINSTANCE>(instance);
  return TRUE;
}

// On Smartphones the registry keys don't get properly updated on subsequent
// installs, including autoupdates. They point to
// '\Windows\FilesToBeDeleted\TMPxxxx.tmp' instead of the Gears DLL.
// We rewrite them to the correct values.
// NOTE: this fix should be left in place even if the problem does not affect
// the current generation of Windows Mobile phones. There may still be old
// devices in use!
// 1. The Bho key comes from base\ie\bho.rgs.
static const char16* kBhoKey =
    STRING16(L"CLSID\\{E0FEFE40-FBF9-42AE-BA58-794CA7E3FB53}\\InprocServer32");
// 2. The Factory ActiveX key comes from factory\factory_ie.rgs.
static const char16* kFactoryActiveXKey =
    STRING16(L"CLSID\\{C93A7319-17B3-4504-87CD-03EFC6103E6E}\\InprocServer32");
// 3. The Html dialog key comes from ui\ie\html_dialog_bridge_iemobile.rgs.
static const char16* kHtmlDialogActiveXKey =
    STRING16(L"CLSID\\{134AB400-1A81-4fc8-85DD-29CD51E9D6DE}\\InprocServer32");
// 4. The tools menu key comes from ui\ie\tools_menu_item.rgs.
static const char16* kToolsMenuKey =
    STRING16(L"CLSID\\{0B4350D1-055F-47A3-B112-5F2F2B0D6F08}\\InprocServer32");
// 5. The TypeLib key comes from base\ie\interfaces.idl.
static const char16* kTypeLibKey =
    STRING16(L"TypeLib\\{7708913A-B86C-4D91-B325-657DD5363433}\\1.0\\0\\win32");
// Note that the wince_cab.inf file specifies Program Files as the installation
// directory (%CE1%).
static const char16* kCorrectValue =
    STRING16(L"\\Program Files\\" PRODUCT_FRIENDLY_NAME
             L"\\" PRODUCT_SHORT_NAME L".dll");
static void CheckRegistryKeys();

// The functions below are called during the Gears installation process.

// TODO(andreip): remove this solution if we ever figure out a way to make
// IE reload BHOs at runtime, so that our extensions to the Tools menu
// would be visible without having to restart the browser.

// This defines a return type for the intaller functions. The pattern
// in <ce_setup.h> is that 0 represents the positive action (continue,
// done), while 1 is the negative action (cancel, uninstall).
enum InstallerActions {
  kContinue = 0,
  kCancel,
};

// This is called before the installation begins.
__declspec(dllexport) InstallerActions Install_Init(
    HWND parent_window,
    BOOL is_first_call,
    BOOL is_previously_installed,
    LPCTSTR installation_directory) {
  // We only handle the first call.
  if (!is_first_call) return kContinue;
  ProcessRestarter restarter(kInternetExplorerExe, kInternetExplorerWindow);
  restarter.KillTheProcess(
      1000,
      ProcessRestarter::KILL_METHOD_1_WINDOW_MESSAGE |
      ProcessRestarter::KILL_METHOD_3_TERMINATE_PROCESS,
      NULL);  
  return kContinue;
}

// This is called after installation completed. We start Internet Explorer
// and point it to the Gears site.
__declspec(dllexport) InstallerActions Install_Exit(
    HWND    parent_window,
    LPCTSTR final_install_dir,
    WORD    failed_directories,
    WORD    failed_files,
    WORD    failed_registry_keys,
    WORD    failed_registry_values,
    WORD    failed_shortcuts) {
  ProcessRestarter restarter(kInternetExplorerExe, kInternetExplorerWindow);
  bool is_running = false;
  HRESULT result = restarter.IsProcessRunning(&is_running);
  if ((SUCCEEDED(result) && is_running) || FAILED(result)) {
    // We could not kill IE or we could not even tell if it was running.
    // Tell the user he needs to reboot. This is because Gears may appear to be
    // working (creations of Gears objects will succeed) but the HttpHandler
    // and the Settings menu entry will not be registered with IE.
    // This leaves Gears in an inconsistent state until IE is restarted.
    LPCTSTR title = reinterpret_cast<LPCTSTR>(LoadString(
        module_instance,
        IDS_RESTART_DIALOG_TITLE,
        NULL,
        0));
    LPCTSTR fail_message = reinterpret_cast<LPCTSTR>(LoadString(
        module_instance,
        IDS_REBOOT_MESSAGE,
        NULL,
        0));

    if (!fail_message || !title) return kContinue;

    MessageBox(parent_window, fail_message, title, MB_OK | MB_ICONEXCLAMATION);
  } else {
    if (FAILED(restarter.StartTheProcess(kGearsSite))) {
      // Unfortunately we failed, so inform the user.
      LPCTSTR title = reinterpret_cast<LPCTSTR>(LoadString(
          module_instance,
          IDS_RESTART_DIALOG_TITLE,
          NULL,
          0));
      LPCTSTR fail_message = reinterpret_cast<LPCTSTR>(LoadString(
          module_instance,
          IDS_START_FAILURE_MESSAGE,
          NULL,
          0));

      if (!fail_message || !title) return kContinue;

      MessageBox(parent_window, fail_message, title, MB_OK);
    }
  }
  CheckRegistryKeys();
  return kContinue;
}

// This is called before the uninstall begins.
__declspec(dllexport) InstallerActions Uninstall_Init(HWND parent_window,
                                                      LPCTSTR install_dir) {
  // Continue, please.
  return kContinue;
}

// This is called after the uninstall completed.
__declspec(dllexport) InstallerActions Uninstall_Exit(HWND parent_window) {
  // Not interested.
  return kContinue;
}

static char16* ReadValue(CRegKey* key) {  
  ULONG size = 0;
  if (key->QueryStringValue(L"", NULL, &size) != ERROR_SUCCESS) return NULL;
  char16* buffer = new char16[size+1];
  ZeroMemory(buffer, size + 1);
  if (key->QueryStringValue(L"", buffer, &size) != ERROR_SUCCESS) {
    delete [] buffer;
    return NULL;
  }
  return buffer;
}

static void ProcessKey(const char16* keyString) {
  CRegKey key;
  if (key.Open(HKEY_CLASSES_ROOT, keyString, KEY_READ) != ERROR_SUCCESS) {
    // If the key doesn't exist at all, something else may have gone wrong.
    // We don't attempt any modification in such cases and just return.
    return;
  }
  char16* value = ReadValue(&key);
  // If the value exists but doesn't contain the word "gears", we rewrite it.
  if (value != NULL && wcsstr(value, PRODUCT_SHORT_NAME) == NULL) {
    key.SetStringValue(NULL, kCorrectValue);
  }
  delete [] value;
  key.Close();
}

static void CheckRegistryKeys() {
  // We open each of the five keys and look at their default string values.
  // If they are not what they should be (e.g. they point to FilesToBeDeleted),
  // we rewrite them programatically.
  // 1: The BHO
  ProcessKey(kBhoKey);
  // 2: The Factory ActiveX
  ProcessKey(kFactoryActiveXKey);
  // 3: The html dialog ActiveX
  ProcessKey(kHtmlDialogActiveXKey);
  // 4: The Tools menu entry
  ProcessKey(kToolsMenuKey);
  // 5: The TypeLib key
  ProcessKey(kTypeLibKey);
}

};  // extern "C"
#endif  // #ifdef WINCE
