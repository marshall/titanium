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

#if defined(WIN32) && !defined(WINCE)
#include "gears/base/common/string16.h"

#include <windows.h>

static const char16 *kGoogleUpdateUsageRegKey =
    STRING16(L"Software\\Google\\Update\\ClientState\\"
             L"{283EAF47-8817-4c2b-A801-AD1FADFB7BAA}");
static const char16 *kGoogleUpdateUsageStatsValue = STRING16(L"usagestats");

bool IsUsageReportingAllowed() {
  HKEY reg_key;
  if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    kGoogleUpdateUsageRegKey, 0, KEY_READ,
                                    &reg_key)) {
    return false;
  }

  DWORD value = 0;
  DWORD byte_count = sizeof(DWORD);
  LONG result = RegQueryValueEx(reg_key, kGoogleUpdateUsageStatsValue,
                                NULL, NULL, reinterpret_cast<byte*>(&value),
                                &byte_count);
  RegCloseKey(reg_key);

  return result == ERROR_SUCCESS && value == 1;
}
#endif
