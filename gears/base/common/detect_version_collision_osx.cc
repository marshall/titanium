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

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>

#include "gears/base/common/common.h"
#include "gears/base/common/detect_version_collision.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/safari/messagebox.h"
#include "gears/base/safari/scoped_cf.h"

#define PRODUCT_COLLISION_GUID L"{gears}"  // [naming]

static const char16 *kMutexName =
    STRING16(L"IsRunning" PRODUCT_COLLISION_GUID);
static const char16 *kOurVersionMutexName =
    STRING16(L"IsRunning" PRODUCT_COLLISION_GUID L"-" PRODUCT_VERSION_STRING);


// TODO(playmobil): merge common parts of this file with 
// detect_version_collision_win32.cc

// This class allows inter-process shared locks.
// When you create an instance of the class and call IncrementSharedLock()
// then a shared lock is acquired on the file, multiple processes
// can do this.
// LockExists() tries to obtain an exclusive lock on the file, which fails
// if a shared lock already exists.
// if a process crashes, it's lock is released.
class FileLock {
 public:
  FileLock() : fd_(-1), has_lock_(false) {}

  virtual ~FileLock() {
    Unlock();
  }
  
  // Try to obtain a shared lock on the file.
  // Returns false on error.
  bool IncrementSharedLock(const char16 *lock_name) {
    assert(fd_ == -1 && !has_lock_);
    
    if (!GetLockFilePath(lock_name, &lock_file_path_)) {
      return false;
    }

    std::string lock_file_path_utf8;
    if (!String16ToUTF8(lock_file_path_.c_str(), lock_file_path_.size(), 
                       &lock_file_path_utf8)) {
      return false;
    }
    
    // Attempt to lock.
    int fd = open(lock_file_path_utf8.c_str(), O_CREAT, S_IRWXU);
    if (fd < 0) return false;
    if (flock(fd, LOCK_SH | LOCK_NB) != 0) {
      close(fd);
      return false;
    }
    
    // Locked.
    fd_ = fd;
    has_lock_ = true;
    return true;
  }
  
  // Checks if the file is locked, returns false on error.
  static bool LockExists(const char16 *lock_name, bool *is_locked) {
    assert(is_locked);
    
    // Assemble full lock path.
    std::string16 lock_file_path;
    if (!GetLockFilePath(lock_name, &lock_file_path)) {
      return false;
    }

    // If lock file doesn't exist, then it isn't locked.
    if (!File::Exists(lock_file_path.c_str())) {
      *is_locked = false;
      return true;
    }

    std::string lock_file_path_utf8;
    if (!String16ToUTF8(lock_file_path.c_str(), lock_file_path.size(), 
                       &lock_file_path_utf8)) {
      return false;
    }
        
    // Try to gain an exclusive lock for the file, if a shared lock
    // already exists, then we won't get it.
    int fd = open(lock_file_path_utf8.c_str(), O_CREAT, S_IRWXU);
    if (fd < 0) return false;
    if (flock(fd, LOCK_EX | LOCK_NB) != 0) {
      *is_locked = true;
    } else {
      *is_locked = false;
    }
    close(fd);
    return true;
  }
  
  // Release lock.
  void Unlock() {
    if (!has_lock_) return;
    assert(fd_ >= 0);
    flock(fd_, LOCK_UN | LOCK_NB);
    has_lock_ = false;
    close(fd_);
    // TODO(playmobil): Delete lock files after use.
    fd_ = -1;
  }
    
 private:
  // Gets the full path of the named lock file in the tmp directory.
  // returns true on success.
  static bool GetLockFilePath(const char16 *lock_name, 
                              std::string16 *full_path) {
    if (!GetUserTempDirectory(full_path)) {
      return false;
    }
    *full_path += STRING16(L"/");
    *full_path += lock_name;
    return true;
  }
 
  int fd_;
  bool has_lock_;
  std::string16 lock_file_path_;
  
  DISALLOW_EVIL_CONSTRUCTORS(FileLock);
};

// We use two named mutex objects to determine if another version of the same
// Gears distribution is running.  The first indicates an instance of ANY
// version is running.  The second indicates an instance of OUR version is
// running.  The base mutex names should not change across versions!
static FileLock running_mutex;
static FileLock our_version_running_mutex;

static bool OneTimeDetectVersionCollision();
static bool detected_collision = OneTimeDetectVersionCollision();

bool DetectedVersionCollision() {
  return detected_collision;
}

// Returns true if we detect that a different version of Gears is running.
// If no collision is detected, leaves FileLock locked to indicate that
// our version is running.  If a collision is detected, this instance of Gears
// will be crippled, so we close all locks so others don't see this instance as
// 'running'. Should only be called once.
static bool OneTimeDetectVersionCollision() {
  // Attempt to lock.
  bool already_running = false;
  
  if (!FileLock::LockExists(kMutexName, &already_running)) {
    return false;
  }
  if (!running_mutex.IncrementSharedLock(kMutexName)) {
    return false;
  }
  
  bool our_version_already_running = false;
  if (!FileLock::LockExists(kOurVersionMutexName, 
      &our_version_already_running)) {
    return false;
  }
  if (!our_version_running_mutex.IncrementSharedLock(kOurVersionMutexName)) {
    return false;
  }

  if (!already_running) {
    // No collision, we are the first instance to run
    assert(!our_version_already_running);
    return false;
  } else if (our_version_already_running) {
    // No collision, other instances of our version are running
    return false;
  } else {
    // A collision with a different version!
    our_version_running_mutex.Unlock();
    running_mutex.Unlock();
    return true;
  }
}

// Low tech UI to notify the user
static bool alerted_user = false;

void MaybeNotifyUserOfVersionCollision() {
  assert(detected_collision);
  if (!alerted_user) {
    NotifyUserOfVersionCollision();
  }
}

void NotifyUserOfVersionCollision() {
  assert(detected_collision);
  alerted_user = true;
  
  // TODO(playmobil): Load from internationalized string table.
  const char16 *kTitle = STRING16(L"Please restart your browser");
  const char16 *kMessage = STRING16(L"A " PRODUCT_FRIENDLY_NAME 
                           L" update has been downloaded.\n"
                           L"\n"
                           L"Please close all browser windows"
                           L" to complete the upgrade process.\n");  
  MessageBox(kTitle, kMessage);
}
