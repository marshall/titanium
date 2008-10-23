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

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include "gears/notifier/user_activity.h"

#include "gears/base/common/timed_call.h"

// Constants.
static const size_t kDefaultUserIdleThresholdSec = 5 * 60;    // 5m
static const size_t kDefaultUserBusyThresholdSec = 30;        // 30s

// In Windows platform, the balloon will reappear shortly after the use
// enters full-screen mode. To work around this, we use a much shorter 
// interval.
#ifdef WIN32
static const int64 kUserActivityCheckIntervalMs = 500;    // 0.5s
#else
static const int64 kUserActivityCheckIntervalMs = 1000;   // 1s
#endif  // WIN32

UserActivityMonitor::UserActivityMonitor()
  : user_mode_(USER_MODE_UNKNOWN) {
  timer_.reset(new TimedCall(kUserActivityCheckIntervalMs,
                             true,
                             UserActivityMonitor::OnTimer,
                             this));
}

UserActivityMonitor::~UserActivityMonitor() {
}

void UserActivityMonitor::AddObserver(UserActivityObserver *observer) {
  assert(observer);
  observers_.push_back(observer);
}

void UserActivityMonitor::OnTimer(void *arg) {
  assert(arg);

  UserActivityMonitor *this_ptr = reinterpret_cast<UserActivityMonitor*>(arg);
  this_ptr->CheckNow();
}

bool UserActivityMonitor::IsUserIdle() {
  uint32 idle_threshold_sec = kDefaultUserIdleThresholdSec;
  uint32 power_off_sec = GetMonitorPowerOffTimeSec();
  if (power_off_sec != 0 && power_off_sec < idle_threshold_sec) {
    idle_threshold_sec = power_off_sec;
  }

  return GetUserIdleTimeMs() > idle_threshold_sec * 1000;
}

bool UserActivityMonitor::IsUserBusy() {
  return GetUserIdleTimeMs() < kDefaultUserBusyThresholdSec * 1000;
}

bool UserActivityMonitor::IsUserAway() {
  return IsScreensaverRunning() || IsWorkstationLocked();
}

void UserActivityMonitor::CheckNow() {
  UserMode previous_user_mode = user_mode_;
  user_mode_ = GetUserActivity();
  if (previous_user_mode != user_mode_) {
    for (size_t i = 0; i < observers_.size(); ++i) {
      observers_[i]->OnUserActivityChange();
    }
  }
}

UserMode UserActivityMonitor::GetUserActivity() {
  // Use platform specific way of detecting user mode.
  UserMode user_mode = PlatformDetectUserMode();
  if (user_mode != USER_MODE_UNKNOWN) {
    return user_mode;
  }

  // Check if the user is away.
  // Note that this check should be done before fullscreen check because
  // when screensaver is running, it could be in fullscreen window.
  if (IsUserAway()) {
    return USER_AWAY_MODE;
  }

  // Check if the user is in fullscreen mode.
  if (IsFullScreenMode()) {
    return USER_PRESENTATION_MODE;
  }

  // Check if the user is idling.
  return IsUserIdle() ? USER_IDLE_MODE : USER_NORMAL_MODE;
}

#endif  // OFFICIAL_BUILD
