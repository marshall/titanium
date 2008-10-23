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

#include <cstring>
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/blob/blob_builder.h"
#include "gears/blob/blob_utils.h"
#include "gears/blob/buffer_blob.h"
#if DEBUG
#include "gears/blob/fail_blob.h"
#endif  // DEBUG
#include "gears/blob/file_blob.h"
#include "gears/blob/join_blob.h"
#include "gears/blob/slice_blob.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

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

class BlobReader : public BlobInterface::Reader {
 public:
  virtual int64 ReadFromBuffer(const uint8 *buffer, int64 max_bytes) {
    // Always pretends that it was able to successfully read everything.
    return max_bytes;
  }
};

}  // namespace

static bool TestBufferBlob(std::string16 *error) {
  std::vector<DataElement> data_elements;
  int64 num_bytes = 0;
  uint8 buffer[64];
  memset(buffer, 0, sizeof(buffer));
  scoped_refptr<BufferBlob> blob;

  // Initialize a blob with an empty vector.
  std::vector<uint8> vec;
  blob.reset(new BufferBlob(&vec));
  TEST_ASSERT(blob->Length() == 0);
  num_bytes = blob->Read(buffer, 0, 64);
  TEST_ASSERT(num_bytes == 0);
  TEST_ASSERT(buffer[0] == 0);
  TEST_ASSERT(blob->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 0);

  // Initialize a blob with a typical vector.
  vec.push_back('a');
  vec.push_back('b');
  vec.push_back('c');
  blob.reset(new BufferBlob(&vec));
  TEST_ASSERT(vec.empty());
  TEST_ASSERT(blob->Length() == 3);
  num_bytes = blob->Read(buffer, 0, 64);
  TEST_ASSERT(num_bytes == 3);
  TEST_ASSERT(buffer[0] == 'a');
  TEST_ASSERT(buffer[1] == 'b');
  TEST_ASSERT(buffer[2] == 'c');
  TEST_ASSERT(buffer[3] == 0);
  TEST_ASSERT(blob->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 1);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[0].bytes_length() == num_bytes);
  TEST_ASSERT(data_elements[0].GetContentLength() ==
              static_cast<size_t>(num_bytes));
  TEST_ASSERT(data_elements[0].bytes());
  TEST_ASSERT(memcmp(data_elements[0].bytes(), buffer, 3) == 0);

  // Initialize buffer blob from an array.
  const uint8 data[] = "abcdefghijklmnopqrstuvwxyz";
  blob.reset(new BufferBlob(data, 9));
  TEST_ASSERT(blob->Length() == 9);

  // Ensure that Read obeys the max_length parameter.
  memset(buffer, 1, sizeof(buffer));
  num_bytes = blob->Read(buffer, 0, 3);
  TEST_ASSERT(num_bytes == 3);
  TEST_ASSERT(buffer[0] == 'a');
  TEST_ASSERT(buffer[1] == 'b');
  TEST_ASSERT(buffer[2] == 'c');
  TEST_ASSERT(buffer[3] == 1);

  // Read with a non-zero starting offset.
  memset(buffer, 1, sizeof(buffer));
  num_bytes = blob->Read(buffer, 3, 3);
  TEST_ASSERT(num_bytes == 3);
  TEST_ASSERT(buffer[0] == 'd');
  TEST_ASSERT(buffer[1] == 'e');
  TEST_ASSERT(buffer[2] == 'f');
  TEST_ASSERT(buffer[3] == 1);

  // Read with an offset beyond the end of the blob.
  memset(buffer, 1, sizeof(buffer));
  num_bytes = blob->Read(buffer, 20, 3);
  TEST_ASSERT(num_bytes == 0);
  TEST_ASSERT(buffer[0] == 1);

  return true;
}

