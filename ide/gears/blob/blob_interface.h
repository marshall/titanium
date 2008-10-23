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

#ifndef GEARS_BLOB_BLOB_INTERFACE_H__
#define GEARS_BLOB_BLOB_INTERFACE_H__

#include <assert.h>
#include <vector>
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/scoped_refptr.h"


// Class used to expose the internal structure of a Blob
class DataElement {
 public:
  enum Type {
    TYPE_BYTES,
    TYPE_FILE
  };

  DataElement() : type_(TYPE_BYTES), bytes_(NULL), bytes_length_(0),
                  file_range_offset_(0), file_range_length_(kuint64max) {
  }


  Type type() const { return type_; }

  // Only valid for TYPE_BYTES
  const uint8 *bytes() const { return bytes_; }
  int bytes_length() const { return bytes_length_; }

  // Only valid for TYPE_FILE
  const std::string16& file_path() const { return file_path_; }
  uint64 file_range_offset() const { return file_range_offset_; }
  uint64 file_range_length() const { return file_range_length_; }

  void SetToBytes(const uint8 *bytes, int bytes_len) {
    type_ = TYPE_BYTES;
    bytes_ = bytes;
    bytes_length_ = bytes_len;
  }

  void SetToFilePath(const std::string16 &path) {
    SetToFilePathRange(path, 0, kuint64max);
  }

  // If offset + length exceeds the end of the file, the length reported
  // by GetContentLength is clipped at eof.
  void SetToFilePathRange(const std::string16 &path,
                          uint64 offset, uint64 length) {
    type_ = TYPE_FILE;
    file_path_ = path;
    file_range_offset_ = offset;
    file_range_length_ = length;
  }

  // Returns the byte-length of the element.  For files that do not exist, 0
  // is returned. This is done for consistency with Mozilla.
  uint64 GetContentLength() const {
    if (type_ == TYPE_BYTES) {
      return static_cast<uint64>(bytes_length_);
    }
    assert(type_ == TYPE_FILE);
    int64 length = File::GetFileSize(file_path_.c_str());
    if (length < 0)
      return 0;  // file does not exist
    if (file_range_offset_ >= static_cast<uint64>(length))
      return 0;  // range is beyond eof

    // compensate for the offset and clip file_range_length_ to eof
    return std::min(static_cast<uint64>(length) - file_range_offset_,
                    file_range_length_);
  }

  void TrimFront(uint64 amount) {
    uint64 length = GetContentLength();
    assert(amount <= length);
    if (type_ == TYPE_BYTES) {
      bytes_ += amount;
      bytes_length_ = static_cast<int>(length - amount);
    } else {
      assert(type_ == TYPE_FILE);
      file_range_offset_ += amount;
    }
  }

  void TrimToLength(uint64 new_length) {
    assert(new_length <= GetContentLength());
    if (type_ == TYPE_BYTES) {
      bytes_length_ = static_cast<int>(new_length);
    } else {
      assert(type_ == TYPE_FILE);
      file_range_length_ = new_length;
    }
  }

 private:
  Type type_;
  const uint8 *bytes_;
  int bytes_length_;
  std::string16 file_path_;
  uint64 file_range_offset_;
  uint64 file_range_length_;
};


class BlobInterface : public RefCounted {
 public:
  // Interface used to read directly from a blob's internal buffer.
  class Reader {
   public:
    // Reads at most max_length bytes from buffer.  Returns the number
    // of bytes read.  Returns 0 when finished.
    virtual int64 ReadFromBuffer(const uint8 *buffer, int64 max_bytes) = 0;
  };

  // Reads up to max_bytes from the blob beginning at the absolute position
  // indicated by offset, and writes the data into destination.  Multiple Reads
  // are unrelated.  Returns the number of bytes successfully read, or -1 on
  // error.  A null destination or negative offset or max_bytes will result in
  // an error.  An offset beyond the end of the stream will succeed and return 0
  // bytes.
  virtual int64 Read(uint8 *destination, int64 offset,
                     int64 max_bytes) const = 0;

  // Reads data directly from the internal data buffer (if applicable).
  // Otherwise, will copy data into a temporary buffer and pass that buffer
  // to the Reader.
  // max_bytes is the upper limit on the amount of data to be read.
  // Returns the number of bytes read.
  virtual int64 ReadDirect(Reader *reader, int64 offset, int64 max_bytes) const;

  // Note that Length can be volatile, e.g. a file-backed Blob can have that
  // file's size change underneath it.
  virtual int64 Length() const = 0;

  // Returns an array that describes storage backing the blob's data.
  // Note that it is the caller's responsibility to ensure that the Blob
  // instance remains in scope for the duration that the 'elements' array
  // needs to be utilized.
  virtual bool GetDataElements(std::vector<DataElement> *elements) const = 0;

 protected:
  BlobInterface() {}
  virtual ~BlobInterface() {}

 private:
  DISALLOW_EVIL_CONSTRUCTORS(BlobInterface);
};


class EmptyBlob : public BlobInterface {
 public:
  EmptyBlob() {}

  virtual int64 Read(uint8 *destination, int64 offset, int64 max_bytes) const {
    if (!destination || (offset < 0) || (max_bytes < 0))
      return -1;
    return 0;
  }

  virtual int64 ReadDirect(Reader *reader, int64 offset,
                           int64 max_bytes) const {
    if (!reader || (offset < 0) || (max_bytes < 0))
      return -1;
    return 0;
  }

  virtual int64 Length() const {
    return 0;
  }

  virtual bool GetDataElements(std::vector<DataElement> *elements) const {
    assert(elements && elements->empty());
    return true;
  }

 private:
  DISALLOW_EVIL_CONSTRUCTORS(EmptyBlob);
};

#endif  // GEARS_BLOB_BLOB_INTERFACE_H__
