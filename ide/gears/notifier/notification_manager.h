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
// Manages all of the desktop notifications.

#ifndef GEARS_NOTIFIER_NOTIFICATION_MANAGER_H__
#define GEARS_NOTIFIER_NOTIFICATION_MANAGER_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include <deque>

#include "gears/base/common/basictypes.h"
#include "gears/base/common/string16.h"
#include "gears/notifier/balloons.h"
#include "gears/notifier/user_activity.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#if USING_CCTESTS
class BalloonCollectionMock;
#endif  // USING_CCTESTS
class DelayedRestartInterface;
class File;
class GearsNotification;
class QueuedNotification;

// Handles all aspects of the notifications to be displayed.
// Note: do not forget to increase kNotificationManagerVersion if you make any
// change to this class which could affect saving notifications.
class NotificationManager : public BalloonCollectionObserver,
                            public UserActivityObserver {
 public:
  NotificationManager(UserActivityInterface *activity,
                      DelayedRestartInterface *delayed_restart,
                      BalloonCollectionObserver *balloons_observer);
  ~NotificationManager();

  // Adds/Updates a notification (which may already be displayed).
  // If a notification has already been displayed and is gone, then
  // this will put it in the queue to show it again.
  void Add(const GearsNotification &notification);

  // Adds the notifications to be displayed in the future.  Used for
  // snooze operations.
  void AddWithDelay(const GearsNotification &notification, int delay_ms);

  // Returns
  //   True if a match for the notification information was found
  //     (either in the queue or on the display).
  //   False if no match was found.
  bool Delete(const SecurityOrigin &security_origin,
              const std::string16 &id);

  // Should only be used by QueuedNotification to move itself
  // from being delayed to being ready to show (as a result of
  // "showAtTime" arriving or a snooze period being over).
  void MoveFromDelayedToShowQueue(const SecurityOrigin &security_origin,
                                  const std::string16 &id);

  // BalloonCollectionObserver implementation.
  virtual void OnBalloonSpaceChanged();
  virtual void OnSnoozeNotification(const GearsNotification &notification,
                                    int snooze_seconds);

  // UserActivityObserver implementation.
  void OnUserActivityChange();

  // Saves notifications so that they can be reloaded when notifier is
  // restarted.
  bool SaveNotifications();

  // Loads notifications during restart.
  bool LoadNotifications();

  // Are we showing any notifications?
  bool showing_notifications() const;

  // Do we have any pending notifications to show?
  bool has_pending_notifications() const;

#if USING_CCTESTS
  BalloonCollectionMock *UseBalloonCollectionMock();
#endif  // USING_CCTESTS

#if defined(OS_MACOSX)
  void SetDoesUseGrowlBalloonCollection(bool should_use_growl);
#endif

 private:
  static const int kNotificationManagerVersion;

  // Attempts to cdisplay notifications from the show_queue if the user
  // is active.
  void CheckAndShowNotifications();

  // Attempts to display notifications from the show_queue.
  void ShowNotifications();

  // Finds a notification with the given security_origin/id and optionally
  // removes it (if "and_remove"), which would transfer ownership
  // to the caller.
  QueuedNotification *Find(const SecurityOrigin &security_origin,
                           const std::string16 &id,
                           bool and_remove);

  // Puts the notification in the show queue.
  // Takes ownership of "queued_notification" (and ensures that no
  // timer is set-up for the queued_notification).
  //
  // A notification with the same security_origin/id shouldn't
  // be in any queues.
  void AddToShowQueue(QueuedNotification *queued_notification);

  // Puts the notification in the delayed queue and sets-up a timer for it.
  // Takes ownership of "queued_notification".
  //
  // A notification with the same security_origin/id shouldn't
  // be in any queues.
  void AddToDelayedQueue(QueuedNotification *queued_notification,
                         int64 delay_ms,
                         bool user_delayed);

  // Find the notification with the given security_origin/id and move it to
  // the delayed queue and sets-up a timer for it.
  //
  // A notification with the same security_origin/id must be in a queue.
  void MoveToDelayedQueue(const SecurityOrigin &security_origin,
                          const std::string16 &id,
                          int64 delay_ms,
                          bool user_delayed);

  // Check if we need to do the delayed restart.
  bool CheckDelayedRestart();

  // Helper function to load notifications.
  bool InternalLoadNotifications(File *file);

  // Helper function to get the path for saving notifications.
  static bool GetNotificationSavePath(std::string16 *file_path,
                                      bool create_if_missing);

  // Helper function to clean up the path used to save notifications.
  static void CleanupNotificationSavePath(const std::string16 &file_path);

  // Helper function to write a notification.
  static bool WriteNotification(File *file,
                                const GearsNotification &notification);

  bool initialized_;
  UserActivityInterface *activity_;
  DelayedRestartInterface *delayed_restart_;
  BalloonCollectionObserver *balloons_observer_;
  scoped_ptr<BalloonCollectionInterface> balloon_collection_;
#if defined(OS_MACOSX)
  bool using_growl_;
  scoped_ptr<BalloonCollectionInterface> inactive_collection_;
#endif
  std::deque<QueuedNotification*> show_queue_;
  std::deque<QueuedNotification*> delayed_queue_;
  bool in_presentation_;
  DISALLOW_EVIL_CONSTRUCTORS(NotificationManager);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_NOTIFICATION_MANAGER_H__
