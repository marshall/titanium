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

#include "gears/base/common/string16.h"

#include <assert.h>
#include "gears/base/common/basictypes.h"


#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4244) // C4244: "possible loss of data" on assignment
#endif

// This is based on FastInt32ToBuffer from google3.
// The only difference is that we changed 'char' to template type 'CharT'
// allowing us to use this with char or char16, and 'int' to 'IntT' in order
// to support separate 32 and 64-bit implementations. (int64 is correct but
// slower for 32-bit integers on 32-bit platforms).

// Offset into buffer where FastIntToBuffer places the end of string
// null character.  Also used by FastIntToBufferLeft
static const int kFastIntToBufferOffset = 21;

template<typename IntT, typename CharT>
inline CharT *FastIntToBuffer(IntT i, CharT* buffer) {
  // We could collapse the positive and negative sections, but that
  // would be slightly slower for positive numbers...
  // 22 chars is enough to store -2**64, -18446744073709551616.
  CharT* p = buffer + kFastIntToBufferOffset;
  *p-- = '\0';
  if (i >= 0) {
    do {
      *p-- = '0' + i % 10;
      i /= 10;
    } while (i > 0);
    return p + 1;
  } else {
    // On different platforms, % and / have different behaviors for
    // negative numbers, so we need to jump through hoops to make sure
    // we don't divide negative numbers.
    if (i > -10) {
      i = -i;
      *p-- = '0' + i;
      *p = '-';
      return p;
    } else {
      // Make sure we aren't at MIN_INT, in which case we can't say i = -i
      i = i + 10;
      i = -i;
      *p-- = '0' + i % 10;
      // Undo what we did a moment ago
      i = i / 10 + 1;
      do {
        *p-- = '0' + i % 10;
        i /= 10;
      } while (i > 0);
      *p = '-';
      return p;
    }
  }
}

#ifdef WIN32
#pragma warning(pop)
#endif

template<typename IntT, typename StringT>
inline void IntegerToStringT(IntT i, StringT *result) {
  assert(result);
  typename StringT::value_type buffer[kFastIntToBufferOffset + 1];
  typename StringT::value_type *start = FastIntToBuffer(i, buffer);
  result->assign(start); 
}

std::string IntegerToString(int32 i) {
  std::string result;
  IntegerToStringT(i, &result);
  return result;
}

std::string16 IntegerToString16(int32 i) {
  std::string16 result;
  IntegerToStringT(i, &result);
  return result;
}

std::string Integer64ToString(int64 i) {
  std::string result;
  IntegerToStringT(i, &result);
  return result;
}

std::string16 Integer64ToString16(int64 i) {
  std::string16 result;
  IntegerToStringT(i, &result);
  return result;
}
