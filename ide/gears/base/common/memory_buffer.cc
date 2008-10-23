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


#include "gears/base/common/memory_buffer.h"

#include <string.h>
#include <algorithm>
#include <cassert>

namespace {
// This value is the initial capacity allocated.
const MemoryBuffer::size_type gInitialCapacity = 16;
}  // namespace


MemoryBuffer::MemoryBuffer() : capacity_(0), size_(0) {
}


MemoryBuffer::MemoryBuffer(size_type num) : capacity_(0), size_(0) {
  Resize(num);
}


void MemoryBuffer::Append(const value_type *data, size_type num) {
  Reserve(size_ + num);
  memcpy(buffer_.get() + size_, data, static_cast<size_t>(num));
  size_ += num;
}


MemoryBuffer::value_type *MemoryBuffer::Data(size_type offset) const {
  assert(offset < size_);
  assert(offset >= 0);
  return buffer_.get() + offset;
}



void MemoryBuffer::Reserve(size_type num) {
  if (num < capacity_) return;
  if (capacity_ == 0) capacity_ = gInitialCapacity;
  assert(capacity_ >= gInitialCapacity);
  // We double the capacity_ whenever more memory is needed.
  while (capacity_ < num) capacity_ *= 2;
  value_type *temp(reinterpret_cast<value_type*>(
                       realloc(buffer_.get(), static_cast<size_t>(capacity_))));
  assert(temp != NULL);
  if (temp != buffer_.get()) {
    buffer_.release();
    buffer_.reset(temp);
  }
}


void MemoryBuffer::Resize(size_type num) {
  assert(num >= 0);
  Reserve(num);
  size_ = num;
}


void MemoryBuffer::Swap(MemoryBuffer &mb) {
  std::swap(capacity_, mb.capacity_);
  std::swap(size_, mb.size_);
  buffer_.swap(mb.buffer_);
}
