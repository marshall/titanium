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

#ifndef GEARS_BLOB_JOIN_BLOB_H__
#define GEARS_BLOB_JOIN_BLOB_H__

#include <map>
#include <vector>
#include "gears/blob/blob_interface.h"
#include "gears/base/common/scoped_refptr.h"

// JoinBlob provides a single blob interface to multiple blobs.
class JoinBlob : public BlobInterface {
 public:
  typedef std::vector<scoped_refptr<BlobInterface> > List;

  explicit JoinBlob(const List &blob_list);

  virtual int64 Read(uint8 *destination, int64 offset, int64 max_bytes) const;
  virtual int64 ReadDirect(Reader *reader, int64 offset, int64 max_bytes) const;
  virtual int64 Length() const { return length_; }
  virtual bool GetDataElements(std::vector<DataElement> *elements) const;
 private:
  typedef std::map<int64, scoped_refptr<BlobInterface> > Map;
  Map blob_map_;
  int64 length_;
  DISALLOW_EVIL_CONSTRUCTORS(JoinBlob);
};

#endif  // GEARS_BLOB_JOIN_BLOB_H__
