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

#include "gears/installer/android/lib_updater.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "gears/base/android/java_class.h"
#include "gears/base/android/java_exception_scope.h"
#include "gears/base/android/java_global_ref.h"
#include "gears/base/android/java_local_frame.h"
#include "gears/base/android/java_jni.h"
#include "gears/base/android/java_string.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/stopwatch.h"
#include "gears/installer/android/version_check_task.h"
#include "gears/installer/android/zip_download_task.h"

static const char16* kUpdateTimestampFile =
     STRING16(L"/" PRODUCT_SHORT_NAME L"timestamp");

static const char16* kTempDirectory =
    STRING16(L"/" PRODUCT_SHORT_NAME L"temp/");

static const char16* kGearsLibrary =
    STRING16(L"/" PRODUCT_SHORT_NAME L".so");

static const char16* kGearsLibrarySymlink =
    STRING16(L"/" PRODUCT_SHORT_NAME L".so.sym");

// The delay before the first update.
static const int kFirstUpdateDelayMs = (1000 * 60 * 2);  // 2 minutes

// The time interval between the rest of the update checks.
static const int kUpdatePeriodMs = (1000 * 60 * 60 * 25);  // 25 hours

static const char* kZipInflaterClass =
    GEARS_JAVA_PACKAGE "/ZipInflater";

static const char* kZipInflateMethodName = "inflate";

static const char* kZipInflateMethodSignature =
    "(Ljava/lang/String;Ljava/lang/String;)Z";

//------------------------------------------------------------------------------
// LibUpdater

LibUpdater* LibUpdater::instance_ = NULL;

// static
void LibUpdater::StartUpdateChecks() {
  assert(instance_ == NULL);
  instance_ = new LibUpdater();
}

// static
void LibUpdater::StopUpdateChecks() {
  delete instance_;
  instance_ = NULL;
}

LibUpdater::~LibUpdater() {
  version_check_task_->Abort();
  version_check_task_->DeleteWhenDone();
  version_check_task_.release();

  download_task_->Abort();
  download_task_->DeleteWhenDone();
  download_task_.release();

  timer_event_.Signal();
  AndroidMessageLoop::Stop(thread_id_);
  Join();
  LOG(("Gears updater stopped."));
}

// AsyncTask
void LibUpdater::HandleEvent(int msg_code,
                             int msg_param,
                             AsyncTask *source) {
  if (source) {
    bool success = static_cast<bool>(msg_param);
    if (source == version_check_task_.get()) {
      assert(VersionCheckTask::VERSION_CHECK_TASK_COMPLETE == msg_code);
      if (success && StartDownloadTask()) {
        // We have a new version of Gears and the download task was
        // started successfully
        return;
      }  // else fallthru to StartVersionCheckTask again after kUpdatePeriodMs.
    } else if (source == download_task_.get()) {
      assert(ZipDownloadTask::DOWNLOAD_COMPLETE == msg_code);
      if (success) {
        // Unzip the file.
        // TODO(andreip): check for free space first.
        if (Inflate(download_task_->Filename())) {
          // Move the files and update the symlink. This should not leave
          // any stale Gears directory behind if it fails.
          if (!MoveFromTempAndUpdateSymlink(download_task_->Version())) {
            LOG(("Failed to move files"));
          }
        } else {
          LOG(("Failed to unzip files"));
        }
        // Remove the temp directory.
        PostCleanup();
      }
    } else {
      // We're not listening to any other tasks.
      LOG(("Unknown task."));
      assert(false);
      return;
    }

    // Either a version check failed or the download task was
    // completed. Record the timestamp and wait some time before
    // doing another update check.
    RecordUpdateTimestamp();
    if (timer_event_.WaitWithTimeout(kUpdatePeriodMs)) {
      // timer_event_ must have been signaled directly in the destructor.
      return;
    }

    // kUpdatePeriodMs has elapsed. Start the version check task again.
    StartVersionCheckTask();
  }
}

