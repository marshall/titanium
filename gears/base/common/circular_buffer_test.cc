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

#include "gears/base/common/circular_buffer.h"
#include "gears/base/common/common.h"

#ifdef USING_CCTESTS

bool TestCircularBuffer(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestCircularBuffer - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestCircularBuffer - failed. "); \
    return false; \
  } \
}

  CircularBuffer circular;
  const size_t kCapacity = 4;
  const size_t kBufSize = kCapacity + 1;
  char buf_in[kCapacity];
  char buf_out[kCapacity];
  char buf_working[kBufSize];
 
  for (size_t i = 0; i < kCapacity; ++i) {
    buf_in[i] = static_cast<char>(i);
  }

  circular.set_buffer(buf_working, kBufSize);
  TEST_ASSERT(circular.is_empty());
  TEST_ASSERT(!circular.is_full());
  TEST_ASSERT(circular.data_available() == 0);
  TEST_ASSERT(circular.space_available() == kCapacity);
  TEST_ASSERT(circular.head() == 0);
  TEST_ASSERT(circular.tail() == 0);
  TEST_ASSERT(circular.peek(buf_out, kCapacity) == 0);
  TEST_ASSERT(circular.read(buf_out, kCapacity) == 0);
  TEST_ASSERT(circular.advance_head(1) == 0);
  TEST_ASSERT(circular.head() == 0);
  TEST_ASSERT(circular.tail() == 0);

  TEST_ASSERT(circular.write(buf_in, kCapacity * 2) == kCapacity);
  TEST_ASSERT(circular.data_available() == kCapacity);
  TEST_ASSERT(circular.advance_tail(1) == 0);
  TEST_ASSERT(circular.read(buf_out, kCapacity * 2) == kCapacity);
  TEST_ASSERT(circular.space_available() == kCapacity);
  TEST_ASSERT(memcmp(buf_in, buf_out, kCapacity) == 0);

  for (size_t position = 0; position < kBufSize; ++position) {
    // Reset to an empty buffer with head and tail at this position
    memset(buf_working, 0, sizeof(buf_working));
    memset(buf_out, 0xff, sizeof(buf_out));
    circular.set_head(position);
    circular.set_tail(position);

    // Check initial conditions
    TEST_ASSERT(circular.is_empty());
    TEST_ASSERT(!circular.is_full());
    TEST_ASSERT(circular.data_available() == 0);
    TEST_ASSERT(circular.space_available() == kCapacity);

    // Write all at once
    TEST_ASSERT(circular.write(buf_in, kCapacity) == kCapacity);
    TEST_ASSERT(!circular.is_empty());
    TEST_ASSERT(circular.is_full());
    TEST_ASSERT(circular.data_available() == kCapacity);
    TEST_ASSERT(circular.space_available() == 0);

    // Read all at once
    TEST_ASSERT(circular.read(buf_out, kCapacity) == kCapacity);
    TEST_ASSERT(circular.is_empty());
    TEST_ASSERT(!circular.is_full());
    TEST_ASSERT(circular.data_available() == 0);
    TEST_ASSERT(circular.space_available() == kCapacity);
    TEST_ASSERT(memcmp(buf_in, buf_out, kCapacity) == 0);

    // Reset to empty at this position again
    memset(buf_working, 0, sizeof(buf_working));
    memset(buf_out, 0xff, sizeof(buf_out));
    circular.set_head(position);
    circular.set_tail(position);

    // Write one byte at a time
    TEST_ASSERT(circular.write(&buf_in[0], 1) == 1);
    for (size_t i = 1; i < kCapacity; ++i) {
      TEST_ASSERT(!circular.is_empty());
      TEST_ASSERT(circular.data_available() == i);
      TEST_ASSERT(circular.space_available() == kCapacity - i);
      TEST_ASSERT(circular.write(&buf_in[i], 1) == 1);
    }
    TEST_ASSERT(!circular.is_empty());
    TEST_ASSERT(circular.is_full());
    TEST_ASSERT(circular.data_available() == kCapacity);
    TEST_ASSERT(circular.space_available() == 0);

    // Read one byte at a time
    TEST_ASSERT(circular.read(&buf_out[0], 1) == 1);
    for (size_t i = 1; i < kCapacity; ++i) {
      TEST_ASSERT(!circular.is_full());
      TEST_ASSERT(circular.data_available() == kCapacity - i);
      TEST_ASSERT(circular.space_available() == i);
      TEST_ASSERT(circular.read(&buf_out[i], 1) == 1);
    }
    TEST_ASSERT(circular.is_empty());
    TEST_ASSERT(!circular.is_full());
    TEST_ASSERT(circular.data_available() == 0);
    TEST_ASSERT(circular.space_available() == kCapacity);
    TEST_ASSERT(memcmp(buf_in, buf_out, kCapacity) == 0);
  }

  return true;
}

#endif
