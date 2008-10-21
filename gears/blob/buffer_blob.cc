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

#include <algorithm>
#include <cassert>
#include <limits>
#include "gears/blob/buffer_blob.h"

BufferBlob::BufferBlob(std::vector<uint8> *buffer) {
  buffer_.swap(*buffer);
}

BufferBlob::BufferBlob(const void *source, int64 num_bytes)
    : buffer_(static_cast<const uint8*>(source),
              static_cast<const uint8*>(source) + num_bytes) {
  // The current implementation of BufferBlob stores data in a vector.
  // A vector has an upper bound for how much data it can hold.
  // Make sure we haven't gone beyond this limit.
  assert(num_bytes <= buffer_.max_size());
}

int64 BufferBlob::Read(uint8 *destination, int64 offset,
                       int64 max_bytes) const {
  if (offset < 0 || max_bytes < 0) {
    return -1;
  }
  if (offset >= buffer_.size() || max_bytes == 0) {
    return 0;
  }
  int64 available = buffer_.size() - offset;
  int64 num_bytes = std::min(available, max_bytes);
  if (num_bytes > std::numeric_limits<size_t>::max()) {
    num_bytes = static_cast<int64>(std::numeric_limits<size_t>::max());
  }
  memcpy(destination, &(buffer_[static_cast<size_type>(offset)]),
         static_cast<size_t>(num_bytes));
  return num_bytes;
}

int64 BufferBlob::ReadDirect(Reader *reader, int64 offset,
                             int64 max_bytes) const {
  if (offset < 0 || max_bytes < 0) {
    return -1;
  }
  if (offset >= buffer_.size() || max_bytes == 0) {
    return 0;
  }
  if (buffer_.size() - offset < max_bytes) {
    max_bytes = buffer_.size() - offset;
  }
  int64 total_bytes_read(0);
  size_type pos(static_cast<size_type>(offset));
  while (max_bytes > 0) {
    int64 bytes_read = reader->ReadFromBuffer(&buffer_[pos], max_bytes);
    assert(bytes_read >= 0);
    if (bytes_read == 0) break;
    assert(bytes_read <= max_bytes);
    total_bytes_read += bytes_read;
    pos += static_cast<size_type>(bytes_read);
    max_bytes -= bytes_read;
  }
  return total_bytes_read;
}

int64 BufferBlob::Length() const {
  return buffer_.size();
}

bool BufferBlob::GetDataElements(std::vector<DataElement> *elements) const {
  assert(elements && elements->empty());
  if (!buffer_.empty()) {
    assert(buffer_.size() <= static_cast<int>(kint32max));
    elements->push_back(DataElement());
    elements->back().SetToBytes(&buffer_[0], 
                                static_cast<int>(buffer_.size()));
  }
  return true;
}
