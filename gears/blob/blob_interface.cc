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

#include "gears/blob/blob_interface.h"

#include "gears/base/common/memory_buffer.h"

namespace {
const int64 kTempBufferSize = 1024 * 1024;  // 1MB
}

// Default implementation of ReadDirect, for blobs that do not have an
// internal data buffer from which to read (ex - files).
// It creates a temporary buffer and calls Read() to fill it.
int64 BlobInterface::ReadDirect(Reader *reader, int64 offset,
                                int64 max_bytes) const {
  int64 buffer_size(std::min(max_bytes, kTempBufferSize));
  MemoryBuffer buffer(buffer_size);
  int64 total_bytes_read(0);
  while (max_bytes > 0) {
    int64 bytes_read1 = Read(buffer.Data(0), offset, buffer_size);
    // Deal with error while reading.
    if (bytes_read1 < 0) {
      // Return the number of bytes successfully read before the error.
      // Return -1 if we haven't read any bytes.
      if (!total_bytes_read) {
        total_bytes_read = -1;
      }
      break;
    }
    if (bytes_read1 == 0) {
      break;  // No more data available.
    }
    int64 bytes_read = reader->ReadFromBuffer(buffer.Data(0), bytes_read1);
    assert(bytes_read >= 0);
    if (bytes_read == 0) break;
    total_bytes_read += bytes_read;
    offset += bytes_read;
    max_bytes -= bytes_read;
    buffer_size = std::min(max_bytes, buffer_size);
  }
  return total_bytes_read;
}
