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

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#include <assert.h>

#include "gears/notifier/notification_manager.h"

#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/serialization.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/timed_call.h"
#include "gears/notifier/const_notifier.h"
#include "gears/notifier/notification.h"
#include "gears/notifier/notification_manager_test.h"
#include "gears/notifier/notifier.h"
#include "gears/notifier/system.h"
#include "gears/notifier/user_activity.h"

#if defined(OS_MACOSX)
#include "gears/notifier/growl_collection.h"
#include "gears/notifier/notifier_pref_common.h"
#include "gears/notifier/notifier_pref_listener.h"
#endif  // defined(OS_MACOSX)

// TODO(levin): Add logging support to gears for exe's.
// LOG_F is like LOG but it also includes the function name in
// addition to the message.
#define LOG_F(x) LOG(x)

typedef std::deque<QueuedNotification*> QueuedNotifications;

// Constants.
const int kDelayedRestartCheckIntervalMs = 60000;   // 1m

// Holds a notification plus additional information needed while the
// notification is waiting to be displayed.
class QueuedNotification {
 public:
  explicit QueuedNotification(const GearsNotification &notification)
      : manager_(NULL),
        user_delayed_(false) {
    notification_.CopyFrom(notification);
  }

  ~QueuedNotification() {
  }

  const GearsNotification &notification() const { return notification_; }
  GearsNotification *mutable_notification()  { return &notification_; }

  bool is_user_delayed() const { return user_delayed_; }

  bool Matches(const SecurityOrigin &security_origin,
               const std::string16 &id) const {
    return notification_.Matches(security_origin, id);
  }

  void ShowWithDelay(NotificationManager *manager,
                     int64 delay_time_ms,
                     bool user_delayed) {
    manager_ = manager;
    user_delayed_ = user_delayed;
    timer_.reset(new TimedCall(delay_time_ms,
                               false,
                               &QueuedNotification::MoveToShowQueue,
                               this));
  }

  void CancelTimer() {
    // Cancel the timer.
    timer_.reset();
    user_delayed_ = false;
    manager_ = NULL;
  }

 private:
  // Callback for the timer for showing with a delay.
  static void MoveToShowQueue(void *cookie) {
    QueuedNotification *queued_notification =
        reinterpret_cast<QueuedNotification*>(cookie);
    assert(queued_notification && queued_notification->manager_);

    // Store the manager since it is cleared by canceling the timer.
    NotificationManager *manager = queued_notification->manager_;
    // Clean up state associated with the timer.
    queued_notification->CancelTimer();

    const GearsNotification &notification = queued_notification->notification();
    manager->MoveFromDelayedToShowQueue(
        notification.security_origin(),
        notification.id());
    // Don't access queued_notification at this point because it may be deleted.
  }

  GearsNotification notification_;
  NotificationManager *manager_;
  bool user_delayed_;
  scoped_ptr<TimedCall> timer_;
  DISALLOW_EVIL_CONSTRUCTORS(QueuedNotification);
};

void Clear(std::deque<QueuedNotification*> *notifications) {
  while (!notifications->empty()) {
    QueuedNotifications::reverse_iterator it =
        notifications->rbegin();
    QueuedNotification *removed = *it;
    notifications->pop_back();
    delete removed;
  }
}

const int NotificationManager::kNotificationManagerVersion = 1;

NotificationManager::NotificationManager(
      UserActivityInterface *activity,
      DelayedRestartInterface *delayed_restart,
      BalloonCollectionObserver *balloons_observer)
    : initialized_(false),
      activity_(activity),
      delayed_restart_(delayed_restart),
      balloons_observer_(balloons_observer),
#if defined(OS_MACOSX)
      using_growl_(false),
#endif  // defined(OS_MACOSX)
      in_presentation_(false) {
  assert(activity);
  activity->AddObserver(this);
  balloon_collection_.reset(new BalloonCollection(this));
  initialized_ = true;

#if defined(OS_MACOSX)
  inactive_collection_.reset(new GrowlBalloonCollection());
  SetDoesUseGrowlBalloonCollection(ShouldUseGrowlPref());
  NotifierPrefListener::RegisterForPreferenceChanges(this);
#endif  // defined(OS_MACOSX)
}

NotificationManager::~NotificationManager() {
  Clear(&show_queue_);
  Clear(&delayed_queue_);
}

#if defined(OS_MACOSX)
// TODO(chimene): In the future, transfer currently displayed notifications
// from one collection to the other when the pref is triggered.
void NotificationManager::SetDoesUseGrowlBalloonCollection(
    bool should_use_growl) {
  if (should_use_growl == using_growl_) {
    return;
  }
  balloon_collection_.swap(inactive_collection_);
  using_growl_ = should_use_growl;
}
#endif  // defined(OS_MACOSX)

