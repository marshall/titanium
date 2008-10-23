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

#ifndef GEARS_BASE_COMMON_BYTE_STORE_H_
#define GEARS_BASE_COMMON_BYTE_STORE_H_

#include "gears/base/common/basictypes.h"
#include "gears/base/common/file.h"
#include "gears/base/common/memory_buffer.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class BlobInterface;
class DataElement;

class ByteStore : public RefCounted {
 public:
  ByteStore();

  // Functor used to read directly from ByteStore's internal buffer.
  class Reader {
   public:
    // Reads at most max_length bytes from buffer.  Returns the number
    // of bytes read.  Returns 0 when finished.
    virtual int64 ReadFromBuffer(const uint8 *buffer, int64 max_length) = 0;
  };

  // Functor used to write directly to ByteStore's internal buffer.
  class Writer {
   public:
    static const int64 ASYNC = -9;
    // Write at most max_length bytes to buffer.  Returns the number
    // of bytes written.  Returns 0 when finished.
    // Returns ASYNC if the write will be finished asynchronously.
    virtual int64 WriteToBuffer(uint8 *buffer, int64 max_length) = 0;
  };

  // Appends raw data.  Returns false if the data could not be added.
  bool AddData(const void *data, int64 length);

  // Appends raw data directly to the internal data buffer.
  // max_length is a value >= the number of bytes that will be added.
  // Returns the number of bytes added.
  int64 AddDataDirect(Writer *writer, int64 max_length);

  // Called when an asynchronous Writer has finished.
  void AddDataDirectFinishAsync(int64 length);

  // String data is assumed to be UTF16 and will be converted to UTF8.
  // Returns false if the data could not be added.
  bool AddString(const std::string16 &data);

  // Returns a blob interface of a snapshot of the ByteStore.
  // Data can continue to be added to the ByteStore, but this blob
  // will only present the data that existed at construction time.
  void CreateBlob(scoped_refptr<BlobInterface> *blob);

  // The contents of a finalized ByteStore are no longer modifiable.
  void Finalize();

  bool IsFinalized() const { return is_finalized_; }

  // Returns the length of all data.
  int64 Length() const;

  // Copies 'length' data at 'offset' into the supplied buffer.
  int64 Read(uint8 *destination, int64 offset, int64 max_length) const;

  // Reads data directly from the internal data buffer.
  // max_length is the upper limit on the amount of data to be read.
  // Returns the number of bytes read.
  int64 ReadDirect(Reader *reader, int64 offset, int64 max_length) const;

  // Attempts to reserve memory for storage.
  void Reserve(int64 length);

 private:
  ~ByteStore();

  bool AddDataToFile(const void *data, int64 length);
  class Blob;
  void GetDataElement(DataElement *elements);
  int64 ReadFromFile(uint8 *destination, int64 offset, int64 max_length) const;

  // Helpers that return whether the store is working out of a file
  // or an in memory data buffer.
  bool IsUsingFile() const { return file_.get() ? true : false; }
  bool IsUsingData() const { return !IsUsingFile(); }

  MemoryBuffer data_;
  scoped_ptr<File> file_;
  mutable File::OpenAccessMode file_op_;
  mutable Mutex mutex_;
  bool is_finalized_;
  bool preserve_data_;
  mutable MemoryBuffer read_buffer_;
  mutable MemoryBuffer write_buffer_;
  int64 async_add_length_;
  int64 length_;
  DISALLOW_EVIL_CONSTRUCTORS(ByteStore);
};

#endif  // GEARS_BASE_COMMON_BYTE_STORE_H_
