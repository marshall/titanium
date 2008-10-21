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

// This is a Windows Vista "broker" process. Right now, it can only be used
// to create desktop shortcuts from details passed in over the command line.

// TODO(aa): Generalize this so that it can be used whenever we need to elevate
// in Vista.

#include <tchar.h>
#include <windows.h>
#include <shellapi.h>

#include "gears/base/common/common.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/desktop/shortcut_utils_win32.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  CoInitializeEx(NULL, GEARS_COINIT_THREAD_MODEL);

  int argc;
  char16 **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argv) { return __LINE__; }  // return line as a ghetto error code

  if (argc != 6) {  // first argument is program path + filename
    return __LINE__;
  }

  std::string16 name(argv[1]);
  std::string16 browser_path(argv[2]);
  std::string16 url(argv[3]);
  std::string16 icon_path(argv[4]);
  const char16 *end_ptr;
  uint16 locations = ParseLeadingInteger(argv[5], &end_ptr);
  if (end_ptr == argv[5]) {
    return __LINE__;
  }

  LocalFree(argv);  // MSDN says to free 'argv', using LocalFree().

  std::string16 error;
  for (uint32 location = 0x8000; location > 0; location >>= 1) {
    if (locations & location) {
      if (!CreateShortcutFileWin32(name, browser_path, url, icon_path,
                                   location, &error)) {
        LOG((error.c_str()));
        return __LINE__;
      }
    }
  }

  return 0;
}
