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
// WHETHER IN CONTRACT,s STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#if defined(OS_MACOSX)

#include "gears/notifier/notifier_pref_listener.h"
#include "gears/notifier/notification_manager.h"
#include "gears/notifier/notifier_pref_common.h"

NotificationManager *NotifierPrefListener::notification_manager_ = NULL;

void NotifierPrefListener::PrefChangeCallback(CFNotificationCenterRef center,
                                              void *observer,
                                              CFStringRef name,
                                              const void *object,
                                              CFDictionaryRef user_info) {
  assert(notification_manager_);
  bool use_growl = ShouldUseGrowlPref();
  notification_manager_->SetDoesUseGrowlBalloonCollection(use_growl);
}

void NotifierPrefListener::RegisterForPreferenceChanges(
    NotificationManager *manager) {
  assert(manager);
  notification_manager_ = manager;

  CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();

  CFNotificationCenterAddObserver(
        center,
        NULL,
        NotifierPrefListener::PrefChangeCallback,
        kGearsNotifierPrefChangedNotification,
        kGearsNotifierAppID,
        CFNotificationSuspensionBehaviorDeliverImmediately);
}

void NotifierPrefListener::UnregisterForPreferenceChanges() {
  CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();

  CFNotificationCenterRemoveObserver(center,
                                     NULL,
                                     kGearsNotifierPrefChangedNotification,
                                     kGearsNotifierAppID);
}

#endif  // OS_MACOSX
#endif  // OFFICIAL_BUILD
