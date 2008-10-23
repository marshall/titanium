// Copyright 2008 Google Inc. All Rights Reserved.
// Author: bgarcia@google.com (Brad Garcia)
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

#include <algorithm>
#include "gears/base/common/string_utils.h"
#include "gears/blob/blob_input_stream_ff.h"
#include "gears/blob/buffer_blob.h"
#if DEBUG
#include "gears/blob/fail_blob.h"
#endif  // DEBUG

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

// This counts how many times any consumer was called.
int consumer_count = 0;

// This sets the limit for ConsumerFails().
int consumer_fail = 0;

// Size of the buffer used by ReadSegments.
const int rs_buffer_size(1024 * 1024);

// Some data we can use to initialize our blobs.
std::vector<uint8>* data(0);

// Amount of data that we create (8 MB).
const std::vector<uint8>::size_type data_size(1024 * 1024 * 8);

// Given an expected number of bytes, compute the expected count.
int ExpectedCount(int bytes) {
  return ((bytes - 1) / rs_buffer_size) + 1;
}

// This consumer accepts up to 1MB of input at a time.
NS_IMETHODIMP ConsumerIdeal(nsIInputStream *aInStream,
                            void *aClosure,
                            const char *aFromSegment,
                            PRUint32 aToOffset,
                            PRUint32 aCount,
                            PRUint32 *aWriteCount) {
  ++consumer_count;
  *aWriteCount = std::min(aCount, static_cast<PRUint32>(1024 * 1024));
  return NS_OK;
}

// This consumer accepts all input up to 1MB at a time
// until it is called for the Nth time.
NS_IMETHODIMP ConsumerFails(nsIInputStream *aInStream,
                            void *aClosure,
                            const char *aFromSegment,
                            PRUint32 aToOffset,
                            PRUint32 aCount,
                            PRUint32 *aWriteCount) {
  ++consumer_count;
  if (consumer_count >= consumer_fail) {
    return NS_ERROR_FAILURE;
  }
  *aWriteCount = std::min(aCount, static_cast<PRUint32>(1024 * 1024));
  return NS_OK;
}

// This consumer sets aWriteCount to 0 and returns NS_OK
// NOTE - This behavior is documented as having undefined meaning.
//        Our implementation of ReadSegments should assert.
//        So this cannot be used in a default build of the tests.
NS_IMETHODIMP ConsumerReadsZero(nsIInputStream *aInStream,
                                void *aClosure,
                                const char *aFromSegment,
                                PRUint32 aToOffset,
                                PRUint32 aCount,
                                PRUint32 *aWriteCount) {
  ++consumer_count;
  *aWriteCount = 0;
  return NS_OK;
}

// Describes the type of writer that should be passed into ReadSegments().
enum Writer {
  SUCCEED,
  FAIL1 = 1,
  FAIL2 = 2,
  FAIL3 = 3,
  FAIL4 = 4,
  FAIL5 = 5,
  FAIL6 = 6,
  FAIL7 = 7,
  FAIL8 = 8,
  READ_ZERO,
};

// Describes the type of blob being used.
enum BlobType {
  NORMAL,
  FAIL,
};

// This function creates a BlobInputStream filled with data_size bytes of data.
// It then calls ReadSegments() on that stream, passing in various writers
// to confirm correct behavior of the stream's ReadSegments function.
bool TestBlobInputStreamFfReadSegments(std::string16 *error,
                                       Writer type,
                                       BlobType blob_type,
                                       BlobInterface* blob,
                                       PRUint32 in_bytes) {
  // Initialize all of these variables because some compilers complain that they
  // "may be used uninitialized in this function".
  nsWriteSegmentFun writer(NULL);
  PRUint32 expected_bytes(0);
  nsresult expected_result(NS_OK);
  int expected_count(0);

  switch (type) {
    case SUCCEED:
      writer = &ConsumerIdeal;
      expected_bytes = (in_bytes > data_size) ? data_size : in_bytes;
      expected_count = ExpectedCount(expected_bytes);
      expected_result = NS_OK;
      break;
    case FAIL1:
    case FAIL2:
    case FAIL3:
    case FAIL4:
    case FAIL5:
    case FAIL6:
    case FAIL7:
    case FAIL8:
      writer = &ConsumerFails;
      expected_count = consumer_fail = static_cast<int>(type);
      if (consumer_fail > 1) ++expected_count;
      expected_bytes = rs_buffer_size * (consumer_fail - 1);
      expected_result = NS_OK;
      // There's no reason to set the failure count too high for the
      // number of bytes we have to read.  Just use SUCCEED instead.
      assert(expected_bytes < in_bytes);
      break;
    case READ_ZERO:
      writer = &ConsumerReadsZero;
      expected_count = -1;
      expected_bytes = 1;
      expected_result = NS_OK;
      break;
  }
  switch (blob_type) {
    case NORMAL:
      // Leave as calculated above.
      break;
    case FAIL:
      // Assumes that Writer type is SUCCEED.
      TEST_ASSERT(type == SUCCEED);
      expected_bytes = 0;
      expected_count = 0;
      expected_result = NS_ERROR_FAILURE;
      break;
  }

  consumer_count = 0;
  PRUint32 out_bytes(0);
  nsCOMPtr<BlobInputStream> bs(new BlobInputStream(blob));
  nsresult result = bs->ReadSegments(writer, 0, in_bytes, &out_bytes);
  TEST_ASSERT(result == expected_result);
  TEST_ASSERT(out_bytes == expected_bytes);
  TEST_ASSERT(consumer_count == expected_count);
  if (blob_type == FAIL) {
    TEST_ASSERT(blob->Length() == -1);
  } else {
    TEST_ASSERT(blob->Length() != -1);
  }
  return true;
}

}  // namespace

