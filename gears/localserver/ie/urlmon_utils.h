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

#ifndef GEARS_LOCALSERVER_IE_URLMON_UTILS_H__
#define GEARS_LOCALSERVER_IE_URLMON_UTILS_H__

#include <urlmon.h>
#include "gears/base/common/string16.h"  // for char16

#ifdef DEBUG
// Given a set of flags returned from GetBindInfo, dumps a comma delimited
// string to the debugger console (via LOG) showing which flags are set
void TraceBindFlags(DWORD flags);

// Given a bind status code reported to IBindStatusCallback::OnProgress, 
// returns a the status code as a string
const char16 *GetBindStatusLabel(DWORD status);

// Given an option passed into IWinInetInfo::QueryOption, returns
// the option as a string
const char16 *GetWinInetInfoLabel(DWORD option);

// Given an option passed into IWinInetHttpInfo::QueryInfo, returns
// the option as a string
const char16 *GetWinInetHttpInfoLabel(DWORD option);

// Given an option passed into IInternetProtocolInfo::QueryInfo, returns 
// the option as a string
const char16 *GetProtocolInfoLabel(QUERYOPTION option);
#endif

// Given an option passed into IWinInetHttpInfo::QueryInfo, returns
// the http header value associated with that option or NULL if the
// option does not map to a header value
const char16 *GetWinInetHttpInfoHeaderName(DWORD option);

#endif  // GEARS_LOCALSERVER_IE_URLMON_UTILS_H__
