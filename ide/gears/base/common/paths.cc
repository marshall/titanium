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

#if defined(WIN32) && !defined(WINCE)
#include <shlobj.h>
#endif

#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "genfiles/product_constants.h"


// Define the unique suffixes for each module.
const char16 *kDataSuffixForDatabase    = STRING16(L"#database");
const char16 *kDataSuffixForDesktop     = STRING16(L"#desktop");
const char16 *kDataSuffixForLocalServer = STRING16(L"#localserver");

const size_t kUserPathComponentMaxChars  = 64;

// Longest file extension we allow, including the preceding . e.g. '.extension'.
const size_t kFileExtensionMaxChars = 10;

bool GetDataDirectory(const SecurityOrigin &origin, std::string16 *path) {
  // Validate parameters.
  if (!IsStringValidPathComponent(origin.host().c_str()) ||
      !IsStringValidPathComponent(origin.scheme().c_str()) ||
      !IsStringValidPathComponent(origin.port_string().c_str())) {
    return false; // invalid security origin
  }

  // Files for a given origin live under /host/scheme_port.
  // This makes it easy to find all "www.google.com" folders in a dir list,
  // regardless scheme (http/https) or port.
  if (!GetBaseDataDirectory(path)) { return false; }

  (*path) += kPathSeparator;
  (*path) += origin.host();

  (*path) += kPathSeparator;
  (*path) += origin.scheme();
  (*path) += STRING16(L"_");
  (*path) += origin.port_string();

  return true;
}

void AppendName(const char16 *name, std::string16 *path) {
  // Validate parameters.
  assert((*path)[path->size() - 1] != kPathSeparator);
  assert(name);
  assert(IsStringValidPathComponent(name));

  (*path) += kPathSeparator;
  (*path) += name;
}

void RemoveName(std::string16 *path) {
  assert(path);
  size_t idx = path->rfind(kPathSeparator);
  if (idx != std::string16::npos) {
    path->erase(idx);
  }
}

void AppendDataName(const char16 *name, const char16 *module_suffix,
                    std::string16 *path) {
  // Validate parameters.
  assert(name);
  assert(IsStringValidPathComponent(name));
  assert(module_suffix);
  assert(module_suffix[0] != L'\0');
  assert(IsStringValidPathComponent(module_suffix));
  assert(path);

  // The data name is a simple concatenation of the inputs.
  (*path) += kPathSeparator;
  (*path) += name;
  (*path) += module_suffix;
}

bool IsUserInputValidAsPathComponent(const std::string16 &user_input,
                                     std::string16 *error_message) {
  // Note that zero length strings are legal, as Gears tags on it's own
  // identifier after user input.
  if (!IsStringValidPathComponent(user_input.c_str())) {
    if (error_message) {
      *error_message = STRING16(L"Name contains invalid characters: ")+
                       user_input +
                       STRING16(L".");;
    }
    return false;
  } else if (user_input.length() > kUserPathComponentMaxChars) {
    if (error_message) {
      *error_message = STRING16(L"Name cannot exceed ") +
                       IntegerToString16(kUserPathComponentMaxChars) +
                       STRING16(L" characters: ") +
                       user_input +
                       STRING16(L".");
    }
    return false;
  }
  return true;
}

#if defined(WIN32) && !defined(WINCE)
bool GetUmbrellaInstallDirectory(std::string16 *path) {
  wchar_t dir[MAX_PATH];

  HRESULT hr = SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES,
                                NULL,  // user access token
                                SHGFP_TYPE_CURRENT, dir);
  if (FAILED(hr) || hr == S_FALSE) {  // MSDN says to handle S_FALSE
    return false;
  }

  *path = dir;
  *path += STRING16(L"\\Google\\" PRODUCT_FRIENDLY_NAME);

  return true;
}
#else
// GetUmbrellaInstallDirectory not needed yet by other platforms.
#endif
