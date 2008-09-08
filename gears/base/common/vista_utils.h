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

#ifndef GEARS_BASE_COMMON_VISTA_UTILS_H__
#define GEARS_BASE_COMMON_VISTA_UTILS_H__

#include <windows.h>
#include "gears/base/common/string16.h"

class VistaUtils {
 public:
  // Checks the OS version and returns true if we are running on Windows Vista
  // and false otherwise.
  static bool IsRunningOnVista();

  // Encapsulates the new IEIsProtectedModeProcess API. It uses dynamic loading
  // of ieframe.dll so that it will work on non-Vista OSes. Returns true if
  // we're in running in IE on Vista under protected mode.
  static bool IEIsProtectedModeProcess();

  // Makes use of the new SHGetKnownFolderPath function to get the path for
  // FOLDERID_LocalAppDataLow. If the function fails, false is returned and
  // fullpath is not modified. This function will fail if called on non-Vista
  // OSes.
  static bool GetLocalAppDataLowPath(std::string16 *fullpath);
};

#endif // GEARS_BASE_COMMON_VISTA_UTILS_H__
