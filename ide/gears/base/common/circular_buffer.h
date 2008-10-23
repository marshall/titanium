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

#ifndef GEARS_BASE_COMMON_CIRCULAR_BUFFER_H__
#define GEARS_BASE_COMMON_CIRCULAR_BUFFER_H__

#include <assert.h>
#include <string.h>

// A primitive circular buffer class. The capacity of the circular buffer is
// one less than the size of the underlying memory block. This class does not
// assume ownership of the underlying memory block.
class CircularBuffer {
 public:
  CircularBuffer() : head_(0), tail_(0), capacity_(0),
                     buf_size_(0), buffer_(NULL) {}

  size_t head() { return head_; }
  size_t tail() { return tail_; }
  size_t capacity() { return capacity_; }
  void *buffer() { return buffer_; }

  void set_buffer(void *buffer, size_t size) {
    assert((buffer && size) || (!buffer && !size));
    buffer_ = buffer;
    buf_size_ = size;
    capacity_ = buffer_ ? size - 1 : 0;
    head_ = tail_ = 0; 
  }

  bool set_head(size_t head) {
    if (!buffer_ || head >= buf_size_)
      return false;
    head_ = head;
    return true;
  }
  
  bool set_tail(size_t tail) {
    if (!buffer_ || tail >= buf_size_)
      return false;
    tail_ = tail;
    return true;
  }

  bool is_empty() {
    return tail_ == head_;
  }

  bool is_full() {
    return ((tail_ + 1) % buf_size_) == head_;
  }

  size_t data_available() {
    if (tail_ >= head_ ) 
      return tail_ - head_;
    else
      return buf_size_ - head_ + tail_;
  }

  size_t space_available() {
    return capacity_ - data_available();
  }

  size_t contiguous_data_available() {
    if (tail_ >= head_)
      return tail_ - head_;
    else
      return  buf_size_ - head_;
  }

  size_t contiguous_space_available() {
    if (is_full())
      return 0;
    else if (head_ > tail_)
      return head_ - tail_ - 1;
    else {
      size_t contig =  buf_size_ - tail_;
      if (head_ == 0)
        --contig;
      return contig;
    }
  }

  void *head_ptr() {
    return reinterpret_cast<char*>(buffer_) + head_;
  }

  void *tail_ptr() {
    return reinterpret_cast<char*>(buffer_) + tail_;
  }

  size_t advance_head(size_t amt) {
    amt = Min(amt, data_available());
    head_ += amt;
    head_ %= buf_size_;
    return amt;
  }

  size_t advance_tail(size_t amt) {
    amt = Min(amt, space_available());
    tail_ += amt;
    tail_ %= buf_size_;
    return amt;
  }

  // Reads from the circular buffer and into 'data', and
  // The head position is advanced by the number of bytes read.
  // Returns the number of bytes read which may be less than
  // the the desired 'size' if less data was available.
  size_t read(void *data, size_t size) {
#ifdef DEBUG
    int iters = 0;
#endif
    size = Min(size, data_available());
    size_t remaining = size;
    while (remaining > 0) {
      assert(++iters <= 2);  // should never take more than two iterations
      void *read_pos = head_ptr();
      size_t read_amount = Min(remaining, contiguous_data_available());
      assert(read_amount != 0);
      memcpy(data, read_pos, read_amount);
      head_ += read_amount;
      head_ %= buf_size_;
      remaining -= read_amount;
      data = reinterpret_cast<char*>(data) + read_amount;
    }
    return size;
  }

  // Reads from the circular buffer and copies into 'data' 
  // without advancing the head position.
  // Returns the number of bytes read which may be less than
  // the the desired 'size' if less data was available.
  size_t peek(void *data, size_t size) {
    size_t saved = head_;
    size = read(data, size);
    head_ = saved;
    return size;
  }

  // Writes into the circular buffer from 'data', 
  // and advances the tail position.
  // Returns the number of bytes written which may be less than
  // the the desired 'size' if less space was available.
  size_t write(const void *data, size_t size) {
#ifdef DEBUG
    int iters = 0;
#endif
    size = Min(size, space_available());
    size_t remaining = size;
    while (remaining > 0) {
      assert(++iters <= 2);  // should never take more than two iterations
      void *write_pos = tail_ptr();
      size_t write_amount = Min(remaining, contiguous_space_available());
      assert(write_amount != 0);
      memcpy(write_pos, data, write_amount);
      tail_ += write_amount;
      tail_ %= buf_size_;
      remaining -= write_amount;
      data = reinterpret_cast<const char*>(data) + write_amount;
    }
    return size;
  }

 private:
  size_t Min(size_t a, size_t b) {
    return (a < b) ? a : b;
  }

  size_t head_;
  size_t tail_;
  size_t capacity_;  // one less than buf_size_
  size_t buf_size_;
  void *buffer_;
};

#endif
