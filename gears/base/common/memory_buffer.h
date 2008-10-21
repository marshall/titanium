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

#ifndef GEARS_BASE_COMMON_MEMORY_BUFFER_H_
#define GEARS_BASE_COMMON_MEMORY_BUFFER_H_

#include "gears/base/common/basictypes.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// This class implements a resizable array of bytes.
//
// The main advantage over using std::vector is that it will not initialize
// the members.  It is meant to be used in cases where the internal array will
// be handed to other code which will write into it.
// TODO(bgarcia): Consider making this a template on value_type.
class MemoryBuffer {
 public:
  typedef uint8 value_type;
  typedef int64 size_type;

  // Creates an empty MemoryBuffer.
  MemoryBuffer();

  // Creates a MemoryBuffer of the given size.
  explicit MemoryBuffer(size_type num);

  // Appends the provided data to the end of the MemoryBuffer.
  void Append(const value_type *data, size_type num);

  // Returns the MemoryBuffer's capacity.
  size_type Capacity() const { return capacity_; }

  // Sets the MemoryBuffer's size to zero.
  void Clear() { size_ = 0; }

  // Returns a pointer to the MemoryBuffer's data at the given offset.
  value_type *Data(size_type offset) const;

  // Return whether or not the MemoryBuffer is empty.
  bool Empty() const { return size_ == 0; }

  // Allocates enough space to store num value_types.
  // If a reallocation must occur, current data up to Size() is copied.
  void Reserve(size_type num);

  // Allocates enough space to store num value_types, and sets the Size().
  void Resize(size_type num);

  // Returns the size of data currently stored.
  size_type Size() const { return size_; }

  // Swaps the contents of two MemoryBuffers.
  void Swap(MemoryBuffer &mb);

 private:
  size_type capacity_;
  size_type size_;
  scoped_ptr_malloc<value_type> buffer_;
};

#endif  // GEARS_BASE_COMMON_MEMORY_BUFFER_H_
