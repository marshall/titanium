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

#include "gears/canvas/blob_backed_skia_input_stream.h"
#include <algorithm>
#include "third_party/scoped_ptr/scoped_ptr.h"

template<class T, class S> T checked_cast(S in) {
  T out = static_cast<T>(in);
  assert (out == in);
  return out;
}

BlobBackedSkiaInputStream::BlobBackedSkiaInputStream(BlobInterface *blob)
    : blob_(blob), blob_offset_(0) {
}

BlobBackedSkiaInputStream::~BlobBackedSkiaInputStream() {
}

size_t BlobBackedSkiaInputStream::read(void* buffer, size_t size) {
  if (buffer == NULL && size == 0) {
    // Must return total length of stream.
    return checked_cast<size_t>(blob_->Length());
  }
  if (buffer == NULL) {
    // Must skip over `size` bytes. 
    // Don't go past the end.
    size = std::min(size, checked_cast<size_t>(blob_->Length() - blob_offset_));
    blob_offset_ += size;
    return size;
  }
  int64 bytes_read = blob_->Read(
      static_cast<uint8 *>(buffer), blob_offset_, size);
  if (bytes_read < 0)
    return 0;
  blob_offset_ += bytes_read;
  return checked_cast<size_t>(bytes_read);
}

bool BlobBackedSkiaInputStream::rewind() {
  blob_offset_ = 0;
  return true;
}
