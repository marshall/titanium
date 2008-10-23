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

#ifndef GEARS_NOTIFIER_NOTIFIER_GROWL_PREFS_H__
#define GEARS_NOTIFIER_NOTIFIER_GROWL_PREFS_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#if defined(OS_MACOSX)

#include <CoreFoundation/CoreFoundation.h>

#include "gears/base/common/basictypes.h"

class NotificationManager;

// Listens for notifications from the prefpane indicating that the user has
// clicked the Growl checkbox.  Lives in Notifier.app.
class NotifierPrefListener {
 public:
  // Registers for pref changes.
  static void RegisterForPreferenceChanges(NotificationManager *manager);

  // Removes listener from distributed notifications.
  static void UnregisterForPreferenceChanges();

 private:
  // static class, constructors should never be used.
  NotifierPrefListener();
  ~NotifierPrefListener();

  static void PrefChangeCallback(CFNotificationCenterRef center,
                                 void *observer,
                                 CFStringRef name,
                                 const void *object,
                                 CFDictionaryRef user_info);

  static NotificationManager *notification_manager_;

  DISALLOW_EVIL_CONSTRUCTORS(NotifierPrefListener);
};

#endif  // OS_MACOSX
#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_NOTIFIER_GROWL_PREFS_H__
