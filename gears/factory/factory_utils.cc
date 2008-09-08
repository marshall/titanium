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

#ifdef BROWSER_WEBKIT
#include <sys/time.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

#include "gears/factory/factory_utils.h"

#include "genfiles/product_constants.h"
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "gears/base/common/stopwatch.h"
#include "gears/ui/common/permissions_dialog.h"

#if BROWSER_FF3
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsISupportsPrimitives.h>
#include <gecko_sdk/include/nsIProperties.h>
#include <gecko_internal/nsIDOMWindowInternal.h>
#include <gecko_internal/nsIPromptService.h>
#endif

const char16 *kAllowedClassVersion = STRING16(L"1.0");

const char16 *kPermissionExceptionString =
  STRING16(L"Page does not have permission to use " PRODUCT_FRIENDLY_NAME L".");

const char16 *kGoogleUpdateClientsRegKey =
                  STRING16(L"Software\\Google\\Update\\ClientState");

#if BROWSER_FF || BROWSER_IE
#ifdef WIN32
const char16 *kGoogleUpdateGearsClientGuid =
                  STRING16(L"{283EAF47-8817-4c2b-A801-AD1FADFB7BAA}");
#endif
#elif BROWSER_WEBKIT
const char16 *kGoogleUpdateGearsClientGuid =
                  STRING16(L"{0006FF50-C0C0-4B9B-973C-4CF98BF4EA9D}");
#elif BROWSER_NPAPI
const char16 *kGoogleUpdateGearsClientGuid =
                  STRING16(L"{00058422-BABE-4310-9B8B-B8DEB5D0B68A}");
#endif

const char16 *kGoogleUpdateDidRunValue = STRING16(L"dr");

void AppendBuildInfo(std::string16 *s) {
  s->append(STRING16(PRODUCT_VERSION_STRING));


#ifdef OFFICIAL_BUILD
  s->append(STRING16(L";official"));
#else
  s->append(STRING16(L";developer"));
#endif


#ifdef DEBUG
  s->append(STRING16(L";dbg"));
#else
  s->append(STRING16(L";opt"));
#endif


#if defined(OS_MACOSX)
  s->append(STRING16(L";osx"));
#elif defined(LINUX)
  s->append(STRING16(L";linux"));
#elif defined(WIN32)
#ifdef WINCE
  s->append(STRING16(L";wince"));
#else
  s->append(STRING16(L";win32"));
#endif
#else
  s->append(STRING16(L";unknown_os"));
#endif


#if BROWSER_IE
  s->append(STRING16(L";ie"));
#ifdef WINCE
  s->append(STRING16(L"_mobile"));
#endif
#elif BROWSER_FF
  s->append(STRING16(L";firefox"));
#elif BROWSER_SAFARI
  s->append(STRING16(L";safari"));
#elif BROWSER_NPAPI
  s->append(STRING16(L";npapi"));
#else
  s->append(STRING16(L";unknown_browser"));
#endif
}

void SetActiveUserFlag() {
#ifdef WIN32
  // We use the HKCU version of the Google Update "did run" value so that
  // we can write to it from IE on Vista.
  HKEY reg_client_state;
  DWORD result = RegCreateKeyExW(HKEY_CURRENT_USER, kGoogleUpdateClientsRegKey,
                                 0, NULL, 0, KEY_WRITE, NULL,
                                 &reg_client_state, NULL);
  if (result == ERROR_SUCCESS) {
    HKEY reg_client;
    result = RegCreateKeyExW(reg_client_state, kGoogleUpdateGearsClientGuid,
                             0, NULL, 0, KEY_WRITE, NULL, &reg_client, NULL);
    if (result == ERROR_SUCCESS) {
      const char16 *kValue = L"1";
      const size_t num_bytes = sizeof(kValue[0]) * 2;  // includes trailing '\0'
      RegSetValueExW(reg_client, kGoogleUpdateDidRunValue, 0, REG_SZ,
                     reinterpret_cast<const BYTE *>(kValue), num_bytes);
      RegCloseKey(reg_client);
    }
    RegCloseKey(reg_client_state);
  }
#elif defined(BROWSER_WEBKIT)
  // Not more than once an hour.
  static const int64 kRateLimitMillis = 60 * 60 * 1000;
  
  // Perform rate-limiting.
  static int64 last_called_timestamp = 0;
  int64 now = GetCurrentTimeMillis();
  if ((now - last_called_timestamp) < kRateLimitMillis) return;
  last_called_timestamp = now;
  
  std::string16 home_dir;
  if (!GetUserHomeDirectory(&home_dir)) return;
  
  std::string16 keystone_dir = home_dir +
                    STRING16(L"/Library/Google/GoogleSoftwareUpdate/Actives/");
  // Keystone should create this directory for us, but we defensively create
  // it in case it doesn't exist.
  if (!File::RecursivelyCreateDir(keystone_dir.c_str())) return;
  
  std::string16 active_file = keystone_dir + kGoogleUpdateGearsClientGuid;
  if (!File::Exists(active_file.c_str())) {
    File::CreateNewFile(active_file.c_str());
    return;
  }
  
  std::string path_utf8;
  String16ToUTF8(active_file.c_str(), active_file.size(), &path_utf8);
  
  // Touch file - NULL means now.
  utimes(path_utf8.c_str(), NULL);
#endif  // #ifdef BROWSER_WEBKIT
}

bool RequiresLocalDataPermissionType(const std::string16 &module_name) {
  return module_name == STRING16(L"beta.audio") ||
         module_name == STRING16(L"beta.audiorecorder") ||
         module_name == STRING16(L"beta.canvas") ||
         module_name == STRING16(L"beta.console") ||
         module_name == STRING16(L"beta.database") ||
         module_name == STRING16(L"beta.databasemanager") ||
         module_name == STRING16(L"beta.imageloader") ||
         module_name == STRING16(L"beta.localserver") ||
         module_name == STRING16(L"beta.test") ||
         module_name == STRING16(L"beta.workerpool");
}