static bool TestFileBlob(std::string16 *error) {
  std::vector<DataElement> data_elements;
  uint8 buffer[64];
  const uint8 vec_contents[] = "abcdef";
  scoped_ptr<File> file(File::CreateNewTempFile());
  file->Write(vec_contents, sizeof(vec_contents) - 1);
  file->Flush();

  // Test FileBlob construction given a File*.
  std::string16 temp_filepath = file->GetFilePath();
  scoped_refptr<FileBlob> blob(new FileBlob(file.release()));
  TEST_ASSERT(blob->Length() == 6);
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(6 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "abcdef", 7) == 0);
  if (temp_filepath.empty()) {
    // nameless temp files (posix) are not supported
    TEST_ASSERT(!blob->GetDataElements(&data_elements));
  }

  // Create a FileBlob from a nonexistent file.
  blob.reset(new FileBlob(STRING16(L"/A/B/C/Z/doesnotexist")));
  TEST_ASSERT(blob->Length() == -1);
  TEST_ASSERT(-1 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(!blob->GetDataElements(&data_elements));

  // Test an empty FileBlob.
  std::string16 temp_dir;
  File::CreateNewTempDirectory(&temp_dir);
  std::string16 filepath(temp_dir + kPathSeparator +
                         STRING16(L"TestFileBlob.ext"));
  File::CreateNewFile(filepath.c_str());
  blob.reset(new FileBlob(filepath));
  TEST_ASSERT(blob->Length() == 0);
  TEST_ASSERT(0 == blob->Read(buffer, 0, sizeof(buffer)));
  // This will close filepath, which is necessary before writing to it on win32.
  blob.reset();

  // Test FileBlob construction given a file name.
  File::WriteBytesToFile(filepath.c_str(), vec_contents, 6);
  blob.reset(new FileBlob(filepath));
  TEST_ASSERT(blob->Length() == 6);
  TEST_ASSERT(6 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "abcdef", 7));
  TEST_ASSERT(blob->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 1);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_FILE);
  TEST_ASSERT(data_elements[0].file_path() == blob->GetFilePath());
  TEST_ASSERT(data_elements[0].GetContentLength() == 6);

  // Cleanup
  File::DeleteRecursively(temp_dir.c_str());

  return true;
}


static bool TestSliceBlob(std::string16 *error) {
  std::vector<DataElement> data_elements;
  uint8 buffer[64];

  // Create a BufferBlob and verify its contents.
  const uint8 vec_contents[] = "abcdef";
  std::vector<uint8> vec(vec_contents, vec_contents + sizeof(vec_contents) - 1);
  scoped_refptr<BufferBlob> blob_buffer(new BufferBlob(&vec));
  TEST_ASSERT(1 == blob_buffer->GetRef());

  TEST_ASSERT(blob_buffer->Length() == 6);
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(6 == blob_buffer->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "abcdef", 7) == 0);

  // Slice the BufferBlob and verify the slice.  abcdef -> cde
  scoped_refptr<BlobInterface> blob1(new SliceBlob(blob_buffer.get(), 2, 3));
  TEST_ASSERT(2 == blob_buffer->GetRef());
  TEST_ASSERT(1 == blob1->GetRef());
  TEST_ASSERT(blob1->Length() == 3);
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(3 == blob1->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "cde", 4) == 0);
  TEST_ASSERT(blob1->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 1);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[0].bytes_length() == blob1->Length());
  TEST_ASSERT(data_elements[0].GetContentLength() ==
              static_cast<size_t>(blob1->Length()));
  TEST_ASSERT(data_elements[0].bytes());
  TEST_ASSERT(memcmp(data_elements[0].bytes(), "cde", 3) == 0);

  // A positive offset combines with the slice offset.
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(1 == blob1->Read(buffer, 2, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "e", 2) == 0);

  // A negative offset fails (even though the combined offsets are valid).
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(-1 == blob1->Read(buffer, -1, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "", 1) == 0);

  // A too-large offset returns no bytes.
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(0 == blob1->Read(buffer, 4, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "", 1) == 0);

  // Slice the existing slice.  cde -> d
  scoped_refptr<BlobInterface> blob2(new SliceBlob(blob1.get(), 1, 1));
  TEST_ASSERT(2 == blob_buffer->GetRef());
  TEST_ASSERT(2 == blob1->GetRef());
  TEST_ASSERT(1 == blob2->GetRef());
  TEST_ASSERT(blob2->Length() == 1);
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(1 == blob2->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "d", 2) == 0);
  data_elements.clear();
  TEST_ASSERT(blob2->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 1);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[0].bytes_length() == blob2->Length());
  TEST_ASSERT(data_elements[0].GetContentLength() ==
              static_cast<size_t>(blob2->Length()));
  TEST_ASSERT(data_elements[0].bytes());
  TEST_ASSERT(memcmp(data_elements[0].bytes(), "d", 1) == 0);

  // Verify that the original slice is unaffected.
  memset(buffer, 0, sizeof(buffer));
  TEST_ASSERT(3 == blob1->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(memcmp(buffer, "cde", 4) == 0);

  // The caller is responsible for ensuring that offset and length are
  // non-negative.
  // blob1 = new SliceBlob(blob_buffer, -1, 1));
  // blob1 = new SliceBlob(blob_buffer, 1, -1));

  return true;
}

