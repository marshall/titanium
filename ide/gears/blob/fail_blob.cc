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

#include "gears/blob/fail_blob.h"
#ifdef DEBUG

FailBlob::FailBlob(int64 num_bytes) : length_(num_bytes), failed_(false) {
}

FailBlob::~FailBlob() {
}

int64 FailBlob::Read(uint8 *destination, int64 offset, int64 max_bytes) const {
  if (offset < 0 || max_bytes < 0) {
    return -1;
  }
  if (failed_) {
    return -1;
  }
  if (offset + max_bytes >= length_) {
    failed_ = true;
    return -1;
  }
  // Just pretend we copied bytes to destination.
  return max_bytes;
}

int64 FailBlob::ReadDirect(Reader *reader, int64 offset,
                           int64 max_bytes) const {
  if (offset < 0 || max_bytes < 0) {
    return -1;
  }
  if (failed_) {
    return -1;
  }
  if (offset + max_bytes >= length_) {
    failed_ = true;
    return -1;
  }
  // Give reader a buffer from which to read.
  int64 total_bytes_read(0);
  while (max_bytes > 0) {
    std::vector<uint8> buffer(static_cast<size_t>(max_bytes));
    int64 bytes_read = reader->ReadFromBuffer(&buffer[0], max_bytes);
    assert(bytes_read >= 0);
    if (bytes_read == 0) break;
    assert(bytes_read <= max_bytes);
    total_bytes_read += bytes_read;
    max_bytes -= bytes_read;
  }
  return total_bytes_read;
}

int64 FailBlob::Length() const {
  if (failed_) {
    return -1;
  }
  return length_;
}

bool FailBlob::GetDataElements(std::vector<DataElement> *elements) const {
  return false;  // Not supported.
}

#endif  // DEBUG
