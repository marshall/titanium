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

#ifndef GEARS_INSTALLER_ANDROID_ZIP_DOWNLOAD_TASK_H__
#define GEARS_INSTALLER_ANDROID_ZIP_DOWNLOAD_TASK_H__

#include "gears/base/common/common.h"
#include "gears/localserver/common/async_task.h"

class BlobInterface;

// A task that downloads a new version of Gears.
class ZipDownloadTask : public AsyncTask {
 public:
  ZipDownloadTask() : AsyncTask(NULL) {}
  bool Init(const char16 *url, const char16 *version, const char16 *temp_path);

  enum {
    DOWNLOAD_COMPLETE = 0,
  };

  // Returns the full path and name of the downloaded zip file.
  const char16* Filename() const;
  // Returns the version of the downloaded zip file.
  const char16* Version() const;

 private:
  // AsyncTask
  virtual void Run();

  // Saved the downloaded data to the temp directory.
  bool SaveToTempDirectory(BlobInterface *data);

  std::string16 filename_;
  std::string16 url_;
  std::string16 version_;

  DISALLOW_EVIL_CONSTRUCTORS(ZipDownloadTask);
};

#endif  // GEARS_INSTALLER_ANDROID_VERSION_CHECK_TASK_H__
