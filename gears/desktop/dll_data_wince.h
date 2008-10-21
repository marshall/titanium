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

#ifdef WINCE

#ifndef GEARS_DESKTOP_DLL_DATA_WINCE_H__
#define GEARS_DESKTOP_DLL_DATA_WINCE_H__

#include "gears/base/common/basictypes.h"

// The resource ID of the icon in the DLL. Note that this value is baked into
// the DLL data in the arrays defined below, so should not be changed.
const int kDllIconId = 2001;

// The DLL data before the embedded icon resource.
extern const uint8 kIcon16DllBegin[];
extern const int kIcon16DllBeginSize;
extern const uint8 kIcon32and16DllBegin[];
extern const int kIcon32and16DllBeginSize;
// The DLL data for the icon header.
extern const uint8 kIcon16DllHeader[];
extern const int kIcon16DllHeaderSize;
extern const uint8 kIcon32and16DllHeader[];
extern const int kIcon32and16DllHeaderSize;
// The DLL data for the icon image footers.
extern const uint8 kIcon16Image16Footer[];
extern const int kIcon16Image16FooterSize;
extern const uint8 kIcon32and16Image32Footer[];
extern const int kIcon32and16Image32FooterSize;
extern const uint8 kIcon32and16Image16Footer[];
extern const int kIcon32and16Image16FooterSize;
// The DLL data after the embedded icon resource.
extern const uint8 kIcon16DllEnd[];
extern const int kIcon16DllEndSize;
extern const uint8 kIcon32and16DllEnd[];
extern const int kIcon32and16DllEndSize;

#endif  // GEARS_DESKTOP_DLL_DATA_WINCE_H__

#endif  // WINCE
