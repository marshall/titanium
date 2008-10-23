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

#include "gears/base/common/serialization.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

const int kTestBytesSize = 8;
class SerializationTest : public Serializable {
 public:
  SerializationTest() : test_bool_(false), test_int_(0) {
    memset(test_bytes_, 0, kTestBytesSize);
  }
  SerializationTest(bool test_bool, int test_int, const char16 *test_string,
                    const char *test_bytes)
      : test_bool_(test_bool), test_int_(test_int), test_string_(test_string) {
    memcpy(test_bytes_, test_bytes, kTestBytesSize);
  }

  virtual SerializableClassId GetSerializableClassId() const {
      return SERIALIZABLE_TEST_OBJECT;
  }

  static Serializable *SerializableFactoryMethod() {
      return new SerializationTest;
  }

  virtual bool Deserialize(Deserializer *in) {
    return in->ReadBool(&test_bool_) &&
        in->ReadInt(&test_int_) &&
        in->ReadString(&test_string_) &&
        in->ReadBytes(test_bytes_, kTestBytesSize);
  }

  virtual bool Serialize(Serializer *out) const {
    out->WriteBool(test_bool_);
    out->WriteInt(test_int_);
    out->WriteString(test_string_.c_str());
    out->WriteBytes(test_bytes_, kTestBytesSize);

    return true;
  }

  bool operator==(SerializationTest &other) {
    return test_bool_ == test_bool_ &&
        test_int_ == other.test_int_ &&
        test_string_ == other.test_string_ &&
        !memcmp(test_bytes_, other.test_bytes_, kTestBytesSize);
  }
  bool operator!=(SerializationTest &other) {
    return !(*this == other);
  }
 private:
  bool test_bool_;
  int test_int_;
  std::string16 test_string_;
  uint8 test_bytes_[kTestBytesSize];
};

bool TestSerialization(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestSerialization - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestSerialization - failed. "); \
    return false; \
  } \
}
  Serializable::RegisterClass(SERIALIZABLE_TEST_OBJECT,
                              SerializationTest::SerializableFactoryMethod);

  const char test_bytes_0[] = {1,2,3,4,5,6,7,8};
  const char test_bytes_1[] = {0xde,0xad,0xbe,0xef,
                               0x42,0x42,0x42,0x42};

  SerializationTest test0(true, 42, STRING16(L"test0"), test_bytes_0);
  SerializationTest test1(false, -13, STRING16(L"test1"), test_bytes_1);

  std::vector<uint8> buffer0;
  Serializer serializer0(&buffer0);

  serializer0.WriteObject(&test0);

  Deserializer deserializer0(&buffer0.at(0), buffer0.size());
  Serializable *output = NULL;
  TEST_ASSERT(deserializer0.CreateAndReadObject(&output));
  scoped_ptr<Serializable> output_ptr(output);
  TEST_ASSERT(output);

  SerializationTest *test_object = static_cast<SerializationTest*>(output);
  TEST_ASSERT(*test_object == test0);
  TEST_ASSERT(*test_object != test1);

  output_ptr.reset();
  output = NULL;

  std::vector<uint8> buffer1;
  Serializer serializer1(&buffer1);

  serializer1.WriteObject(&test1);

  Deserializer deserializer1(&buffer1.at(0), buffer1.size());
  TEST_ASSERT(deserializer1.CreateAndReadObject(&output));
  output_ptr.reset(output);
  TEST_ASSERT(output);

  test_object = static_cast<SerializationTest*>(output);
  TEST_ASSERT(*test_object != test0);
  TEST_ASSERT(*test_object == test1);

  return true;
}
#endif
