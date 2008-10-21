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

#ifdef WIN32

#ifdef OFFICIAL_BUILD

// The notification API has not been finalized for official builds.
#include <windows.h>
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  return 0;
}
#else

#include <assert.h>
#include <windows.h>
#include "gears/notifier/const_notifier.h"
#include "gears/notifier/notifier_utils_win32.h"

// Constants
static const char *kCoreEntryPoint = "_DllEntry@4";
static const int kFailedToLoadCoreDll = 1;
static const int kFailedToRunCoreDll = 2;
static const int kGenericShellError = 3;

typedef int (WINAPI *DLL_ENTRY_POINT)(const wchar_t *);

int InvokeNotifierCore(LPTSTR command_line) {
  // Get the install version.
  std::string16 version;
  GetNotifierVersion(&version);

  // Get the path to the current module.
  std::string16 module_path;
  GetMainModulePath(&module_path);

  HMODULE core_dll_module = NULL;
  std::string16 core_path;

  // Try to load the core DLL from the verson-specific sub-directory.
  if (!version.empty()) {
    core_path = module_path;
    core_path += version;
    core_path += L"\\";
    std::string16 core_dll_path = core_path + kCoreDllName;

    // We must load dependencies of core DLL modules from the same directory
    // where we load it from, as opposed to from the directory the exe is in
    // This is what LOAD_WITH_ALTERED_SEARCH_PATH does.
    core_dll_module = ::LoadLibraryEx(core_dll_path.c_str(),
                                      NULL,
                                      LOAD_WITH_ALTERED_SEARCH_PATH);
  }

  // Try to load the core DLL from the same directory.
  // This is for easy debugging.
#ifdef DEBUG
  if (!core_dll_module) {
    version.clear();
    core_path = module_path;
    std::string16 core_dll_path = core_path + kCoreDllName;
    core_dll_module = ::LoadLibrary(core_dll_path.c_str());
  }
#endif  // DEBUG

  // Bail out if we fail to load the core DLL.
  if (!core_dll_module) {
    return kFailedToLoadCoreDll;
  }

  // Get the entry point in the core DLL.
  DLL_ENTRY_POINT dll_entry_point = reinterpret_cast<DLL_ENTRY_POINT>(
      ::GetProcAddress(core_dll_module, kCoreEntryPoint));
  if (!dll_entry_point) {
    return kFailedToLoadCoreDll;
  }

  // Set the current directory to the path containing the core DLL.
  // This is to make all LoadLibrary call be able to find the needed DLLs.
  if (!::SetCurrentDirectory(core_path.c_str())) {
    return kGenericShellError;
  }

  // Call the entry point to do all the work.
  return (*dll_entry_point)(version.c_str());
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR command_line, int) {
  return InvokeNotifierCore(command_line);
}

#endif  // OFFICIAL_BUILD

#endif  // WIN32