LibUpdater::LibUpdater() : version_check_task_(new VersionCheckTask()),
                           download_task_(new ZipDownloadTask()) {
  if (!GetInstallDirectory(&temp_path_)) {
    LOG(("LibUpdater ctor: Could not get resources directory."));
    assert(false);
  };

  temp_path_ += kTempDirectory;

  thread_id_ = Start();
  assert(thread_id_ != 0);
}

// Thread
void LibUpdater::Run() {
  LOG(("Gears updater started."));
  if (!PreCleanup()) {
    LOG(("Pre-cleanup operation failed."));
  }

  int delay = 0;
  // Determine what the initial delay should be.
  if (!MillisUntilNextUpdateCheck(&delay)) {
    LOG(("Run: Failed to get the initial delay."));
    delay = kFirstUpdateDelayMs;
  }

  // Wait for delay millis.
  if (!timer_event_.WaitWithTimeout(delay)) {
    // the wait timed out. Start the version task.
    StartVersionCheckTask();
  }

  // Run the message loop.
  AndroidMessageLoop::Start();
}

bool LibUpdater::StartVersionCheckTask() {
  // Initialize the task.
  if (!version_check_task_->Init()) {
    // The task could not be initialized.
    LOG(("StartVersionCheckTask:: Could not initialize version check task."));
    return false;
  }
  // Set ourselves as the listener.
  version_check_task_->SetListener(this);
  // Start the task.
  return version_check_task_->Start();
}

bool LibUpdater::StartDownloadTask() {
  // Initialize the task.
  if (!download_task_->Init(version_check_task_->Url(),
                            version_check_task_->Version(),
                            temp_path_.c_str())) {
    // The task could not be initialized.
    LOG(("StartDownloadTask:: Could not initialize download task."));
    return false;
  }

  // Set ourselves as the listener.
  download_task_->SetListener(this);
  // Start the task.
  return download_task_->Start();
}

bool LibUpdater::RecordUpdateTimestamp() {
  std::string16 timestamp_file_path;
  if (!GetBaseResourcesDirectory(&timestamp_file_path)) {
    LOG(("RecordUpdateTimestamp: Could not get resources directory."));
    return false;
  }

  timestamp_file_path += kUpdateTimestampFile;

  // Create the timestamp file if it does not exist and open it.
  // This will set / update its modified time.
  scoped_ptr<File> timestamp_file(
      File::Open(timestamp_file_path.c_str(), File::WRITE, File::NEVER_FAIL));

  if (!timestamp_file.get()) {
    std::string timestamp_file_utf8;
    String16ToUTF8(timestamp_file_path.c_str(), &timestamp_file_utf8);
    LOG(("RecordUpdateTimestamp: Failed to create or open timestamp file: %s",
         timestamp_file_utf8.c_str()));

    LOG(("RecordUpdateTimestamp: Errno is : %d", errno));
    return false;
  }

  if (!timestamp_file->Truncate(0)) {
    LOG(("RecordUpdateTimestamp: Failed to touch timestamp file."));
  }

  return true;
}

