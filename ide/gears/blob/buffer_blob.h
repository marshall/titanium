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

#ifndef GEARS_BLOB_BUFFER_BLOB_H__
#define GEARS_BLOB_BUFFER_BLOB_H__

#include <vector>
#include "gears/blob/blob_interface.h"

// Because BufferBlobs store their contents in a vector, they are restricted in
// size by the maximum value of int.
class BufferBlob : public BlobInterface {
 public:
  // Initialize a BufferBlob from the contents of a vector.
  // It performs a vector::swap, so that no copy is made.
  // The vector argument is an in/out parameter, and will be returned empty.
  BufferBlob(std::vector<uint8> *buffer);

  // Initializes a BufferBlob from an arbitrary array of bytes.
  // The bytes are copied.
  BufferBlob(const void *source, int64 num_bytes);

  virtual int64 Read(uint8 *destination, int64 offset, int64 max_bytes) const;
  virtual int64 ReadDirect(Reader *reader, int64 offset, int64 max_bytes) const;
  virtual int64 Length() const;
  virtual bool GetDataElements(std::vector<DataElement> *elements) const;
 private:
  typedef std::vector<uint8> BufferType;
  typedef BufferType::size_type size_type;
  BufferType buffer_;

  DISALLOW_EVIL_CONSTRUCTORS(BufferBlob);
};

#endif  // GEARS_BLOB_BUFFER_BLOB_H__