QueuedNotification* FindNotification(
    const SecurityOrigin &security_origin,
    const std::string16 &id,
    bool and_remove,
    QueuedNotifications *queue) {
  assert(queue);
  for (QueuedNotifications::iterator it = queue->begin();
       it != queue->end();
       ++it) {
    if ((*it)->notification().Matches(security_origin, id)) {
      QueuedNotification *queued_notification = *it;
      if (and_remove) {
        queue->erase(it);
      }
      return queued_notification;
    }
  }
  return NULL;
}

QueuedNotification *NotificationManager::Find(
    const SecurityOrigin &security_origin,
    const std::string16 &id,
    bool and_remove) {

  QueuedNotification *queued_notification = FindNotification(
      security_origin, id, and_remove, &show_queue_);
  if (queued_notification) {
    return queued_notification;
  }
  return FindNotification(security_origin, id, and_remove, &delayed_queue_);
}

void NotificationManager::AddWithDelay(const GearsNotification &notification,
                                       int delay_ms) {
  QueuedNotification *queued_notification =
      new QueuedNotification(notification);
  AddToDelayedQueue(queued_notification, delay_ms, true);
}

void NotificationManager::Add(const GearsNotification &notification) {
  LOG_F(("security_origin.url: %s, id: %s",
         notification.security_origin().url().c_str(),
         notification.id().c_str()));
  // First, try to update the notification in the display
  // in case it is already showing.
  if (balloon_collection_->Update(notification)) {
    return;
  }

  // Next, see if it is already queued up (and update it).
  QueuedNotification *queued_notification =
      Find(notification.security_origin(), notification.id(), false);
  if (queued_notification) {
    LOG_F(("Updated. security_origin.url: %s, id: %s",
           notification.security_origin().url().c_str(),
           notification.id().c_str()));
    int64 now = GetCurrentTimeMillis();
    int64 previous_display_at_time_ms =
        queued_notification->notification().display_at_time_ms();
    queued_notification->mutable_notification()->CopyFrom(notification);

    // Handle display_at_time changes.
    if (notification.display_at_time_ms() != previous_display_at_time_ms) {
      // Several cases to consider:
      if (notification.display_at_time_ms() <= now) {
        if (previous_display_at_time_ms <= now) {
          // Both times are in the past.
          //     Result: Do nothing.
        } else {
          // The previous time is in the future.  The new time is in the past.
          //     Result: Move it to the display queue.
          MoveFromDelayedToShowQueue(notification.security_origin(),
                                     notification.id());
        }
      } else {
        if (previous_display_at_time_ms <= now) {
          // The previous time is in the past.  The new time is in the future.
          //    In this case, the notification is in one of two states:
          if (queued_notification->is_user_delayed()) {
            //    1. in a snooze state (i.e. delayed by the user).
            //       Result: Do nothing.
          } else {
            //    2. waiting to be displayed
            //       Result: Delay the notification.
            MoveToDelayedQueue(notification.security_origin(),
                               notification.id(),
                               notification.display_at_time_ms() - now,
                               false);
          }
        } else {
          // Both times are in the future.
          //     Result: Reschedule the display.
          queued_notification->ShowWithDelay(
              this, notification.display_at_time_ms() - now, false);
        }
      }
    }

    return;
  }

  // It appears that this is a new notification so add it.
  LOG_F(("Added. security_origin.url: %s, id: %s",
         notification.security_origin().url().c_str(),
         notification.id().c_str()));
  queued_notification = new QueuedNotification(notification);

  // If it is delayed until sometime in the future, set-up the delay.
  int64 display_at_time_ms =
      queued_notification->notification().display_at_time_ms();
  int64 delay_ms = display_at_time_ms - GetCurrentTimeMillis();
  if (delay_ms > 0) {
    AddToDelayedQueue(queued_notification, delay_ms, false);
    return;
  }
  AddToShowQueue(queued_notification);
}

void NotificationManager::AddToDelayedQueue(
    QueuedNotification *queued_notification,
    int64 delay_ms,
    bool user_delayed) {
  assert(queued_notification);
  delayed_queue_.push_back(queued_notification);
  queued_notification->ShowWithDelay(this, delay_ms, user_delayed);
}

void NotificationManager::AddToShowQueue(
    QueuedNotification *queued_notification) {
  assert(queued_notification);
  assert(queued_notification->notification().display_at_time_ms() <=
         GetCurrentTimeMillis());
  show_queue_.push_back(queued_notification);
  CheckAndShowNotifications();
}

