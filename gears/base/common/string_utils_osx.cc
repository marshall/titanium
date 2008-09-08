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

#ifdef OS_MACOSX

#include "gears/base/common/string_utils_osx.h"

#include "gears/base/safari/scoped_cf.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

bool CFStringRefToString16(CFStringRef str, std::string16 *out16) {
  if (!str || !out16)
    return false;
  
  unsigned long length = CFStringGetLength(str);
  const UniChar *outStr = CFStringGetCharactersPtr(str);

  if (!outStr) {
    scoped_array<UniChar> buffer(new UniChar[length + 1]);
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer.get());
    buffer[length] = 0;
    out16->assign(buffer.get());
  } else {
    out16->assign(outStr, length);
  }
  
  return true;
}

CFStringRef CFStringCreateWithString16(const char16 *str) {
  if (!str)
    return CFSTR("");
  
  return CFStringCreateWithCharacters(NULL, str, 
                                      std::char_traits<char16>::length(str));
}

bool ConvertToString16FromCharset(const char *in, int len,
                                  const std::string16 &charset,
                                  std::string16 *out16) {
  CFStringEncoding encoding;
  encoding = CFStringConvertIANACharSetNameToEncoding(
      CFStringCreateWithBytes(NULL, (const UInt8 *)charset.data(),
                              charset.size(), kCFStringEncodingUTF16, false));
  scoped_cftype<CFStringRef> in_str(CFStringCreateWithBytes(NULL, 
                                                            (const UInt8 *)in, 
                                                            len, 
                                                            encoding, 
                                                            false));
  if (!in_str.get()) {
    return false;
  }
  
  const UniChar *out_str = CFStringGetCharactersPtr(in_str.get());
  CFIndex length = CFStringGetLength(in_str.get());
  
  if (!length) {
    return false;
  }
  
  // If the outStr is empty, we'll have to convert in a slower way
  if (!out_str) {
    scoped_array<UniChar> buffer(new UniChar[length + 1]);
    CFStringGetCharacters(in_str.get(), CFRangeMake(0, length), buffer.get());
    buffer[length] = 0;
    out16->assign(buffer.get());
  } else {
    out16->assign(out_str, length);
  }
  
  return true;
}

#endif  // OS_MACOSX
