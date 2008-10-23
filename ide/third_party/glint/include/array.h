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

#ifndef GLINT_INCLUDE_ARRAY_H__
#define GLINT_INCLUDE_ARRAY_H__

#include "glint/include/allocator.h"
#include "glint/include/base_object.h"
#include "glint/include/platform.h"
#include "glint/include/types.h"

namespace glint {

// A simple dynamic array to keep pointers to objects.
// Takes ownership of the pointers and calls 'delete' on them when needed.
// Designed to be very small for small numbers of elements
// Add() time for 'n' elements is n log n.
template <class T>
class Array : public BaseObject {
 public:
  Array(): buffer_(NULL), length_(0), capacity_(0) {
  }

  ~Array() {
    Reset();
  }

  void Swap(Array<T>* b) {
    T** bf = buffer_;
    buffer_ = b->buffer_;
    b->buffer_ = bf;

    int u = length_;
    length_ = b->length_;
    b->length_ = u;

    int c = capacity_;
    capacity_ = b->capacity_;
    b->capacity_ = c;
  }

  bool InsertAt(int position, T* item) {
    if (position <= length_) {
      // grow array
      if (!EnsureSize(length_ + 1))
        return false;

      // shift to the right
      for (int i = length_ - 1; i >= position; --i) {
        buffer_[i + 1] = buffer_[i];
      }

      buffer_[position] = item;
      ++length_;

      return true;
    } else {
      return false;
    }
  }

  bool Add(T* item) {
    if (!EnsureSize(length_ + 1))
      return false;

    buffer_[length_] = item;
    ++length_;

    return true;
  }

  bool RemoveAt(int index) {
    if (index >= length_)
      return false;
    T* e = EraseAt(index);
    delete e;
    return true;
  }

  // Removes slot from the array, returns the value that was stored there
  // and does not destruct the value. This is the only method here that
  // allows one to get ownership of stored object back from the Array.
  T* EraseAt(int index) {
    if (index < length_) {
      T* e = buffer_[index];

      if (length_ - index - 1 > 0) {
        int byte_size = (length_ - index - 1) * sizeof(T*);
        Allocator::Move(buffer_ + index, buffer_ + index + 1, byte_size);
      }

      --length_;
      return e;
    }
    return NULL;
  }

  void Reset() {
    CallDestructors();

    if (buffer_) Allocator::Free(buffer_);

    buffer_ = NULL;
    length_ = 0;
    capacity_ = 0;
  }

  inline T* operator[](int index) const {
    if (index < length_ && index >= 0) {
      return buffer_[index];
    } else {
      CrashOnBadIndex(index);  // crash here. No better option.
      return NULL;
    }
  }

  int FindSlowly(const T* target) const {
    for (int i = 0; i < length_; i++) {
      if (buffer_[i] == target) return i;
    }
    return -1;
  }

  inline int length() const {
    return length_;
  }

 private:
  void CallDestructors() {
    for (int i = 0; i < length_; i++) {
      T* e = buffer_[i];
      delete e;
    }
  }

  void CrashOnBadIndex(int index) const {
    platform()->CrashWithMessage(
      "Array - accessing index %d while having %d elements. Crash.\n",
      index, length_);
  }

  bool EnsureSize(int size) {
    if (size > capacity_) {
      if (capacity_ == 0) capacity_ = 1;
      while (capacity_ < size) {
        capacity_ *= 2;
      }

      int byte_size = capacity_ * sizeof(T*);
      T** newbuf = reinterpret_cast<T**>(Allocator::Allocate(byte_size));
      if (!newbuf) return false;

      if (buffer_ && length_)
        Allocator::Copy(newbuf, buffer_, length_ * sizeof(T*));

      if (buffer_)
        Allocator::Free(buffer_);

      buffer_ = newbuf;
    }
    return true;
  }

  T** buffer_;
  int length_;
  int capacity_;

  DISALLOW_EVIL_CONSTRUCTORS(Array<T>);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_ARRAY_H__
