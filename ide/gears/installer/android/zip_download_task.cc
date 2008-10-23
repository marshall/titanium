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

#include "gears/installer/android/zip_download_task.h"

#include "gears/base/common/file.h"
#include "gears/blob/blob_interface.h"
#include "gears/blob/blob_utils.h"

static const char16* kArchiveExtension = STRING16(L".zip");

static const int kBufferSizeBytes = 32 * 1024;  // 32 Kb

bool ZipDownloadTask::Init(const char16* url,
                           const char16* version,
                           const char16* temp_path) {
  url_ = url;
  version_ = version;
  filename_ = temp_path;
  if (is_initialized_) return true;
  return AsyncTask::Init();
}

const char16* ZipDownloadTask::Filename() const {
  return filename_.c_str();
}

const char16* ZipDownloadTask::Version() const {
  return version_.c_str();
}

void ZipDownloadTask::Run() {
  WebCacheDB::PayloadInfo payload;
  scoped_refptr<BlobInterface> payload_data;
  bool was_redirected;
  std::string16 error_msg;
  std::string16 url;
  bool success = false;
  if (AsyncTask::HttpGet(url_.c_str(),
                         true,
                         NULL,
                         NULL,
                         NULL,
                         &payload,
                         &payload_data,
                         &was_redirected,
                         &url,
                         &error_msg)) {
    std::string16 xml;
    const std::string16 charset;
    // payload_data can be empty in case of a 30x response.
    // The update server does not redirect, so we treat this as an error.
    if (!was_redirected &&
        payload.PassesValidationTests(NULL) &&
        payload_data->Length() &&
        SaveToTempDirectory(payload_data.get())) {
      success = true;
    }
  }
  NotifyListener(DOWNLOAD_COMPLETE, success);
}

bool ZipDownloadTask::SaveToTempDirectory(BlobInterface *data) {
  assert(data != NULL);
  if (!File::RecursivelyCreateDir(filename_.c_str())) {
    LOG(("SaveToTempDirectory: Could not create temp directory."));
    return false;
  }

  filename_ += STRING16(PRODUCT_SHORT_NAME);
  filename_ += STRING16(L"-");
  filename_ += version_.c_str();
  filename_ += kArchiveExtension;

  scoped_ptr<File> zip_file(File::Open(filename_.c_str(),
                                       File::WRITE, File::NEVER_FAIL));

  if (!zip_file.get()) {
    LOG(("SaveToTempDirectory: Could not create zip file."));
    return false;
  }

  scoped_array<uint8> buffer;
  buffer.reset(new uint8[kBufferSizeBytes]);
  int64 offset = 0;
  int64 counter = 0;
  do  {
    counter = data->Read(buffer.get(), offset, kBufferSizeBytes);
    if (counter > 0) {
      zip_file->Write(buffer.get(), counter);
      offset += counter;
    }
  } while (kBufferSizeBytes == counter);

  if (counter < 0) {
    assert(counter == -1);
    LOG(("SaveToTempDirectory: Could not save to temp directory."));
    return false;
  }

  return true;
}
