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

#include "gears/base/common/file.h"
#include "gears/blob/file_blob.h"


FileBlob::FileBlob(const std::string16& filename)
    : file_(File::Open(filename.c_str(), File::READ,
                       File::FAIL_IF_NOT_EXISTS)),
      size_(File::kInvalidSize),
      last_modified_time_(File::kInvalidLastModifiedTime) {
}


FileBlob::FileBlob(File *file)
    : file_(file),
      size_(File::kInvalidSize),
      last_modified_time_(File::kInvalidLastModifiedTime) {
}


int64 FileBlob::Read(uint8 *destination, int64 offset, int64 max_bytes) const {
  MutexLock locker(&file_lock_);
  if (file_.get() && file_->Seek(offset, File::SEEK_FROM_START)) {
    int64 result = file_->Read(destination, max_bytes);
    if (!FileHasChanged()) {
      return result;
    }
  }
  return -1;
}


int64 FileBlob::Length() const {
  MutexLock locker(&file_lock_);
  if (size_ != File::kInvalidSize) {
    if (!FileHasChanged()) {
      return size_;
    }
  } else if (file_.get()) {
    int64 result = file_->Size();
    if (!FileHasChanged()) {
      size_ = result;
      return size_;
    }
  }
  return -1;
}


bool FileBlob::GetDataElements(std::vector<DataElement> *elements) const {
  assert(elements && elements->empty());
  if (!file_.get()) {
    return false;
  }
  const std::string16 &file_path = file_->GetFilePath();
  if (file_path.empty()) {
    return false;
  }
  elements->push_back(DataElement());
  elements->back().SetToFilePath(file_path);
  return true;
}


const std::string16 &FileBlob::GetFilePath() const {
  if (!file_.get()) {
    static std::string16 kEmpty;
    return kEmpty;
  }
  return file_->GetFilePath();
}


bool FileBlob::FileHasChanged() const {
  // To check whether a file has changed, we simply look at the mtime, since a
  // file should not be able to change its size without also modifying that.
  int64 mtime = File::LastModifiedTime(file_->GetFilePath().c_str());
  if (last_modified_time_ == File::kInvalidLastModifiedTime) {
    last_modified_time_ = mtime;
    return false;
  }
  return last_modified_time_ != mtime;
}
