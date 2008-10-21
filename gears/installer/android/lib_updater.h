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

#ifndef GEARS_INSTALLER_ANDROID_LIB_UPDATER_H__
#define GEARS_INSTALLER_ANDROID_LIB_UPDATER_H__

#include "gears/base/common/common.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/thread.h"
#include "gears/localserver/common/async_task.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// This class takes care of periodic version updates. It does this as follows:
// 1. When the Gears plugin is loaded, the updater is started immediately
//    (NP_Initialize() in module.cc)
// 2. The updater performs a cleanup operation, deleting all resource
//    directories that belong to Gears versions different to the current one.
// 3. The updater sets up a timer and goes to sleep for some time.
// 4. When the updater wakes up, it does an httpS GET to the update server URL.
// 5. Next, the updater parses the XML document and extracts the upgrade URL.
// 6. The updater then does an httpS GET for the new Gears zip package.
// 7. The updater queries the zip package to get the uncompressed size of
//    each file in the package.
// 8. If 2 * (total uncompressed size) >= the free space on the device,
//    then delete the zip package and go to step 2.
// 9. The updater then unzips the package into a temporary directory,
// 10. Moves the unzipped files to app_plugins\gears-newVersionNumber,
// 11. Inside app_plugins\gears-newVersionNumber, creates a symlink to gears.so,
// 12. Atomically renames app_plugins\gears-newVersionNumber\gearsLink.so into
//     app_plugins\gearsLink.so and goes to step 2.

class VersionCheckTask;
class ZipDownloadTask;

class LibUpdater : public Thread,
                   public AsyncTask::Listener {
 public:
  // Constructs the updater singleton and starts the updater thread.
  static void StartUpdateChecks();
  // Stops the updater and destroys the updater singleton.
  static void StopUpdateChecks();

  virtual ~LibUpdater();

  // AsyncTask
  virtual void HandleEvent(int msg_code, int msg_param, AsyncTask *source);
 private:
  LibUpdater();

  // Thread
  virtual void Run();

  // Initiates and starts the version check task.
  bool StartVersionCheckTask();

  // Initiates and starts the zip download task.
  bool StartDownloadTask();

  // Records the time of the update check to a file.
  bool RecordUpdateTimestamp();

  // Determines the number of milliseconds until the next update is due.
  bool MillisUntilNextUpdateCheck(int *milliseconds_out);

  // Unzips the downloaded file.
  bool Inflate(const char16* filename);

  // Moves the Gears directory from the temp directory into the browser's
  // plugins directory. Updates the symlink to point to the new .so file.
  bool MoveFromTempAndUpdateSymlink(const char16* new_version);

  // Removes any stale Gears directories, plus the temp directory.
  bool PreCleanup();

  // Removes the temp directory.
  bool PostCleanup();

  ThreadId thread_id_;

  // The task that is used to talk to the update server and determine
  // if a new version of Gears is on the download server.
  scoped_ptr<VersionCheckTask> version_check_task_;

  // The task that is used to download the new Gears zip.
  scoped_ptr<ZipDownloadTask> download_task_;

  // Event used for waiting between updates.
  Event timer_event_;

  // Path to the temporary directory used by the updater.
  std::string16 temp_path_;

  static LibUpdater* instance_;

  DISALLOW_EVIL_CONSTRUCTORS(LibUpdater);
};

#endif  // GEARS_INSTALLER_ANDROID_LIB_UPDATER_H__
