// Copyright 2006, Google Inc.
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

#include <windows.h>
#include <shlobj.h>
#ifdef WINCE
// No shlwapi.h on Windows Mobile.
#else
#include <shlwapi.h>
#endif
#include "genfiles/product_constants.h"
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/vista_utils.h"
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"
#endif

// The convention under APPDATA is \Company\Product.
// Append "for Internet Explorer" in case we support other browsers and
// want to use this same directory.
static const char16 *kDataSubdir = STRING16(L"Google\\"
                                            PRODUCT_FRIENDLY_NAME
                                            L" for Internet Explorer");

bool GetInstallDirectory(std::string16 *path) {
#ifdef WINCE
  assert(path);
  wchar_t dir[MAX_PATH];
  HRESULT hr = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES,
                               NULL, // user access token
                               SHGFP_TYPE_CURRENT, dir);
  if (FAILED(hr)) {
    return false;
  }
  *path = dir;
  *path += kPathSeparator;
  *path += PRODUCT_FRIENDLY_NAME;
  return true;
#else
  if (!GetUmbrellaInstallDirectory(path)) return false;
  *path += STRING16(L"\\Internet Explorer\\" PRODUCT_VERSION_STRING);
  return true;
#endif
}

bool GetComponentDirectory(std::string16 *path) {
  return GetInstallDirectory(path);
}

bool GetBaseResourcesDirectory(std::string16 *path) {
  // TODO(nigeltao): implement.
  return false;
}

bool GetBaseDataDirectory(std::string16 *path) {
  std::string16 path_long;

  if (VistaUtils::IsRunningOnVista()) {
    // Get the low rights app data folder on Vista for two reasons.
    // 1. Avoids problems with virtualized file paths, we won't be using them
    // 2. Instances of IE running in protected mode and elevated mode
    //    will use the same set of data files, as they should.
    if (!VistaUtils::GetLocalAppDataLowPath(&path_long)) {
      return false;
    }
  } else {
    wchar_t dir[MAX_PATH];
    HRESULT hr = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA,
                                NULL, // user access token
                                SHGFP_TYPE_CURRENT, dir);
    // Note: CSIDL_LOCAL_APPDATA is available in shell32.dll version 5.0+,
    // which means Win2K/XP and higher, plus WinME. On Windows Mobile
    // we define it to CSIDL_APPDATA.
    if (FAILED(hr)) {
      return false;
    }

    path_long = dir;
  }

  path_long += kPathSeparator;
  path_long += kDataSubdir;

  // Create the directory prior to getting the name in short form on Windows.
  // We do this to ensure the short name generated will actually map to our
  // directory rather than another file system object created before ours.
  // Also, we do this for all OSes to behave consistently.
  if (!File::RecursivelyCreateDir(path_long.c_str())) {
    return false;
  }

  // Shorten the path to mitigate the 'path is too long' problems we
  // have on Windows, we append to this path when forming full paths
  // to data files. Callers can mix short and long path components in
  // a single path string. The only requirement is that the combination
  // fits in MAX_PATH.
  wchar_t path_short[MAX_PATH];
  DWORD rv = GetShortPathNameW(path_long.c_str(), path_short, MAX_PATH);
  if (rv == 0 || rv > MAX_PATH) {
    return false;
  }

  (*path) = path_short;
  return true;
}