void NotificationManager::MoveFromDelayedToShowQueue(
    const SecurityOrigin &security_origin,
    const std::string16 &id) {
  QueuedNotification *queued_notification =
      FindNotification(security_origin, id, true, &delayed_queue_);
  // This should only be called when it is known that the notification is
  // in the delayed queue.
  assert(queued_notification);
  queued_notification->CancelTimer();
  AddToShowQueue(queued_notification);
}

void NotificationManager::MoveToDelayedQueue(
    const SecurityOrigin &security_origin,
    const std::string16 &id,
    int64 delay_ms,
    bool user_delayed) {
  QueuedNotification *queued_notification = Find(security_origin, id, true);
  // This should only be called when it is known that the notification is
  // in a queue.
  assert(queued_notification);
  AddToDelayedQueue(queued_notification, delay_ms, user_delayed);
}

bool NotificationManager::Delete(const SecurityOrigin &security_origin,
                                 const std::string16 &id) {
  LOG_F(("Deleting. security_origin.url: %s, id: %s",
         security_origin.url().c_str(), id.c_str()));

  // If displayed, then delete it.
  if (balloon_collection_->Delete(security_origin, id)) {
    return true;
  }

  QueuedNotification *queued_notification = Find(security_origin, id, true);
  if (queued_notification) {
    delete queued_notification;
    return true;
  }

  // Not found.
  LOG_F(("Not found. security_origin.url: %s, id: %s",
         security_origin.url().c_str(), id.c_str()));
  return false;
}

void NotificationManager::CheckAndShowNotifications() {
  // Can we perform the delayed restart now?
  //
  // Do this before "IsActiveUserMode" because this
  // check wants the user to be idle.
  if (CheckDelayedRestart()) {
    return;
  }

  // Is it ok to show the notification now?
  activity_->CheckNow();
  if (!IsActiveUserMode(activity_->user_mode())) {
    return;
  }

  ShowNotifications();
}

void NotificationManager::ShowNotifications() {
  while (!show_queue_.empty() && balloon_collection_->HasSpace()) {
    QueuedNotification* queued_notification = show_queue_.front();
    show_queue_.pop_front();
    balloon_collection_->Add(queued_notification->notification());
    delete queued_notification;
  }
}

void NotificationManager::OnUserActivityChange() {
  bool previous_in_presentation = in_presentation_;
  UserMode user_mode = activity_->user_mode();
  in_presentation_ = user_mode == USER_PRESENTATION_MODE;
  if (previous_in_presentation != in_presentation_) {
    if (in_presentation_) {
      balloon_collection_->HideAll();
    } else {
      balloon_collection_->ShowAll();
    }
  }

  if (IsActiveUserMode(user_mode)) {
    ShowNotifications();
  }
}

void NotificationManager::OnBalloonSpaceChanged() {
  CheckAndShowNotifications();
  if (balloons_observer_) {
    balloons_observer_->OnBalloonSpaceChanged();
  }
}

void NotificationManager::OnSnoozeNotification(
    const GearsNotification &notification, int snooze_seconds) {
  AddWithDelay(notification, snooze_seconds * 1000);
}

bool NotificationManager::CheckDelayedRestart() {
  if (initialized_ && delayed_restart_) {
    // Only perform the restart if the user is away or idle.
    if (activity_->user_mode() != USER_AWAY_MODE &&
        activity_->user_mode() != USER_IDLE_MODE) {
      // Only perform the restart if no notification is showing.
      if (!showing_notifications()) {
        // Perform the restart if needed.
        if (delayed_restart_->IsRestartNeeded()) {
          delayed_restart_->Restart();
          return true;
        }
      }
    }
  }

  return false;
}

bool NotificationManager::showing_notifications() const {
  return balloon_collection_.get() && balloon_collection_->count() > 0;
}

bool NotificationManager::has_pending_notifications() const {
  return !show_queue_.empty() || !delayed_queue_.empty();
}

// Helper function to write a notification.
bool NotificationManager::WriteNotification(
    File *file, const GearsNotification &notification) {
  assert(file);

  std::vector<uint8> serialized_data;
  Serializer serializer(&serialized_data);
  if (!notification.Serialize(&serializer)) {
    return false;
  }

  int size = static_cast<int>(serialized_data.size());
  if (file->Write(reinterpret_cast<const uint8*>(&size), sizeof(size)) !=
      sizeof(size)) {
    return false;
  }
  return file->Write(reinterpret_cast<const uint8*>(&serialized_data.at(0)),
                     size) == size;
}

bool NotificationManager::GetNotificationSavePath(std::string16 *file_path,
                                                  bool create_if_missing) {
  assert(file_path);

  if (!System::GetUserDataLocation(file_path, create_if_missing)) {
    return false;
  }
  *file_path += kPathSeparator;
  *file_path += kNotifierShortName;
  *file_path += STRING16(L".dat");
  return true;
}

