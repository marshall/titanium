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

#ifndef GLINT_INCLUDE_BASE_OBJECT_H__
#define GLINT_INCLUDE_BASE_OBJECT_H__

#include <stddef.h>  // for size_t

// Static global operators. This can be enabled in tests compilation
// to verify that Glint does not use platform-specific new and delete.
#if TESTING_MEMORY_OPERATORS
static void *operator new(size_t size) {
#error consider not using generic new
  return NULL;
}

static void *operator new[](size_t size) {
#error consider not using generic new
  return NULL;
}

static void operator delete(void *d) {}

static void operator delete[](void *d) {}

#endif

namespace glint {

// The base object - all classes (not simple structs) in Glint are derived
// from this one. This makes it possible to allocate all of them from a
// platform - specific allocator and control allocations/leaks
class BaseObject {
 public:
  // Need to have it, we do have some virtual methods in the hierarchy.
  virtual ~BaseObject();

  static void *operator new(size_t size);
  static void *operator new[](size_t size);
  static void *operator new(size_t size, const char *file, int line);

  static void operator delete(void *d);
  static void operator delete[](void *d);
  static void operator delete(void *d, const char *file, int line);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_BASE_OBJECT_H__