static bool TestJoinBlob(std::string16 *error) {
  std::vector<DataElement> data_elements;
  uint8 buffer[64];
  BlobBuilder builder;
  scoped_refptr<BlobInterface> blob;

  // JoinBlob with no content.
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == 0);
  TEST_ASSERT(0 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(blob->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 0);

  // JoinBlob containing one BufferBlob.
  const char data1[] = "nom";
  scoped_refptr<BlobInterface> blob1(new BufferBlob(data1, strlen(data1)));
  builder.AddBlob(blob1.get());
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob1.get() == blob.get());  // should return the single blob
  TEST_ASSERT(blob->Length() == strlen(data1));
  TEST_ASSERT(3 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, data1, strlen(data1)));

  // JoinBlob containing one BufferBlob, one string, and one data buffer.
  builder.AddBlob(blob1.get());
  builder.AddString(STRING16(L"burp"));
  builder.AddData("ahh", 3);
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == 10);
  TEST_ASSERT(10 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "nomburpahh", 10));
  TEST_ASSERT(0 == blob->Read(buffer, 12, sizeof(buffer)));
  TEST_ASSERT(3 == blob->Read(buffer, 1, 3));
  TEST_ASSERT(0 == memcmp(buffer, "omb", 3));
  data_elements.clear();
  TEST_ASSERT(blob->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 2);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[0].bytes_length() == blob1->Length());
  TEST_ASSERT(data_elements[1].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[1].bytes_length() == 7);


  // JoinBlob containing data, blob, then more data.
  builder.AddData("foo", 3);
  builder.AddBlob(blob1.get());
  builder.AddData("barr", 4);
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == 10);
  TEST_ASSERT(10 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "foonombarr", 10));

  // JoinBlob containing EmptyBlobs and other empty data.
  builder.AddBlob(new EmptyBlob);
  builder.AddBlob(blob1.get());
  builder.AddString(STRING16(L""));
  builder.AddData("", 0);
  builder.AddBlob(new EmptyBlob);
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == strlen(data1));
  TEST_ASSERT(3 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, data1, strlen(data1)));
  data_elements.clear();
  TEST_ASSERT(blob->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 1);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[0].bytes_length() == blob->Length());

  // JoinBlob containing a string with non-ascii data.
  builder.AddString(STRING16(L"\xFFFF\x0080"));
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == 5);
  TEST_ASSERT(5 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "\xef\xbf\xbf\xc2\x80", 5));

  // JoinBlob containing the same BufferBlob multiple times.
  for (unsigned i = 0; i < 3; ++i) {
    builder.AddBlob(blob1.get());
  }
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == 9);
  TEST_ASSERT(9 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "nomnomnom", 9));
  data_elements.clear();
  TEST_ASSERT(blob->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 3);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[0].bytes_length() == blob1->Length());
  TEST_ASSERT(data_elements[1].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[1].bytes_length() == blob1->Length());
  TEST_ASSERT(data_elements[1].bytes() == data_elements[0].bytes());
  TEST_ASSERT(data_elements[2].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[2].bytes_length() == blob1->Length());
  TEST_ASSERT(data_elements[2].bytes() == data_elements[0].bytes());

  // SliceBlob containing a JoinBlob.
  // NOTE - assumes that blob currently contains "nomnomnom".
  scoped_refptr<BlobInterface> blob2 = new SliceBlob(blob.get(), 2, 5);
  TEST_ASSERT(blob2->Length() == 5);
  TEST_ASSERT(5 == blob2->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "mnomn", 5));
  data_elements.clear();
  TEST_ASSERT(blob2->GetDataElements(&data_elements));
  TEST_ASSERT(data_elements.size() == 3);
  TEST_ASSERT(data_elements[0].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[0].bytes_length() == 1);
  TEST_ASSERT(data_elements[0].bytes() == data_elements[1].bytes() + 2);
  TEST_ASSERT(data_elements[1].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[1].bytes_length() == 3);
  TEST_ASSERT(data_elements[2].type() == DataElement::TYPE_BYTES);
  TEST_ASSERT(data_elements[2].bytes_length() == 1);
  TEST_ASSERT(data_elements[2].bytes() == data_elements[1].bytes());

  // JoinBlob containing a SliceBlob.
  builder.AddBlob(blob1.get());
  builder.AddBlob(blob2.get());
  builder.AddString(STRING16(L"burp"));
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == 12);
  TEST_ASSERT(12 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "nommnomnburp", 12));

  // Create JoinBlob from strings surrounding a FileBlob.
  const uint8 vec_contents[] = "abcdef";
  scoped_ptr<File> file(File::CreateNewTempFile());
  file->Write(vec_contents, sizeof(vec_contents) - 1);
  file->Flush();
  scoped_refptr<FileBlob> blob3(new FileBlob(file.release()));
  builder.AddString(STRING16(L"one"));
  builder.AddString(STRING16(L"two"));
  builder.AddBlob(blob3.get());
  builder.AddString(STRING16(L"three"));
  builder.CreateBlob(&blob);
  builder.Reset();
  TEST_ASSERT(blob->Length() == 17);
  TEST_ASSERT(17 == blob->Read(buffer, 0, sizeof(buffer)));
  TEST_ASSERT(0 == memcmp(buffer, "onetwoabcdefthree", 17));

  return true;
}

