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

#ifndef GEARS_BLOB_FAIL_BLOB_H_
#define GEARS_BLOB_FAIL_BLOB_H_
#ifdef DEBUG

#include "gears/blob/blob_interface.h"

// It is a blob containing arbitrary data of specified size.  Once an attempt
// is made to read the last byte, both Read() and Length() will return -1
// (the error condition).
// This class is used for debugging only.  In particular, it is meant to
// replicate a file-backed blob whose file contents have changed.
class FailBlob : public BlobInterface {
 public:
  explicit FailBlob(int64 num_bytes);
  ~FailBlob();
  virtual int64 Read(uint8 *destination, int64 offset, int64 max_bytes) const;
  virtual int64 ReadDirect(Reader *reader, int64 offset, int64 max_bytes) const;
  virtual int64 Length() const;
  virtual bool GetDataElements(std::vector<DataElement> *elements) const;
 private:
  int64 length_;
  mutable bool failed_;
  DISALLOW_EVIL_CONSTRUCTORS(FailBlob);
};

#endif  // DEBUG
#endif  // GEARS_BLOB_FAIL_BLOB_H_
