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

#include "gears/installer/android/version_check_task.h"

#include "gears/base/android/java_class.h"
#include "gears/base/android/java_exception_scope.h"
#include "gears/base/android/java_global_ref.h"
#include "gears/base/android/java_local_frame.h"
#include "gears/base/android/java_string.h"
#include "gears/blob/blob_interface.h"
#include "gears/blob/blob_utils.h"

#define ANDROID_UID L"%7B000C0320-A01D-4D7E-B3C9-66B2ACB7FF80%7D"

static const char16* kUpgradeUrl = STRING16 (
    L"https://tools.google.com/service/update2/ff?"
    L"guid=" ANDROID_UID
    L"&version=" PRODUCT_VERSION_STRING
    L"&application=" ANDROID_UID
    L"&appversion=1.0"
#ifdef OFFICIAL_BUILD
    L"&os=android&dist=google");
#else
    L"&os=android&dist=google&dev=1");
#endif

static const char* kVersionExtractorClass =
   GEARS_JAVA_PACKAGE  "/VersionExtractor";

// Java methods used by the version check task class.
static JavaClass::Method version_extractor_methods[] = {
  { JavaClass::kStatic, "extract", "(Ljava/lang/String;J)Z"},
};

enum VersionExtractorMethodID {
  VERSION_EXTRACTOR_METHOD_ID_EXTRACT = 0,
};

// The native method declared by the version check task.
JNINativeMethod VersionCheckTask::native_methods_[] = {
  {"setVersionAndUrl",
   "(Ljava/lang/String;Ljava/lang/String;J)V",
   reinterpret_cast<void*>(VersionCheckTask::SetVersionAndUrl)
  },
};

bool VersionCheckTask::Init() {
  if (is_initialized_) return true;
  return AsyncTask::Init();
}

const char16* VersionCheckTask::Version() const {
  return version_.c_str();
}

const char16* VersionCheckTask::Url() const {
  return url_.c_str();
}

// AsyncTask
void VersionCheckTask::Run() {
  WebCacheDB::PayloadInfo payload;
  scoped_refptr<BlobInterface> payload_data;
  bool was_redirected;
  std::string16 error_msg;
std::string16 url;
  bool success = false;
  if (AsyncTask::HttpGet(kUpgradeUrl,
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
        BlobToString16(payload_data.get(), charset, &xml)) {
      if (ExtractVersionAndDownloadUrl(xml)) {
        success = true;
      }
    }
  }
  NotifyListener(VERSION_CHECK_TASK_COMPLETE, success);
}

bool VersionCheckTask::ExtractVersionAndDownloadUrl(const std::string16& xml) {
  JavaExceptionScope scope;
  JavaLocalFrame frame;
  JavaClass extractor_java_class;

  if (!extractor_java_class.FindClass(kVersionExtractorClass)) {
    LOG(("Could not find the VersionExtractor class.\n"));
    assert(false);
    return false;
  }

  // Register the native callback.
  jniRegisterNativeMethods(JniGetEnv(),
                           kVersionExtractorClass,
                           native_methods_,
                           NELEM(native_methods_));


  // Get the Java method ID.
  if (!extractor_java_class.GetMultipleMethodIDs(
          version_extractor_methods, NELEM(version_extractor_methods))) {
    LOG(("Could not find the VersionExtractor methods.\n"));
    assert(false);
    return false;
  }

  jlong native_object_ptr = reinterpret_cast<jlong>(this);
  jboolean result = JniGetEnv()->CallStaticBooleanMethod(
      extractor_java_class.Get(),
      version_extractor_methods[VERSION_EXTRACTOR_METHOD_ID_EXTRACT].id,
      JavaString(xml).Get(),
      native_object_ptr);

  return result;
}

// static
void VersionCheckTask::SetVersionAndUrl(JNIEnv* env,
                                        jclass cls,
                                        jstring version,
                                        jstring url,
                                        jlong self) {

  JavaString version_string(version);
  JavaString url_string(url);
  VersionCheckTask* task = reinterpret_cast<VersionCheckTask*>(self);

  // version, url and self must not be NULL. Currently there seems to
  // be a bug in JNI that manifests itself by creating jstrings that
  // point to a string containing the word "null" instead of the
  // jstrings being NULL themselves. TODO(andreip): remove the
  // comparison against "null" once the bug is fixed.
  assert(version_string.Get() != NULL &&
         version_string.ToString8().compare("null") != 0);

  assert(url_string.Get() != NULL &&
         url_string.ToString8().compare("null") != 0);

  assert(task);

  task->version_ = version_string.ToString16();
  task->url_ = url_string.ToString16();
}
