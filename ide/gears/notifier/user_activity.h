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
//
// Definitions for detecting user activity.

#ifndef GEARS_NOTIFIER_USER_ACTIVITY_H__
#define GEARS_NOTIFIER_USER_ACTIVITY_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include <vector>
#include "gears/base/common/basictypes.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class TimedCall;

// Categorization of the user mode
enum UserMode {
  // User is away
  // * A screen saver is displayed.
  // * The machine is locked.
  // * Non-active Fast User Switching session is on (Windows XP & Vista).
  USER_AWAY_MODE = 0,

  // User is idle
  USER_IDLE_MODE,

  // User is interrupted by some critical event
  // * Laptop power is being suspended.
  USER_INTERRUPTED_MODE,

  // User is in not idle
  USER_NORMAL_MODE,

  // User is in full-screen presentation mode
  // * A full-screen application is running.
  // * A full-screen (exclusive mode) Direct3D application is running
  //  (Windows XP & Vista).
  // * The user activates Windows presentation settings (Windows Vista).
  USER_PRESENTATION_MODE,

  // User mode is undetermined
  USER_MODE_UNKNOWN,
};

class UserActivityObserver {
 public:
  // Called when there is any change to user activity.
  virtual void OnUserActivityChange() = 0;
};

class UserActivityInterface {
 public:
  // Add the observer to be notified when a user mode has been changed.
  virtual void AddObserver(UserActivityObserver *observer) = 0;

  // Check the user activity immediately.
  virtual void CheckNow() = 0;

  // Get the user activity value. Note that this is the latest cached value.
  virtual UserMode user_mode() const = 0; 

  // Returns the number of milliseconds the system has had no user input.
  virtual uint32 QueryUserIdleTimeMs() = 0;
};

class UserActivityMonitor : public UserActivityInterface {
 public:
  // Creator.
  static UserActivityMonitor *Create();

  UserActivityMonitor();
  virtual ~UserActivityMonitor();

  // UserActivityInterface implementations.
  virtual void AddObserver(UserActivityObserver *observer);
  virtual void CheckNow();
  virtual UserMode user_mode() const { return user_mode_; }
  virtual uint32 QueryUserIdleTimeMs() { return GetUserIdleTimeMs(); }

 protected:
  // Gets the user mode by using platform-specific function if possible.
  virtual UserMode PlatformDetectUserMode() = 0;

  // Returns the number of seconds for the monitor power off.
  virtual uint32 GetMonitorPowerOffTimeSec() = 0;

  // Returns the number of milliseconds the system has had no user input.
  virtual uint32 GetUserIdleTimeMs() = 0;

  // Returns true if screen saver is running.
  virtual bool IsScreensaverRunning() = 0;

  // Returns true if workstation is locked.
  virtual bool IsWorkstationLocked() = 0;

  // Returns true if in full screen mode.
  virtual bool IsFullScreenMode() = 0;

 private:
  // Get the current user activity mode.
  UserMode GetUserActivity();

  // Returns true if the user is idle.
  bool IsUserIdle();

  // Returns true if the user is busy.
  bool IsUserBusy();

  // Returns true if the user is away.
  bool IsUserAway();

  // Callback for the timer to check the user activity peridocially.
  static void OnTimer(void *arg);

  scoped_ptr<TimedCall> timer_;
  std::vector<UserActivityObserver*> observers_;
  UserMode user_mode_;
  DISALLOW_EVIL_CONSTRUCTORS(UserActivityMonitor);
};

// Is user active?
inline bool IsActiveUserMode(UserMode user_mode) {
  return user_mode == USER_NORMAL_MODE;
}

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_USER_ACTIVITY_H__
