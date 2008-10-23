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

// Utilities operating on NSStrings, char16*, std::string16, etc.

#ifndef GEARS_BASE_COMMON_STRING_UTILS_OSX_H__
#define GEARS_BASE_COMMON_STRING_UTILS_OSX_H__

#include <CoreFoundation/CoreFoundation.h>
#include "gears/base/common/string16.h"

// Create a String16 from a CFStringRef
bool CFStringRefToString16(CFStringRef str, std::string16 *out16);

// Create a CFStringRef from a String16.  Caller is responsible for releasing
// CFStringRef.
CFStringRef CFStringCreateWithString16(const char16 *str);

// Convert an input buffer encoded with the specified charset to UTF16.
// TODO(bgarcia): Implement this for other platforms and move this
// declaration to string_utils.h
bool ConvertToString16FromCharset(const char *in, int len,
                                  const std::string16 &charset,
                                  std::string16 *out16);

#endif  // GEARS_BASE_COMMON_STRING_UTILS_OSX_H__
