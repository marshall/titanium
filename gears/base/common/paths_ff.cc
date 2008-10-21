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

class nsIFile; // must declare this before including nsDirectoryServiceUtils.h
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsXPCOM.h>
#include <gecko_sdk/include/nsStringAPI.h>
#include <gecko_sdk/include/nsILocalFile.h>
#include <gecko_sdk/include/nsDirectoryServiceDefs.h>
#include <gecko_sdk/include/nsDirectoryServiceUtils.h>

#include "genfiles/product_constants.h"
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string16.h"

#ifdef WIN32
bool GetInstallDirectory(std::string16 *path) {
  if (!GetUmbrellaInstallDirectory(path)) return false;
  (*path) += STRING16(L"\\Firefox");
  return true;
}
#else
// The text between the slashes must match the Gears <em:id> tag in
// install.rdf.m4.
static const char16 *kGearsDir =
  STRING16(L"extensions/{000a9d1c-beef-4f90-9363-039d445309b8}");

bool GetInstallDirectory(std::string16 *path) {
  // Get the "regular" profile directory (not the "local").
  nsresult nr;
  nsCOMPtr<nsIFile> profile_path;
  nr = NS_GetSpecialDirectory("ProfD", getter_AddRefs(profile_path));
  if (NS_FAILED(nr)) {
    return false;
  }

  // The XULPlanet docs warn against using the 'path' property in non-XUL
  // functions. But Darin says that was only a problem on MacOS 9;
  // Linux, Windows, and OSX and later are fine.
  nsString ns_path;
  profile_path->GetPath(ns_path);

  (*path) = ns_path.get();
  (*path) += kPathSeparator;
  (*path) += kGearsDir;

  return true;
}

#endif

bool GetComponentDirectory(std::string16 *path) {
  if (!GetInstallDirectory(path)) return false;
  AppendName(STRING16(L"components"), path);
  return true;
}

bool GetBaseResourcesDirectory(std::string16 *path) {
#if defined(OS_MACOSX) || defined(LINUX)
  std::string16 tmp_path;

  if (!GetInstallDirectory(&tmp_path)) {
    return false;
  }

  (*path) = tmp_path + kPathSeparator;
  (*path) += STRING16(L"resources");

  return true;
#else
  // TODO(nigeltao): implement on Windows.
  return false;
#endif
}

// Append "for Firefox" to be consistent with IE naming scheme.
static const char16 *kDataSubdir = STRING16(PRODUCT_FRIENDLY_NAME
                                            L" for Firefox");

bool GetBaseDataDirectory(std::string16 *path) {
  std::string16 path_long;

  // Get the "local" profile directory (or the regular profile directory
  // if the former is undefined).
  nsresult nr;
  nsCOMPtr<nsIFile> profile_path;
  nr = NS_GetSpecialDirectory("ProfLD", getter_AddRefs(profile_path));
  if (NS_FAILED(nr)) {
    return false;
  }

  // The XULPlanet docs warn against using the 'path' property in non-XUL
  // functions. But Darin says that was only a problem on MacOS 9;
  // Linux, Windows, and OSX and later are fine.
  nsString ns_path;
  profile_path->GetPath(ns_path);

  path_long = ns_path.get();
  path_long += kPathSeparator;
  path_long += kDataSubdir;

  // Create the directory prior to getting the name in short form on Windows.
  // We do this to ensure the short name generated will actually map to our
  // directory rather than another file system object created before ours.
  // Also, we do this for all OSes to behave consistently.
  // TODO(michaeln): Document this behavior in the .h file and
  // update the callers of this function to no longer do this themselves.
  if (!File::RecursivelyCreateDir(path_long.c_str())) {
    return false;
  }

#ifdef WIN32
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
#else
  (*path) = path_long;
#endif

  return true;
}