void NotificationManager::CleanupNotificationSavePath(
        const std::string16 &file_path) {
  File::Delete(file_path.c_str());

  // We save notifications in the user data folder, which is not created and
  // deleted by the installer. Thus we have to do the cleanup here.
  // TODO: This is not an ideal solution. It also have a small chance of risk.
  // Consider moving the management of user data folder to the installer.
  std::string16 user_data_folder;
  if (File::GetParentDirectory(file_path, &user_data_folder) &&
      File::DirectoryExists(user_data_folder.c_str()) &&
      File::GetDirectoryFileCount(user_data_folder.c_str()) == 0) {
    File::DeleteRecursively(user_data_folder.c_str());
  }
}

bool NotificationManager::SaveNotifications() {
  // Get number of all notifications being shown and queued. If it is zero,
  // no need to save.
  int count = static_cast<int>(show_queue_.size()) +
              static_cast<int>(delayed_queue_.size());
  if (balloon_collection_.get()) {
    count += balloon_collection_->count();
  }
  if (count == 0) {
    return true;
  }

  // Create the saved file.
  std::string16 file_path;
  if (!GetNotificationSavePath(&file_path, /*create_if_missing*/ true)) {
    return false;
  }
  scoped_ptr<File> file(File::Open(file_path.c_str(),
                                   File::WRITE,
                                   File::NEVER_FAIL));
  if (!file.get()) {
    return false;
  }

  // Write version.
  if (file->Write(reinterpret_cast<const uint8*>(&kNotificationManagerVersion),
                  sizeof(kNotificationManagerVersion)) !=
      sizeof(kNotificationManagerVersion)) {
    return false;
  }

  // Write number of all notifications being shown and queued.
  if (file->Write(reinterpret_cast<const uint8*>(&count), sizeof(count)) !=
      sizeof(count)) {
    return false;
  }

  // Write the notifications being displayed.
  bool res = true;
  if (balloon_collection_.get()) {
    for (int i = 0; i < balloon_collection_->count(); ++i) {
      if (!WriteNotification(file.get(),
                             *(balloon_collection_->notification_at(i)))) {
        res = false;
        break;
      }
    }
  }

  // Write the notifications in the show queue.
  if (res) {
    for (size_t i = 0; i < show_queue_.size(); ++i) {
      if (!WriteNotification(file.get(),
                             show_queue_[i]->notification())) {
        res = false;
        break;
      }
    }
  }

  // Write the notifications in the delayed queue.
  if (res) {
    for (size_t i = 0; i < delayed_queue_.size(); ++i) {
      if (!WriteNotification(file.get(),
                             delayed_queue_[i]->notification())) {
        res = false;
        break;
      }
    }
  }

  // Delete the intermediate file if failed.
  if (!res) {
    file.reset();
    File::Delete(file_path.c_str());
  }

  return res;
}

bool NotificationManager::LoadNotifications() {
  // Open the saved file.
  std::string16 file_path;
  if (!GetNotificationSavePath(&file_path, /*create_if_missing*/ false)) {
    return false;
  }
  scoped_ptr<File> file(File::Open(file_path.c_str(),
                                   File::READ,
                                   File::FAIL_IF_NOT_EXISTS));
  if (!file.get()) {
    return false;
  }

  bool res = InternalLoadNotifications(file.get());

  // Delete the file not matter if the loading is successful or failed.
  file.reset();
  CleanupNotificationSavePath(file_path);

  return res;
}

bool NotificationManager::InternalLoadNotifications(File *file) {
  assert(file);

  // Read and validate the version.
  int version = 0;
  int64 bytes_read = file->Read(reinterpret_cast<uint8*>(&version),
                                sizeof(version));
  if (bytes_read != sizeof(version) || version != kNotificationManagerVersion) {
    return false;
  }

  // Read number of notifications.
  int count = 0;
  bytes_read = file->Read(reinterpret_cast<uint8*>(&count), sizeof(count));
  if (bytes_read != sizeof(count) || count <= 0) {
    return false;
  }

  // Read each notification and add it.
  for (int i = 0; i < count; ++i) {
    int size = 0;
    bytes_read = file->Read(reinterpret_cast<uint8*>(&size), sizeof(size));
    if (bytes_read != sizeof(size) || size <= 0) {
      return false;
    }

    std::vector<uint8> serialized_data;
    serialized_data.resize(size);
    if (!file->Read(reinterpret_cast<uint8*>(&serialized_data.at(0)), size)) {
      return false;
    }

    Deserializer deserializer(&serialized_data[0], size);
    GearsNotification notification;
    if (!notification.Deserialize(&deserializer)) {
      return false;
    }

    Add(notification);
  }

  return true;
}

#endif  // OFFICIAL_BUILD