static bool TestBlobDataElements(std::string16 *error) {
  // Write the test data to a newly created file
  const uint8 kData[] = "0123456789";
  const int kDataLen = 10;
  std::string16 temp_dir;
  File::CreateNewTempDirectory(&temp_dir);
  std::string16 file_path(temp_dir + kPathSeparator +
                          STRING16(L"file_blob.ext"));
  File::CreateNewFile(file_path.c_str());
  File::WriteBytesToFile(file_path.c_str(), kData, kDataLen);

  // Load a buffer and file blob with our test data
  scoped_refptr<BufferBlob> buffer_blob(new BufferBlob(kData, kDataLen));
  scoped_refptr<FileBlob> file_blob(new FileBlob(file_path));

  // Take one byte slices of each and join them
  std::vector< scoped_refptr<BlobInterface> > buffer_slices;
  std::vector< scoped_refptr<BlobInterface> >file_slices;
  for (int i = 0; i < kDataLen; ++i) {
    scoped_refptr<SliceBlob>
        buffer_slice(new SliceBlob(buffer_blob.get(), i, 1));
    scoped_refptr<SliceBlob> file_slice(new SliceBlob(file_blob.get(), i, 1));
    buffer_slices.push_back(buffer_slice);
    file_slices.push_back(file_slice);
  }
  scoped_refptr<JoinBlob> joined_buffers(new JoinBlob(buffer_slices));
  scoped_refptr<JoinBlob> joined_files(new JoinBlob(file_slices));

  // Retrieve the elements and examine them
  std::vector<DataElement> buffer_elements;
  std::vector<DataElement> file_elements;
  TEST_ASSERT(joined_buffers->GetDataElements(&buffer_elements));
  TEST_ASSERT(joined_files->GetDataElements(&file_elements));
  TEST_ASSERT(buffer_elements.size() == static_cast<size_t>(kDataLen));
  TEST_ASSERT(file_elements.size() == static_cast<size_t>(kDataLen));
  for (size_t i = 0; i < static_cast<size_t>(kDataLen); ++i) {
    TEST_ASSERT(buffer_elements[i].type() == DataElement::TYPE_BYTES);
    TEST_ASSERT(buffer_elements[i].GetContentLength() == 1);
    TEST_ASSERT(buffer_elements[i].bytes()[0] == kData[i]);
    TEST_ASSERT(buffer_elements[i].bytes_length() == 1);
    TEST_ASSERT(file_elements[i].type() == DataElement::TYPE_FILE);
    TEST_ASSERT(file_elements[i].GetContentLength() == 1);
    TEST_ASSERT(file_elements[i].file_path() == file_path);
    TEST_ASSERT(file_elements[i].file_range_offset() == i);
    TEST_ASSERT(
        file_elements[i].file_range_length() == 1 ||
        ((i == 9) && (file_elements[i].file_range_length() == kuint64max)));
  }

  // Cleanup the temp file
  File::DeleteRecursively(temp_dir.c_str());
  return true;
}

