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

#ifndef GEARS_DESKTOP_FILE_BLOB_H__
#define GEARS_DESKTOP_FILE_BLOB_H__

#include "gears/base/common/file.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/string16.h"
#include "gears/blob/blob_interface.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class File;

// FileBlob provides a blob interface to a file's contents.
class FileBlob : public BlobInterface {
 public:
  // The filename should be an absolute path, not a relative one.
  // If it is not valid, Read() and Length() will return -1.
  FileBlob(const std::string16& filename);

  // FileBlob will assume ownership of the File object and will delete
  // it when FileBlob is deleted.
  FileBlob(File *file);

  // The absolute path of the contained file.
  const std::string16 &GetFilePath() const;

  virtual int64 Read(uint8 *destination, int64 offset, int64 max_bytes) const;
  virtual int64 Length() const;
  virtual bool GetDataElements(std::vector<DataElement> *elements) const;
 private:
  mutable Mutex file_lock_;
  scoped_ptr<File> file_;
  mutable int64 size_;
  mutable int64 last_modified_time_;

  // Only call this private function when the caller is holding the
  // file_lock_ Mutex.
  bool FileHasChanged() const;

  DISALLOW_EVIL_CONSTRUCTORS(FileBlob);
};

#endif  // GEARS_DESKTOP_FILE_BLOB_H__
