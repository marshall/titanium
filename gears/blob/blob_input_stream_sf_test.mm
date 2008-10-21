// Copyright 2008 Google Inc. All Rights Reserved.
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

#import <cstring>

#import "gears/blob/blob_input_stream_sf.h"
#import "gears/blob/buffer_blob.h"
#if DEBUG
#import "gears/blob/fail_blob.h"
#endif  // DEBUG
#import "third_party/scoped_ptr/scoped_ptr.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOCATION __FILE__ ", line " TOSTRING(__LINE__)
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestBlobInputStreamSf - failed (%d)\n", __LINE__)); \
    assert(error); \
    if (!error->empty()) *error += STRING16(L", "); \
    *error += STRING16(L"failed at " LOCATION); \
    return false; \
  } \
}

namespace {
  // Amount of data that we create (8MB).
  const std::vector<uint8>::size_type data_size(1024 * 1024 * 8);

  // Buffer for holding that amount of data.
  uint8_t *buffer(0);
}

// This function assumes that the stream passed in has |data_size|
// bytes available for reading.
static bool TestBlobInputStreamSfReadFullStream(std::string16 *error,
                                                NSInputStream *stream) {
  assert(stream);
  BOOL rslt = [stream hasBytesAvailable];
  TEST_ASSERT(rslt == YES);

  memset(buffer, 0, data_size);
  NSInteger numRead = [stream read:buffer maxLength:8];
  TEST_ASSERT(numRead == 8);
  TEST_ASSERT(buffer[0] == 79);
  TEST_ASSERT(buffer[7] == 79);
  TEST_ASSERT(buffer[8] == 0);

  memset(buffer, 0, data_size);
  numRead = [stream read:buffer maxLength:data_size];
  TEST_ASSERT(numRead == data_size - 8);
  TEST_ASSERT(buffer[0] == 79);
  TEST_ASSERT(buffer[data_size - 9] == 79);
  TEST_ASSERT(buffer[data_size - 8] == 0);

  rslt = [stream hasBytesAvailable];
  TEST_ASSERT(rslt == YES);  // Testing OSX 10.4 workaround.

  memset(buffer, 0, data_size);
  numRead = [stream read:buffer maxLength:data_size];
  TEST_ASSERT(numRead == 0);
  TEST_ASSERT(buffer[0] == 0);

  return true;
}

bool TestBlobInputStreamSf(std::string16 *error) {
  // Some data we can use to initialize our blobs.
  std::vector<uint8> data(data_size, 79);
  scoped_refptr<BlobInterface> blob(new BufferBlob(&data));
  bool ok = true;

  // Initialize the buffer.
  buffer = new uint8[data_size];
  scoped_array<uint8> buffer_cleaner(buffer);

  {
    BlobInputStream *stream;

    // Test BlobInputStream::initFromBlob
    stream = [[[BlobInputStream alloc] initFromBlob:blob.get()] autorelease];
    uint8_t *bufferPointer;
    NSUInteger len;
    [stream open];
    BOOL rslt = [stream getBuffer:&bufferPointer length:&len];
    TEST_ASSERT(rslt == NO);

    ok &= TestBlobInputStreamSfReadFullStream(error, stream);
    [stream close];
  }
  {
    // Test a BlobInputStream that has not been initialized with a blob.
    BlobInputStream *blobStream = [[[BlobInputStream alloc] init] autorelease];

    [blobStream open];
    BOOL rslt = [blobStream hasBytesAvailable];
    TEST_ASSERT(rslt == YES);  // Testing OSX 10.4 workaround.

    memset(buffer, 0, data_size);
    NSInteger numRead = [blobStream read:buffer maxLength:data_size];
    TEST_ASSERT(numRead == 0);
    TEST_ASSERT(buffer[0] == 0);
    [blobStream close];

    // Hand a blob to the BlobInputStream.
    [blobStream reset:blob.get()];
    [blobStream open];
    ok &= TestBlobInputStreamSfReadFullStream(error, blobStream);
    [blobStream close];

    // reset a BlobInputStream that already has a blob assigned.
    [blobStream reset:blob.get()];
    [blobStream open];
    ok &= TestBlobInputStreamSfReadFullStream(error, blobStream);
    [blobStream close];
  }

#if DEBUG
  // FailBlob is only available in debug builds.
  {
    // Test a BlobInputStream that has been initialized with a FailBlob.
    // It should propagate the failure by returning -1 from |read:maxLength:|.
    scoped_refptr<BlobInterface> blob(new FailBlob(5000));
    BlobInputStream *stream;
    stream = [[[BlobInputStream alloc] initFromBlob:blob.get()] autorelease];
    [stream open];
    NSInteger numRead = [stream read:buffer maxLength:data_size];
    TEST_ASSERT(numRead == -1);
    [stream close];
  }
#endif  // DEBUG

  return ok;
}
