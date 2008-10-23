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
#ifdef WIN32

#include "gears/notifier/notifier_utils_win32.h"

#include <assert.h>
#include <tlhelp32.h>
#include <windows.h>

#include "gears/base/common/common.h"

// Constants
static const char16 *kNotifierRootRegKey = L"Software\\Google\\Gears";
static const char16 *kNotifierInstallPath = L"install_dir";
static const char16 *kNotifierInstallVersion = L"install_ver";

// Helper function to get the registry value.
bool GetRegStringValue(HKEY parent_key,
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

bool GetNotifierPath(std::string16 *path) {
  assert(path);

  if (!GetRegStringValue(HKEY_LOCAL_MACHINE,
                         kNotifierRootRegKey,
                         kNotifierInstallPath,
                         path)) {
    return false;
  }

  if (path->at(path->length() - 1) != L'\\') {
    path->append(L"\\");
  }
  path->append(L"Shared\\notifier.exe");

  return true;
}

bool GetNotifierVersion(std::string16 *version) {
  assert(version);
  return GetRegStringValue(HKEY_LOCAL_MACHINE,
                           kNotifierRootRegKey,
                           kNotifierInstallVersion,
                           version);
}

void GetMainModulePath(std::string16 *path) {
  assert(path);

  char16 temp[MAX_PATH + 1] = {0};
  ::GetModuleFileName(NULL, temp, ARRAYSIZE(temp));

  path->assign(temp);
  size_t idx = path->rfind('\\');
  if (idx != std::wstring::npos) {
    path->erase(idx + 1);
  }
}

// Function pointer to be called when enumerating processes
// Return true to abort the enumeration
typedef bool EnumProcessesFunc(const PROCESSENTRY32 &process, LONG_PTR param);

// Helper function to enumerate all processes.
bool EnumProcesses(EnumProcessesFunc *handler, LONG_PTR param) {
  assert(handler);

  HANDLE snapshot= ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    LOG(("CreateToolhelp32Snapshot failed with error=%u\n", ::GetLastError()));
    return false;
  }
  
  PROCESSENTRY32 process = {0};
  process.dwSize = sizeof(PROCESSENTRY32);
  for (BOOL found = ::Process32First(snapshot, &process);
       found;
       found = ::Process32Next(snapshot, &process)) {

    bool res = (*handler)(process, param);
    if (res) {
      break;
    }
  }

  ::CloseHandle(snapshot);

  return true;
}

struct GetParentProcessIdData {
  uint32 process_id;
  uint32 parent_process_id;
};

// Callback for EnumProcesses in order to get parent process ID.
bool GetParentProcessIdHandler(const PROCESSENTRY32 &process, LONG_PTR param) {
  assert(param);

  GetParentProcessIdData* data = 
      reinterpret_cast<GetParentProcessIdData*>(param);

  if (process.th32ProcessID == data->process_id) {
    data->parent_process_id = process.th32ParentProcessID;
    return true;
  }

  return false;
}

bool GetParentProcessId(uint32 process_id, uint32 *parent_process_id) {
  assert(parent_process_id);

  GetParentProcessIdData data;
  data.process_id = process_id;
  data.parent_process_id = 0;
  if (!EnumProcesses(&GetParentProcessIdHandler,
                     reinterpret_cast<LONG_PTR>(&data))) {
    return false;
  }

  *parent_process_id = data.parent_process_id;
  return S_OK;
}

#endif  // WIN32

#endif  // OFFICIAL_BUILD
