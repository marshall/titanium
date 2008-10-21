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

#include "gears/base/common/vista_utils.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/base/ie/ie_version.h"


static CString GetIEVersion() {
  // See Microsoft's Knowledge Base article #164539 "How to Determine Which
  // Version of Internet Explorer Is Installed".
  // If for some reason their IE version reg key is corrupt or unreadable,
  // just assume a reasonable default.
  CString value(VistaUtils::IsRunningOnVista() ? _T("7.0") : _T("6.0"));
  CRegKey key;
  if (key.Open(HKEY_LOCAL_MACHINE,
               _T("Software\\Microsoft\\Internet Explorer"),
               KEY_READ) != ERROR_SUCCESS) {
    return value;
  }
  TCHAR buffer[256];
  ULONG buffer_size = sizeof(buffer)/sizeof(buffer[0]);
  if (key.QueryStringValue(_T("Version"),
                           buffer,
                           &buffer_size) == ERROR_SUCCESS) {
    value = buffer;
  }

  return value;
}


bool IsIEAtLeastVersion(int major, int minor, int build, int subbuild) {
  CString version(GetIEVersion());

  //  Grab the version parts. Tokenize will modify i.
  int i = 0;
  CString major_str(version.Tokenize(_T("."), i));
  int major_now = major_str.GetLength() > 0 ? _tstoi(major_str) : 0;
  if (major_now < major)
    return false;
  if (major_now > major)
    return true;

  CString minor_str(version.Tokenize(_T("."), i));
  int minor_now = minor_str.GetLength() > 0 ? _tstoi(minor_str) : 0;
  if (minor_now < minor)
    return false;
  if (minor_now > minor)
    return true;

  CString build_str(version.Tokenize(_T("."), i));
  int build_now = build_str.GetLength() > 0 ? _tstoi(build_str) : 0;
  if (build_now < build)
    return false;
  if (build_now > build)
    return true;

  CString subbuild_str(version.Tokenize(_T("."), i));
  int subbuild_now = subbuild_str.GetLength() > 0 ? _tstoi(subbuild_str) : 0;
  if (subbuild_now < subbuild)
    return false;

  return true;
}