static bool TestBlobUtils(std::string16 *error) {
  static const int kBufferSize = 128;
  uint8 buffer[kBufferSize];

  const char data1[] = "plain ascii text";
  scoped_refptr<BlobInterface> blob1(new BufferBlob(data1, strlen(data1)));
  // Test BlobToVector on plain ascii text.
  std::vector<uint8> vector1;
  TEST_ASSERT(BlobToVector(blob1.get(), &vector1));
  TEST_ASSERT(static_cast<int64>(vector1.size()) == blob1->Length());
  TEST_ASSERT(blob1->Length() == blob1->Read(buffer, 0, kBufferSize));
  TEST_ASSERT(0 == memcmp(buffer, &vector1[0], vector1.size()));
  // Test BlobToString on plain ascii text.
  std::string string1;
  TEST_ASSERT(BlobToString(blob1.get(), &string1));
  TEST_ASSERT(static_cast<int64>(string1.size()) == blob1->Length());
  TEST_ASSERT(blob1->Length() == blob1->Read(buffer, 0, kBufferSize));
  TEST_ASSERT(0 == memcmp(buffer, &string1[0], string1.size()));
  // Test BlobToString16 on plain ascii text.
  std::string16 utf16_1;
  std::string16 expected1;
  TEST_ASSERT(UTF8ToString16(data1, &expected1));
  TEST_ASSERT(BlobToString16(blob1.get(), STRING16(L"UTF-8"), &utf16_1));
  TEST_ASSERT(utf16_1 == expected1);

  // Test BlobToString16 with some multi-byte utf8 characters.
  const char data2[] = "some multi-byte utf8: \xCF\x8F";
  scoped_refptr<BlobInterface> blob2(new BufferBlob(data2, strlen(data2)));
  std::string16 utf16_2;
  std::string16 expected2;
  TEST_ASSERT(UTF8ToString16(data2, &expected2));
  TEST_ASSERT(BlobToString16(blob2.get(), STRING16(L"UTF-8"), &utf16_2));
  TEST_ASSERT(utf16_2 == expected2);

  // Test BlobToString16 on invalid UTF-8 data.
  const char data3[] = "some invalid utf8: \xFF\xFF";
  scoped_refptr<BlobInterface> blob3(new BufferBlob(data3, strlen(data3)));
  std::string16 utf16_3;
  std::string16 expected3;
  TEST_ASSERT(!UTF8ToString16(data3, &expected3));
  TEST_ASSERT(!BlobToString16(blob3.get(), STRING16(L"UTF-8"), &utf16_3));

  // Test BlobToString16 on a UTF-8 string ending in an incomplete character.
  const char data4[] = "Last char should be three bytes: \xEF\x8F";
  scoped_refptr<BlobInterface> blob4(new BufferBlob(data4, strlen(data4)));
  std::string16 utf16_4;
  std::string16 expected4;
  TEST_ASSERT(!UTF8ToString16(data4, &expected4));
  TEST_ASSERT(!BlobToString16(blob4.get(), STRING16(L"UTF-8"), &utf16_4));

  // Test BlobToString16 on a JoinBlob.
  const char data5_1[] = "this";
  const char data5_2[] = "is";
  const char data5_3[] = "a";
  const char data5_4[] = "test";
  JoinBlob::List blob_list5;
  blob_list5.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data5_1, strlen(data5_1))));
  blob_list5.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data5_2, strlen(data5_2))));
  blob_list5.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data5_3, strlen(data5_3))));
  blob_list5.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data5_4, strlen(data5_4))));
  scoped_refptr<BlobInterface> blob5(new JoinBlob(blob_list5));
  std::string text5;
  text5.append(data5_1);
  text5.append(data5_2);
  text5.append(data5_3);
  text5.append(data5_4);
  std::string16 expected5;
  std::string16 utf16_5;
  TEST_ASSERT(UTF8ToString16(text5.c_str(), &expected5));
  TEST_ASSERT(BlobToString16(blob5.get(), STRING16(L"UTF-8"), &utf16_5));
  TEST_ASSERT(utf16_5 == expected5);

  // Test BlobToString16 on a JoinBlob with split characters.
  const char data6_1[] = "Two byte character: \xCF";
  const char data6_2[] = "\x8F.  A 4-byte character split twice: \xF4";
  const char data6_3[] = "\x8F";
  const char data6_4[] = "\xAF\xAF.  See if that works!";
  JoinBlob::List blob_list6;
  blob_list6.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data6_1, strlen(data6_1))));
  blob_list6.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data6_2, strlen(data6_2))));
  blob_list6.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data6_3, strlen(data6_3))));
  blob_list6.push_back(scoped_refptr<BlobInterface>(
                           new BufferBlob(data6_4, strlen(data6_4))));
  scoped_refptr<BlobInterface> blob6(new JoinBlob(blob_list6));
  std::string text6;
  text6.append(data6_1);
  text6.append(data6_2);
  text6.append(data6_3);
  text6.append(data6_4);
  std::string16 expected6;
  std::string16 utf16_6;
  TEST_ASSERT(UTF8ToString16(text6.c_str(), &expected6));
  TEST_ASSERT(BlobToString16(blob6.get(), STRING16(L"UTF-8"), &utf16_6));
  TEST_ASSERT(utf16_6 == expected6);

  return true;
}

