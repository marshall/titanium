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

#ifndef GEARS_INSTALLER_ANDROID_VERSION_CHECK_TASK_H__
#define GEARS_INSTALLER_ANDROID_VERSION_CHECK_TASK_H__

#include "gears/base/android/java_jni.h"
#include "gears/base/common/common.h"
#include "gears/localserver/common/async_task.h"

// A simple task that checks for a new Gears version.
class VersionCheckTask : public AsyncTask {
 public:
  VersionCheckTask() : AsyncTask(NULL) {}
  bool Init();

  enum {
    VERSION_CHECK_TASK_COMPLETE = 0,
  };

  // Returns the URL of the Gears zip package.
  const char16* Url() const;
  // Returns the version number of Gears on the update server.
  const char16* Version() const;

 private:
  // AsyncTask
  virtual void Run();

  // Parses XML and extracts the Gears version number and download URL.
  bool ExtractVersionAndDownloadUrl(const std::string16& xml);

  // Native callback, invoked if the parsing of the XML succeeds.
  static void SetVersionAndUrl(JNIEnv* env, jclass cls, jstring version,
                               jstring url, jlong self);

  static JNINativeMethod native_methods_[];

  std::string16 url_;
  std::string16 version_;

  DISALLOW_EVIL_CONSTRUCTORS(VersionCheckTask);
};

#endif  // GEARS_INSTALLER_ANDROID_VERSION_CHECK_TASK_H__
