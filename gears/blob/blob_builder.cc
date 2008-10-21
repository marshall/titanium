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

#include "gears/blob/blob_builder.h"

#include "gears/blob/blob_interface.h"
#include "gears/blob/join_blob.h"

namespace {
const int64 kMaxBufferSize = 1024 * 1024; // 1MB
}

BlobBuilder::BlobBuilder() : byte_store_(new ByteStore) {
}

BlobBuilder::~BlobBuilder() {
  byte_store_->Finalize();
}

bool BlobBuilder::AddBlob(BlobInterface *blob) {
  if (blob->Length() < 0) return false;
  if (blob->Length() == 0) return true;
  if (byte_store_->Length()) {
    scoped_refptr<BlobInterface> byte_store_blob;
    byte_store_->CreateBlob(&byte_store_blob);
    blob_list_.push_back(byte_store_blob.get());
    byte_store_->Finalize();
    byte_store_.reset(new ByteStore);
  }
  blob_list_.push_back(blob);
  return true;
}

bool BlobBuilder::AddData(const void *data, int64 length) {
  return byte_store_->AddData(data, length);
}

bool BlobBuilder::AddString(const std::string16 &data) {
  return byte_store_->AddString(data);
}

void BlobBuilder::CreateBlob(scoped_refptr<BlobInterface> *blob) {
  bool pushed_unfinalized_data = false;
  if (byte_store_->Length()) {
    scoped_refptr<BlobInterface> byte_store_blob;
    byte_store_->CreateBlob(&byte_store_blob);
    blob_list_.push_back(byte_store_blob.get());
    pushed_unfinalized_data = true;
  }

  if (blob_list_.empty()) {
    *blob = new EmptyBlob;
  } else if (blob_list_.size() == 1) {
    *blob = blob_list_.back();
  } else {
    *blob = new JoinBlob(blob_list_);
  }

  if (pushed_unfinalized_data) {
    blob_list_.pop_back();
  }
}

int64 BlobBuilder::Length() const {
  int64 length(byte_store_->Length());
  for (unsigned i = 0; i < blob_list_.size();  ++i) {
    length += blob_list_[i]->Length();
  }
  return length;
}

void  BlobBuilder::Reset() {
  byte_store_->Finalize();
  byte_store_.reset(new ByteStore);
  blob_list_.clear();
}