#if DEBUG
static bool TestFailBlob(std::string16 *error) {
  const int64 kLength(512);
  uint8 buffer[kLength];
  scoped_refptr<BlobInterface> blob;

  // Test FailBlob::Read()
  blob.reset(new FailBlob(kLength));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(511 == blob->Read(buffer, 0, 511));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(-1 == blob->Read(buffer, 511, 1));
  TEST_ASSERT(-1 == blob->Length());
  blob.reset(new FailBlob(kLength));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(-1 == blob->Read(buffer, 0, kLength));
  TEST_ASSERT(-1 == blob->Length());
  blob.reset(new FailBlob(kLength));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(-1 == blob->Read(buffer, kLength * 2, kLength));
  TEST_ASSERT(-1 == blob->Length());

  // Test FailBlob::ReadDirect()
  BlobReader reader;
  blob.reset(new FailBlob(kLength));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(511 == blob->ReadDirect(&reader, 0, 511));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(-1 == blob->ReadDirect(&reader, 511, 1));
  TEST_ASSERT(-1 == blob->Length());
  blob.reset(new FailBlob(kLength));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(-1 == blob->ReadDirect(&reader, 0, kLength));
  TEST_ASSERT(-1 == blob->Length());
  blob.reset(new FailBlob(kLength));
  TEST_ASSERT(kLength == blob->Length());
  TEST_ASSERT(-1 == blob->ReadDirect(&reader, kLength * 2, kLength));
  TEST_ASSERT(-1 == blob->Length());

  return true;
}
#endif  // DEBUG

bool TestBlob(std::string16 *error) {
  bool ok = true;
  ok &= TestBufferBlob(error);
  ok &= TestFileBlob(error);
  ok &= TestJoinBlob(error);
  ok &= TestSliceBlob(error);
  ok &= TestBlobDataElements(error);
  ok &= TestBlobUtils(error);
#if DEBUG
  ok &= TestFailBlob(error);
#endif  // DEBUG
  return ok;
}

#endif  // USING_CCTESTS
