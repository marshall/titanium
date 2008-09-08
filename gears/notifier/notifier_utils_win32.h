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

#ifndef GEARS_NOTIFIER_NOTIFIER_UTILS_WIN32_H__
#define GEARS_NOTIFIER_NOTIFIER_UTILS_WIN32_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef WIN32

#include <windows.h>
#include "gears/base/common/string16.h"

// Get the installed path to the notifier executable.
bool GetNotifierPath(std::string16 *path);

// Get the installed version of the notifier executable.
bool GetNotifierVersion(std::string16 *version);

// Get the path to the main module.
void GetMainModulePath(std::string16 *path);

// Get the parent process ID of the specified process.
bool GetParentProcessId(uint32 process_id, uint32 *parent_process_id);

// Get the registry value of string type.
bool GetRegStringValue(HKEY parent_key, 
                       const char16 *subkey_name,
                       const char16 *value_name,
                       std::string16 *value);

#endif  // WIN32

#endif  // OFFICIAL_BUILD

#endif  // GEARS_NOTIFIER_NOTIFIER_UTILS_WIN32_H__
