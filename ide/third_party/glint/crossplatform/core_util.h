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

// This file contains implementation-specific definitions for
// Glint core and should only be included by Glint components

#ifndef GLINT_CROSSPLATFORM_CORE_UTIL_H__
#define GLINT_CROSSPLATFORM_CORE_UTIL_H__

#include "glint/include/platform.h"

namespace glint {

#ifdef DEBUG
#define ASSERT(expr) do { \
  if (!(expr)) \
    glint::platform()->CrashWithMessage(\
        "Failed ASSERT '%s' in file '%s', line %d\n", \
        #expr, __FILE__, __LINE__); \
} while (0)

#define ASSERT2(expr, message) do { \
  if (!(expr)) glint::platform()->CrashWithMessage( \
      "Failed ASSERT '%s' with messaqe '%s' in file '%s', line %d\n", \
      #expr, message, __FILE__, __LINE__); \
} while (0)

#define CHECK(expr) do { bool __error = (expr); if (!__error) \
  { \
    glint::platform()->Trace("Failed CHECK: %s(%d)\n", \
                              __FILE__, __LINE__); \
    return __error; \
  } \
} while (0)
#define VERIFY(expr) ASSERT(expr)
#else
#define ASSERT(expr) do {} while (0)
#define ASSERT2(expr, message) do {} while (0)
#define CHECK(expr) do { \
  bool __error = (expr); \
  if (!__error) return __error; \
} while (0)
#define VERIFY(expr) (expr)
#endif

inline int Round(real32 a) {
// TODO(dimich): - only works if FPU state is "round to nearest" and even
// then rounds to even. Experiment in VC8.
#if MSC_ASSEMBLY
  int retval;

  __asm fld a
  __asm fistp retval

  return retval;
#else
  return static_cast<int>(a + 0.5);
#endif
}

inline real32 DegToRad(real32 deg) { return .017453292519938f * deg; }
inline real32 RadToDeg(real32 rad) { return 57.29577951309f * rad; }

inline int Close(real32 x, real32 y) {
  // 10E-5 is close enough for most layout/rendering operations.
  // Where it matters, apply meaningful algorithm.
  real32 d = x - y;
  if (d < 0.0f) d = -d;
  return (d < 10E-5);
}

// New templated versions for cross-platform porting.
// They don't conflict with macros.  They don't conflict with std::min/max.
// Note on use: prefer min<real32>(x,y) rather than min(scast<real32>(x),y)
// when types are ambiguous.
template<class T>
inline T min(T a, T b) { return (a > b) ? b : a; }

template<class T>
inline T max(T a, T b) { return (a < b) ? b : a; }

template<class T>
inline T abs(T a) { return (a >= 0) ? a : (-a); }

}  // namespace glint

#endif  // GLINT_CROSSPLATFORM_CORE_UTIL_H__