bool TestBlobInputStreamFf(std::string16 *error) {
  std::vector<uint8>* data(new std::vector<uint8>(data_size, 79));
  scoped_refptr<BlobInterface> blob(new BufferBlob(data));
  bool ok = true;

  // Test a writer that always succeeds.
  // Try to read all available data.
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, NORMAL, blob.get(),
                                          data_size);
  // Try to read 1KB of data (less than a full buffer).
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, NORMAL, blob.get(),
                                          1024);
  // Try to read 1MB of data (exactly a full buffer).
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, NORMAL, blob.get(),
                                          1024 * 1024);
  // Try to read 1MB + 1 byte of data (just over 1 full buffer).
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, NORMAL, blob.get(),
                                          1024 * 1024 + 1);
  // Try to read 2MB of data (multiple full buffers).
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, NORMAL, blob.get(),
                                          1024 * 1024 * 2);
  // Try to read 2MB + 1 byte of data (just over 2 full buffers).
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, NORMAL, blob.get(),
                                          1024*1024*2 + 1);
  // Try to read 10MB of data (more than available).  Expect to return 8MB.
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, NORMAL, blob.get(),
                                          1024*1024*10);

  // Test a writer failing on the first call.
  // Try to read all available data.
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL1, NORMAL, blob.get(),
                                          data_size);
  // Try to read 1KB of data (less than a full buffer).
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL1, NORMAL, blob.get(),
                                          1024);
  // Try to read 1MB of data (exactly a full buffer).
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL1, NORMAL, blob.get(),
                                          1024 * 1024);
  // Try to read 1MB + 1 byte of data (just over 1 full buffer).
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL1, NORMAL, blob.get(),
                                          1024 * 1024 + 1);
  // Try to read 2MB of data (multiple full buffers).
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL1, NORMAL, blob.get(),
                                          1024 * 1024 * 2);
  // Try to read 2MB + 1 byte of data (just over 2 full buffers).
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL1, NORMAL, blob.get(),
                                          1024*1024*2 + 1);
  // Try to read 10MB of data (more than available).
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL1, NORMAL, blob.get(),
                                          1024 * 1024 * 10);

  // Test a writer that fails on a later call.
  // Try to read all available data, fail on 2.
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL2, NORMAL, blob.get(),
                                          data_size);
  // Try to read all available data, fail on 8.
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL8, NORMAL, blob.get(),
                                          data_size);
  // Try to read 1MB + 1 byte of data (just over 1 full buffer), fail on 2.
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL2, NORMAL, blob.get(),
                                          1024 * 1024 + 1);
  // Try to read 2MB of data (multiple full buffers), fail on 2.
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL2, NORMAL, blob.get(),
                                          1024 * 1024 * 2);
  // Try to read 2MB + 1 byte of data (just over 2 full buffers), fail on 2.
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL2, NORMAL, blob.get(),
                                          1024*1024*2 + 1);
  // Try to read 2MB + 1 byte of data (just over 2 full buffers), fail on 3.
  ok &= TestBlobInputStreamFfReadSegments(error, FAIL3, NORMAL, blob.get(),
                                          1024*1024*2 + 1);

  // Test a writer that returns NS_OK and a byte count of 0.
  // NOTE - this should result in an assert, so do NOT enable this by default!
  //ok &= TestBlobInputStreamFfReadSegments(error, READ_ZERO, NORMAL,
  //                                        blob.get(), data_size);

#if DEBUG
  // Test a fail blob.
  scoped_refptr<BlobInterface> fail_blob(new FailBlob(1024));
  ok &= TestBlobInputStreamFfReadSegments(error, SUCCEED, FAIL, fail_blob.get(),
                                          1024);
#endif  // DEBUG

  if (!ok) {
    assert(error);
    *error += STRING16(L"TestBlobInputStreamFf - failed. ");
  }
  return ok;
}