bool LibUpdater::MillisUntilNextUpdateCheck(int *milliseconds_out) {
  std::string16 timestamp_file_path;
  if (!GetBaseResourcesDirectory(&timestamp_file_path)) {
    LOG(("MillisUntilNextUpdateCheck: Could not get resources directory."));
    return false;
  }

  timestamp_file_path += kUpdateTimestampFile;

  std::string timestamp_file_path_utf8;
  if (!String16ToUTF8(timestamp_file_path.c_str(), &timestamp_file_path_utf8)) {
    LOG(("MillisUntilNextUpdateCheck: Could not convert path strings."));
    return false;
  }

  struct stat statbuf;
  if (stat(timestamp_file_path_utf8.c_str(), &statbuf)) {
    LOG(("MillisUntilNextUpdateCheck: Could not stat timestamp file: %s",
         timestamp_file_path_utf8.c_str()));
    return false;
  }

  int64 last_update_check_millis = statbuf.st_mtime * 1000LL;
  int64 now = GetCurrentTimeMillis();
  int64 elapsed = now - last_update_check_millis;

  if (elapsed < 0) {
    // Bizarre situation when the timestamp file appears to have been modified
    // in the future.
    LOG(("MillisUntilNextUpdateCheck: update timestamp from the future!"));
    *milliseconds_out = kFirstUpdateDelayMs;
    return true;
  }

  if (elapsed > kUpdatePeriodMs) {
    // An update is overdue.
    *milliseconds_out = kFirstUpdateDelayMs;
    return true;
  }

  *milliseconds_out = kUpdatePeriodMs - static_cast<int>(elapsed);

  if (*milliseconds_out < kFirstUpdateDelayMs) {
    // Avoid starting the update immediately so that we don't slow
    // down the browser.
    *milliseconds_out = kFirstUpdateDelayMs;
  }

  return true;
}

bool LibUpdater::Inflate(const char16* filename) {
  JavaExceptionScope scope;
  JavaLocalFrame frame;
  JavaClass inflater_java_class;

  if (!inflater_java_class.FindClass(kZipInflaterClass)) {
    LOG(("Could not find the ZipInflater class.\n"));
    assert(false);
    return false;
  }

  // Get the Java method ID.
  jmethodID inflate_id = inflater_java_class.GetMethodID(
      JavaClass::kStatic,
      kZipInflateMethodName,
      kZipInflateMethodSignature);

  if (inflate_id == 0) {
    LOG(("Could not find the ZipInflater methods.\n"));
    assert(false);
    return false;
  }

  return JniGetEnv()->CallStaticBooleanMethod(
      inflater_java_class.Get(),
      inflate_id,
      JavaString(filename).Get(),
      JavaString(temp_path_).Get());
}

bool LibUpdater::MoveFromTempAndUpdateSymlink(const char16* new_version) {
  std::string16 install_dir;
  if (!GetInstallDirectory(&install_dir)) {
    LOG(("Could not get the install directory."));
    return false;
  }

  RemoveName(&install_dir);
  install_dir += STRING16(L"/");

  // The convention is that Gears lives in a directory named 'gears-versionNr'.
  std::string16 new_gears_dir;
  new_gears_dir += STRING16(PRODUCT_SHORT_NAME);
  new_gears_dir += STRING16(L"-");
  new_gears_dir += new_version;

  // gears_dest is /data/data/com.browser.android/app_plugins/gears-newVersion.
  std::string16 gears_dest = new_gears_dir;
  gears_dest.insert(0, install_dir);

  // gears_src is
  // /data/data/com.browser.android/app_plugins/gearstemp/gears-newVersion.
  std::string16 gears_src = new_gears_dir;
  gears_src.insert(0, temp_path_);

  // Update new_gears_dir so we can delete it easily if something goes wrong.
  // The gears_dest variable gets reused later.
  new_gears_dir = gears_dest;

  // Convert to UTF8.
  std::string gears_src_utf8;
  if (!String16ToUTF8(gears_src.c_str(), &gears_src_utf8)) {
    LOG(("Could not convert source directory name string."));
    return false;
  }

  std::string gears_dest_utf8;
  if (!String16ToUTF8(gears_dest.c_str(), &gears_dest_utf8)) {
    LOG(("Could not convert destination directory name string."));
    return false;
  }
  // Move the directory.
  int rez = rename(gears_src_utf8.c_str(), gears_dest_utf8.c_str());
  if (rez != 0) {
    LOG(("Could not move from temp: %d", errno));
    LOG(("Old was %s", gears_src_utf8.c_str()));
    LOG(("New was %s", gears_dest_utf8.c_str()));
    return false;
  }

  // Now create a new symlink to point to the new Gears file.
  gears_src = gears_dest + kGearsLibrary;
  gears_dest += kGearsLibrarySymlink;
  if (!String16ToUTF8(gears_src.c_str(), &gears_src_utf8)) {
    LOG(("Could not convert source symlink string."));
    File::DeleteRecursively(new_gears_dir.c_str());
    return false;
  }

  if (!String16ToUTF8(gears_dest.c_str(), &gears_dest_utf8)) {
    LOG(("Could not convert destination symlink name string."));
    File::DeleteRecursively(new_gears_dir.c_str());
    return false;
  }

  LOG(("Symlink source: %s", gears_src_utf8.c_str()));
  LOG(("Symlink dest: %s", gears_dest_utf8.c_str()));
  rez = symlink(gears_src_utf8.c_str(), gears_dest_utf8.c_str());
  if (rez != 0) {
    LOG(("Could not create symlink: %d", errno));
    File::DeleteRecursively(new_gears_dir.c_str());
    return false;
  }
  LOG(("Symlink created successfully"));
  // Finally atomically move the symlink.
  gears_src_utf8 = gears_dest_utf8;
  install_dir += kGearsLibrary;
  if (!String16ToUTF8(install_dir.c_str(), &gears_dest_utf8)) {
    LOG(("Could not convert the symlink name string."));
    File::DeleteRecursively(new_gears_dir.c_str());
    return false;
  }

  rez = rename(gears_src_utf8.c_str(), gears_dest_utf8.c_str());
  if (rez != 0) {
    LOG(("Could not move symlink: %d", errno));
    File::DeleteRecursively(new_gears_dir.c_str());
    return false;
  }

  return true;
}

