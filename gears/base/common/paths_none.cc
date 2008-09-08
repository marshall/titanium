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

#ifdef BROWSER_NONE

#include "gears/base/common/paths.h"

// Constants to access Windows registry.
#ifdef WIN32
static const char16 *kGearsRootRegKey = L"Software\\Google\\Gears";
static const char16 *kGearsInstallPath = L"install_dir";
static const char16 *kGearsInstallVersion = L"install_ver";
#endif  // WIN32

#ifdef WIN32
// Helper function to retrieve a string-typed registry value.
bool GetRegValueHelper(HKEY parent_key,
                       const char16 *subkey_name,
                       const char16 *value_name,
                       std::string16 *value) {
  assert(subkey_name && *subkey_name);
  assert(value);

  HKEY key = NULL;
  LONG res = ::RegOpenKeyEx(parent_key, subkey_name, 0, KEY_READ, &key);
  if (res != ERROR_SUCCESS) {
    return false;
  }

  DWORD type = 0;
  DWORD byte_count = 0;
  res = ::RegQueryValueEx(key, value_name, 0, &type, NULL, &byte_count);
  if (byte_count != 0 && type == REG_SZ) {
    value->resize(byte_count / sizeof(wchar_t) + 1);
    res = ::RegQueryValueEx(key, value_name, 0, &type,
                            reinterpret_cast<byte*>(&value->at(0)),
                            &byte_count);
    value->resize(wcslen(value->data()));
  } else {
    value->clear();
  }

  ::RegCloseKey(key);

  return res == ERROR_SUCCESS;
}

bool GetInstallDirectory(std::string16 *path) {
  std::string16 install_path;
  if (!GetRegValueHelper(HKEY_LOCAL_MACHINE,
                         kGearsRootRegKey,
                         kGearsInstallPath,
                         &install_path) ||
      install_path.empty()) {
    return false;
  }

  std::string16 install_version;
  if (!GetRegValueHelper(HKEY_LOCAL_MACHINE,
                         kGearsRootRegKey,
                         kGearsInstallVersion,
                         &install_version) ||
      install_version.empty()) {
    return false;
  }

  *path = install_path;
  if (path->at(path->length() - 1) != L'\\') {
    path->append(L"\\");
  }
  path->append(L"Shared\\");
  path->append(install_version);

  return true;
}
#endif  // WIN32

#endif  // BROWSER_NONE
