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

// Basic types used in glint namespace.

#ifndef GLINT_INCLUDE_TYPES_H__
#define GLINT_INCLUDE_TYPES_H__

namespace glint {

// COMPILER PRAGMAS AND PLATFORM-INDEPENDENT THINGS
#if _MSC_VER > 1000

// Normally, projects in VC8 do not define DEBUG, unlike mk build environment.
#ifndef DEBUG
#ifdef _DEBUG
#define DEBUG
#endif  // _DEBUG
#endif  // DEBUG

#define MSC_ASSEMBLY 1

#define LITTLE_ENDIAN 1
#elif defined(__GNUC__)
// TODO(pankaj): See which ones of the above definitions are needed.
// DEBUG shouldn't be. LITTLE_ENDIAN depends on the platform?
#else
#error ** NEVER COMPILED BY ANYTHING ELSE BUT MSVC
#error ** SEE IF THE DEFINITIONS ABOVE SHOULD BE CHANGED
#endif

// Common Types
#ifndef NULL
#define NULL (0)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

// A macro to disallow the evil copy constructor and operator= functions
// This should be used in the private: declarations for a class
#ifndef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)
#endif  // DISALLOW_EVIL_CONSTRUCTORS

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned long long uint64;

typedef float  real32;
typedef double real64;

}  // namespace glint

#endif  // GLINT_INCLUDE_TYPES_H__
