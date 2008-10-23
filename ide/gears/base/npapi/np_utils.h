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

#ifndef GEARS_BASE_NPAPI_NP_UTILS_H__
#define GEARS_BASE_NPAPI_NP_UTILS_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/string16.h"

// Utility functions to allocate an NPString.  Must be freed using
// NPN_MemFree(str.utf8characters).
NPString NPN_StringDup(const char *value, int length);
NPString NPN_StringDup(const char16 *value, int length);
NPString NPN_StringDup(const NPString &str);

// Wrapper functions to abstract-out differences between WebKit & Gecko NPAPI.
inline const NPUTF8 *GetNPStringUTF8Characters(const NPString &npstr) {
  return npstr.UTF8Characters;
}

inline uint32 GetNPStringUTF8Length(const NPString &npstr) {
  return npstr.UTF8Length;
}

// Convenience wrappers to make an NPVariant from various string types.
#define STDSTRING_TO_NPVARIANT(str, var) \
  STRINGN_TO_NPVARIANT(str.data(), str.length(), var)

#define NPSTRING_TO_NPVARIANT(npstr, var) \
  STRINGN_TO_NPVARIANT(GetNPStringUTF8Characters(npstr), \
                       GetNPStringUTF8Length(npstr), var)

#endif  // GEARS_BASE_NPAPI_NP_UTILS_H__
