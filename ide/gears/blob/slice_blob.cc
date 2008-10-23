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

#include <assert.h>

#include "gears/blob/slice_blob.h"

SliceBlob::SliceBlob(BlobInterface *source, int64 offset, int64 length)
    : blob_(source), offset_(offset), length_(length) {
  // We explicitly allow offset and length to indicate a slice that extends
  // beyond the end of the source blob, although Read will return no data for
  // that portion.
  assert(offset_ >= 0);
  assert(length_ >= 0);
}

int64 SliceBlob::Read(uint8 *destination, int64 offset, int64 max_bytes) const {
  if (offset < 0 || max_bytes < 0) {
    return -1;
  }
  if (offset >= length_) {
    return 0;
  }
  int64 available = length_ - offset;
  if (available > max_bytes) {
    available = max_bytes;
  }
  return blob_->Read(destination, offset_ + offset, available);
}

int64 SliceBlob::ReadDirect(Reader *reader, int64 offset,
                            int64 max_bytes) const {
  if (offset < 0 || max_bytes < 0) {
    return -1;
  }
  if (offset >= length_) {
    return 0;
  }
  int64 available = length_ - offset;
  if (available > max_bytes) {
    available = max_bytes;
  }
  return blob_->ReadDirect(reader, offset_ + offset, available);
}


int64 SliceBlob::Length() const {
  return length_;
}

bool SliceBlob::GetDataElements(std::vector<DataElement> *elements) const {
  assert(elements && elements->empty());

  std::vector<DataElement> nested_elements;
  if (!blob_->GetDataElements(&nested_elements)) {
    return false;
  }
  
  if (!nested_elements.empty()) {
    std::vector<DataElement>::iterator iter = nested_elements.begin();

    // Skip over elements until we reach our offset_
    uint64 skip_remaining = static_cast<uint64>(offset_);
    while (skip_remaining > 0 && iter != nested_elements.end()) {
      uint64 content_length = (*iter).GetContentLength();
      if (content_length > skip_remaining) {
        (*iter).TrimFront(skip_remaining);
        break;  // don't advance the iter, this element is included below
      }
      skip_remaining -= content_length;
      ++iter; 
    }      

    // Include elements until we reach our length_
    uint64 include_remaining = static_cast<uint64>(length_);
    while (include_remaining > 0 && iter != nested_elements.end()) {
      uint64 content_length = (*iter).GetContentLength();
      if (content_length > include_remaining) {
        (*iter).TrimToLength(include_remaining);
        content_length = include_remaining;
      }
      elements->push_back(*iter);
      include_remaining -= content_length;
      ++iter;
    }
  }

  return true;
}
