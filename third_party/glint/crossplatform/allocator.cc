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

#include "glint/include/allocator.h"
#include "glint/include/platform.h"

namespace glint {

void *Allocator::Allocate(size_t byte_count) {
  return platform()->AllocateMemory(byte_count);
}

void *Allocator::Reallocate(void *p, size_t byte_count) {
  if (p) {
    return platform()->ReallocateMemory(p, byte_count);
  } else {
    return platform()->AllocateMemory(byte_count);
  }
}

void Allocator::Free(void *p) {
  if (p) {
    platform()->FreeMemory(p);
  }
}

void Allocator::Copy(void *to, const void *from, size_t byte_count) {
  platform()->CopyMemoryBlock(to, from, byte_count);
}

void Allocator::Move(void *to, const void *from, size_t byte_count) {
  platform()->MoveMemoryBlock(to, from, byte_count);
}

}  // namespace glint