bool LibUpdater::PreCleanup() {
  std::string16 install_dir;
  if (!GetInstallDirectory(&install_dir)) {
    LOG(("Could not get the install directory."));
    return false;
  }

  RemoveName(&install_dir);
  install_dir += STRING16(L"/");

  std::string install_dir_utf8;
  if (!String16ToUTF8(install_dir.c_str(), &install_dir_utf8)) {
    LOG(("Could not convert install directory name to UTF8."));
    return false;
  }

  // Open a directory iterator
  DIR* dir = opendir(install_dir_utf8.c_str());
  if(!dir) {
    LOG(("Could not open install directory."));
    return false;
  }

  // Scan the directory for "gears-oldVersionNr" subdirectories.
  std::string search_pattern = install_dir_utf8 + PRODUCT_SHORT_NAME_ASCII "-";
  std::string current_directory_utf8;
  std::vector<std::string16> dirs_to_delete;
  struct dirent* entry;
  while((entry = readdir(dir)) != NULL) {
    const char* name = entry->d_name;
    // Skip current and parent directory entries and anything that is not
    // a directory.
    if(!strcmp(name, ".") || !strcmp(name, "..") || entry->d_type != DT_DIR) {
      continue;
    }

    current_directory_utf8 = install_dir_utf8 + name;
    LOG(("Processing: %s", current_directory_utf8.c_str()));
    if (current_directory_utf8.find(search_pattern) == 0 &&
        current_directory_utf8.find(PRODUCT_VERSION_STRING_ASCII) ==
        std::string::npos) {
      LOG(("Found stale Gears directory: %s", current_directory_utf8.c_str()));
      std::string16 current_directory;
      if (UTF8ToString16(current_directory_utf8.c_str(), &current_directory)) {
        dirs_to_delete.push_back(current_directory);
      }
    }
  }

  closedir(dir);
  int result = true;

  for(std::vector<std::string16>::size_type i = 0;
      i < dirs_to_delete.size();
      ++i) {
    result &= File::DeleteRecursively(dirs_to_delete[i].c_str());
  }

  return result;
}

bool LibUpdater::PostCleanup() {
  return File::DeleteRecursively(temp_path_.c_str());
}
