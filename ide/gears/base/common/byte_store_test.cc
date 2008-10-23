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

#include "gears/base/common/byte_store.h"
#include "gears/base/common/string_utils.h"
#include "gears/blob/blob_interface.h"

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

class TestByteStoreReader : public ByteStore::Reader {
 public:
  TestByteStoreReader(std::string16 *error, uint8 *data, int length)
      : error_(error), pos_(0), data_length_(length), data_(data) {
  }
  int64 ReadFromBuffer(const uint8 *buffer, int64 max_length) {
    int64 len = std::min(static_cast<int64>(data_length_ - pos_), max_length);
    memcpy(&data_[pos_], buffer, static_cast<size_t>(len));
    pos_ += static_cast<int>(len);
    return len;
  }
  bool Test(int64 len) {
    std::string16 *error(error_);
    TEST_ASSERT(pos_ <= data_length_);
    TEST_ASSERT(pos_ == len);
    return true;
  }
 private:
  std::string16 *error_;
  int pos_;
  int data_length_;
  uint8* data_;
};

class TestByteStoreWriter : public ByteStore::Writer {
 public:
  TestByteStoreWriter(std::string16 *error, const char *data, int length)
      : error_(error), pos_(0), data_length_(length), data_(data) {
  }
  int64 WriteToBuffer(uint8 *buffer, int64 max_length) {
    int64 len = std::min(static_cast<int64>(data_length_ - pos_), max_length);
    memcpy(buffer, &data_[pos_], static_cast<size_t>(len));
    pos_ += static_cast<int>(len);
    return len;
  }
  bool Test(int64 len) {
    std::string16 *error(error_);
    TEST_ASSERT(pos_ == data_length_);
    TEST_ASSERT(pos_ == len);
    return true;
  }
 private:
  std::string16 *error_;
  int pos_;
  int data_length_;
  const char* data_;
};

}  // namespace

bool TestByteStore(std::string16 *error) {
  const char data1[] = "bytes";
  std::string16 data2(STRING16(L"string"));
  bool ok = true;
  uint8 buffer[64];
  memset(buffer, 0, sizeof(buffer));
  scoped_refptr<ByteStore> byte_store(new ByteStore);

  // Test an empty ByteStore.
  TEST_ASSERT(byte_store->Length() == 0);
  TEST_ASSERT(0 == byte_store->Read(buffer, 0, sizeof(buffer)));

  // Add data to the ByteStore.
  byte_store->AddData(data1, 5);
  TEST_ASSERT(byte_store->Length() == 5);
  TEST_ASSERT(5 == byte_store->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, data1, 5));
  memset(buffer, 0, sizeof(buffer));

  // Get a blob from the ByteStore.
  scoped_refptr<BlobInterface> blob;
  byte_store->CreateBlob(&blob);
  TEST_ASSERT(blob.get());
  TEST_ASSERT(blob->Length() == 5);
  TEST_ASSERT(5 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, data1, 5));
  memset(buffer, 0, sizeof(buffer));

  // Add a string to the ByteStore.
  byte_store->AddString(data2);
  TEST_ASSERT(byte_store->Length() == 11);
  TEST_ASSERT(11 == byte_store->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "bytesstring", 11));
  memset(buffer, 0, sizeof(buffer));

  // Confirm that our old blob hasn't changed.
  TEST_ASSERT(blob->Length() == 5);
  TEST_ASSERT(5 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, data1, 5));
  memset(buffer, 0, sizeof(buffer));

  // Get a new blob from the ByteStore.
  byte_store->CreateBlob(&blob);
  TEST_ASSERT(blob.get());
  TEST_ASSERT(blob->Length() == 11);
  TEST_ASSERT(11 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "bytesstring", 11));
  memset(buffer, 0, sizeof(buffer));

  // Destroy the ByteStore.
  byte_store.reset();

  // Confirm that our blob hasn't changed.
  TEST_ASSERT(blob.get());
  TEST_ASSERT(blob->Length() == 11);
  TEST_ASSERT(11 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "bytesstring", 11));
  memset(buffer, 0, sizeof(buffer));

  // Create a ByteStore with more than 1MB.
  byte_store.reset(new ByteStore);
  for (int i = 0; i < 1024 * 256; ++i) {
    TEST_ASSERT(byte_store->AddData(data1, 5));
  }
  TEST_ASSERT(1024 * 256 * 5 == byte_store->Length());
  TEST_ASSERT(sizeof(buffer) == byte_store->Read(buffer, 0, sizeof(buffer)));
  for (unsigned i = 0; i < sizeof(buffer) - 5; i += 5) {
    TEST_ASSERT(0 == memcmp(&buffer[i], data1, 5));
  }
  memset(buffer, 0, sizeof(buffer));

  // Write a few more bytes to our large ByteStore.
  TEST_ASSERT(byte_store->AddString(data2));
  TEST_ASSERT(12 == byte_store->Read(buffer, 1024 * 256 * 5 - 6,
                                     sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "sbytesstring", 12));
  TEST_ASSERT(1024 * 256 * 5 + 6 == byte_store->Length());
  memset(buffer, 0, sizeof(buffer));

  // Access a large ByteStore as a blob, after writing more to the ByteStore.
  byte_store->CreateBlob(&blob);
  TEST_ASSERT(blob.get());
  TEST_ASSERT(byte_store->AddString(data2));
  TEST_ASSERT(1024 * 256 * 5 + 6 == blob->Length());
  TEST_ASSERT(12 == blob->Read(buffer, 1024 * 256 * 5 - 6, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "sbytesstring", 12));
  memset(buffer, 0, sizeof(buffer));

  // Test AddDataDirect
  byte_store.reset(new ByteStore);
  std::string large_data;  // large, but less than 1MB
  for (int i = 0; i < 1024 * 128; ++i) {
    large_data.append(data1);
  }
  scoped_array<uint8> large_buffer(new uint8[large_data.size() * 2]);
  TestByteStoreWriter writer(error, large_data.data(), large_data.size());
  // Pick a size larger than data to write, but small enough to stay in buffer.
  int64 len = byte_store->AddDataDirect(&writer, 1024 * 1024);
  ok &= writer.Test(len);
  TEST_ASSERT(len == large_data.size());
  TEST_ASSERT(len == byte_store->Length());
  memset(large_buffer.get(), 0, large_data.size());
  TEST_ASSERT(len == byte_store->Read(large_buffer.get(), 0,
                                      large_data.size()));
  for (unsigned i = 0; i < len - 5; i += 5) {
    TEST_ASSERT(0 == memcmp(&(large_buffer.get())[i], data1, 5));
  }

  // Test ReadDirect
  // Assumes that byte_store already contains 1024 * 128 copies of data1.
  TestByteStoreReader reader(error, large_buffer.get(), large_data.size() * 2);
  memset(large_buffer.get(), 0, large_data.size());
  len = byte_store->ReadDirect(&reader, 0, large_data.size() * 2);
  ok &= reader.Test(len);
  TEST_ASSERT(len == large_data.size());
  TEST_ASSERT(len == byte_store->Length());
  TEST_ASSERT(0 == memcmp(large_buffer.get(), large_data.data(),
                          static_cast<size_t>(len)));

  return ok;
}

#endif  // USING_CCTESTS
