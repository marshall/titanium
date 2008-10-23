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

#ifdef USING_CCTESTS

#include "gears/base/common/common.h"
#include "gears/base/common/memory_buffer.h"
#include "gears/base/common/string_utils.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOCATION __FILE__ ", line " TOSTRING(__LINE__)
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("failed at " LOCATION)); \
    assert(error); \
    if (!error->empty()) *error += STRING16(L", "); \
    *error += STRING16(L"failed at "); \
    std::string16 location; \
    UTF8ToString16(LOCATION, &location); \
    *error += location; \
    return false; \
  } \
}

namespace {
// This value is the initial capacity allocated.
// This should match the same value given in memory_buffer.cc
const MemoryBuffer::size_type gInitialCapacity = 16;
}  // namespace

bool TestMemoryBuffer(std::string16 *error) {
  bool ok = true;

  // Test an empty MemoryBuffer.
  MemoryBuffer buf;
  TEST_ASSERT(buf.Empty());
  TEST_ASSERT(buf.Size() == 0);
  TEST_ASSERT(buf.Capacity() == 0);

  // Reserve space for 1 byte.
  buf.Reserve(1);
  TEST_ASSERT(buf.Empty());
  TEST_ASSERT(buf.Size() == 0);
  TEST_ASSERT(buf.Capacity() == gInitialCapacity);

  // Reserve additional bytes within the initial capacity.
  buf.Reserve(gInitialCapacity);
  TEST_ASSERT(buf.Empty());
  TEST_ASSERT(buf.Size() == 0);
  TEST_ASSERT(buf.Capacity() == gInitialCapacity);

  // Resize the buffer within the current capacity.
  buf.Resize(gInitialCapacity);
  TEST_ASSERT(!buf.Empty());
  TEST_ASSERT(buf.Size() == gInitialCapacity);
  TEST_ASSERT(buf.Capacity() == gInitialCapacity);

  // Resize the buffer beyond the current capacity.
  int64 new_size(gInitialCapacity * 8 + 1);
  buf.Resize(new_size);
  TEST_ASSERT(!buf.Empty());
  TEST_ASSERT(buf.Size() == new_size);
  TEST_ASSERT(buf.Capacity() == gInitialCapacity * 16);

  // Clear the buffer.
  buf.Clear();
  TEST_ASSERT(buf.Empty());
  TEST_ASSERT(buf.Size() == 0);
  TEST_ASSERT(buf.Capacity() == gInitialCapacity * 16);

  // Add data to the buffer.
  const char data1[] = "bufferstring";
  buf.Append(reinterpret_cast<const uint8*>(data1), sizeof(data1));
  TEST_ASSERT(!buf.Empty());
  TEST_ASSERT(buf.Size() == sizeof(data1));
  TEST_ASSERT(buf.Capacity() == gInitialCapacity * 16);
  TEST_ASSERT(0 == memcmp(buf.Data(0), data1, sizeof(data1)));

  // Create a MemoryBuffer of a given size.
  MemoryBuffer buf2(gInitialCapacity * 2 + 1);
  TEST_ASSERT(!buf2.Empty());
  TEST_ASSERT(buf2.Size() == gInitialCapacity * 2 + 1);
  TEST_ASSERT(buf2.Capacity() == gInitialCapacity * 4);

  // Swap two MemoryBuffers.
  buf2.Swap(buf);
  TEST_ASSERT(!buf.Empty());
  TEST_ASSERT(buf.Size() == gInitialCapacity * 2 + 1);
  TEST_ASSERT(buf.Capacity() == gInitialCapacity * 4);
  TEST_ASSERT(!buf2.Empty());
  TEST_ASSERT(buf2.Size() == sizeof(data1));
  TEST_ASSERT(buf2.Capacity() == gInitialCapacity * 16);
  TEST_ASSERT(0 == memcmp(buf2.Data(0), data1, sizeof(data1)));

  return ok;
}

#endif  // USING_CCTESTS
