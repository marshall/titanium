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

#include "gears/blob/blob_input_stream_ff.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>
#include "gears/blob/blob_interface.h"

//------------------------------------------------------------------------------
// BlobInputStream implementation
//------------------------------------------------------------------------------

BlobInputStream::BlobInputStream()
    : offset_(0) {
}

BlobInputStream::BlobInputStream(BlobInterface *blob)
    : blob_(blob),
      offset_(0) {
}

BlobInputStream::~BlobInputStream() {
}

void BlobInputStream::Reset(BlobInterface *blob) {
  blob_.reset(blob);
  offset_ = 0;
}

//------------------------------------------------------------------------------
// nsIInputStream implementation
//------------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ISUPPORTS1(BlobInputStream,
                              nsIInputStream)

NS_IMETHODIMP BlobInputStream::Available(PRUint32 *out_available_bytes) {
  if (blob_.get() == NULL) {
    return NS_ERROR_NOT_INITIALIZED;
  }

  int64 remaining = blob_->Length() - offset_;
  if (remaining < 0) {
    remaining = 0;
  }
  if (remaining > std::numeric_limits<PRUint32>::max()) {
    remaining = std::numeric_limits<PRUint32>::max();
  }

  if (!out_available_bytes) {
    return NS_ERROR_NULL_POINTER;
  }

  *out_available_bytes = static_cast<PRUint32>(remaining);
  return NS_OK;
}

NS_IMETHODIMP BlobInputStream::Close() {
  blob_.reset();
  offset_ = 0;
  return NS_OK;
}

NS_IMETHODIMP BlobInputStream::IsNonBlocking(PRBool *out_non_blocking) {
  if (!out_non_blocking) {
    return NS_ERROR_NULL_POINTER;
  }

  *out_non_blocking = false;
  return NS_OK;
}

NS_IMETHODIMP BlobInputStream::Read(char *buffer,
                                    PRUint32 buffer_size,
                                    PRUint32 *out_num_bytes_read) {
  if (blob_.get() == NULL) {
    return NS_ERROR_NOT_INITIALIZED;
  }
  if (!out_num_bytes_read) {
    return NS_ERROR_NULL_POINTER;
  }
  int64 num_read = blob_->Read(reinterpret_cast<uint8*>(buffer),
                               offset_, buffer_size);
  if (num_read < 0) {
    return NS_ERROR_FAILURE;
  }
  // Confirm that BlobInterface::Read returned <= bytes requested
  assert(num_read <= static_cast<int64>(buffer_size));
  offset_ += num_read;
  *out_num_bytes_read = static_cast<PRUint32>(num_read);
  // EOS is indicated by setting *out_num_bytes_read = 0.
  return NS_OK;
}

namespace {
class ReadSegmentsReader : public BlobInterface::Reader {
 public:
  ReadSegmentsReader(BlobInputStream *stream, nsWriteSegmentFun writer,
                     void *writer_arg)
      : stream_(stream), writer_(writer), writer_arg_(writer_arg) {
  }
  virtual int64 ReadFromBuffer(const uint8 *buffer, int64 max_bytes) {
    PRUint32 available(static_cast<uint32>(
                           std::min(max_bytes,
                                    static_cast<int64>(kuint32max))));
    PRUint32 consumed(0);
    // The fourth argument to writer is supposed to be "The number of bytes
    // already consumed during this call to nsIInputStream::readSegments",
    // presumably to allow writer to skip that far into the buffer before
    // writing. However, I found several implementations of nsWriteSegmentFun
    // within Firefox source code, and every one of them simply ignores
    // that argument. So we'll ignore that argument too.
    nsresult result = writer_(stream_, writer_arg_,
                              reinterpret_cast<const char*>(buffer), 0,
                              available, &consumed);
    if (NS_FAILED(result)) {
      // If the writer returns an error, then ReadSegments should exit.
      // Any error returned from writer is not passed on to the
      // caller of ReadSegments.
      return 0;
    }
    // Confirm that writer consumed <= number of bytes provided to it.
    assert(consumed <= available);
    // Returning NS_OK with consumed == 0 has undefined meaning.
    assert(consumed > 0);
    return consumed;
  }

 private:
  BlobInputStream *stream_;
  nsWriteSegmentFun writer_;
  void *writer_arg_;
};
}

// Low-level read method that has access to the stream's underlying buffer.
// The writer function may be called multiple times for segmented buffers.
// ReadSegments is expected to keep calling the writer until either there is
// nothing left to read or the writer returns an error.  ReadSegments should
// not call the writer with zero bytes to consume.
NS_IMETHODIMP BlobInputStream::ReadSegments(nsWriteSegmentFun writer,
                                            void *writer_arg,
                                            PRUint32 num_bytes_to_read,
                                            PRUint32 *out_num_bytes_read) {
  if (blob_.get() == NULL) {
    return NS_ERROR_NOT_INITIALIZED;
  }
  if (!out_num_bytes_read) {
    return NS_ERROR_NULL_POINTER;
  }
  *out_num_bytes_read = 0;

  ReadSegmentsReader reader(this, writer, writer_arg);

  while (num_bytes_to_read) {
    int64 num_read = blob_->ReadDirect(&reader, offset_, num_bytes_to_read);
    if (num_read < 0) {
      return NS_ERROR_FAILURE;
    }
    if (num_read == 0) {
      // No more data to read.
      break;
    }
    // Confirm that BlobInterface::Read returned <= bytes requested
    assert(num_read <= static_cast<int64>(num_bytes_to_read));
    offset_ += num_read;
    *out_num_bytes_read += static_cast<uint32>(num_read);
    num_bytes_to_read -= static_cast<uint32>(num_read);
  }
  // EOS is indicated by setting *out_num_bytes_read = 0.
  // This is indistinguishable from the writer failing case.
  // Callers can distinguish between the two by calling Available().
  return NS_OK;
}
