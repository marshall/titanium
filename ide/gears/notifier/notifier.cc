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
#include "gears/notifier/notifier.h"

#include "gears/base/common/security_model.h"
#include "gears/base/common/serialization.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/timed_call.h"
#include "gears/notifier/balloons.h"
#include "gears/notifier/const_notifier.h"
#include "gears/notifier/notification.h"
#include "gears/notifier/notification_manager.h"
#include "gears/notifier/notifier_process.h"
#include "gears/notifier/user_activity.h"
#if USING_CCTESTS
#include "gears/notifier/unit_test.h"
#endif  // USING_CCTESTS
#include "third_party/glint/include/platform.h"
#include "third_party/glint/include/work_item.h"

// Constants.
const int kInactivityToShutdownIntervalMs = 30 * 60 * 1000;    // 30m

// Posted as workitem from IPC worker thread to main UI thread.
class NotificationTask : public glint::WorkItem {
 public:
  enum Action { ACTION_ADD, ACTION_DELETE };
  NotificationTask(Notifier *notifier,
                   const GearsNotification &notification,
                   Action action)
    : action_(action),
      notifier_(notifier) {
    assert(notifier);
    notification_.CopyFrom(notification);
  }

  void Post() {
    glint::platform()->PostWorkItem(NULL, this);
  }

  virtual void Run() {
    if (action_ == ACTION_ADD) {
      notifier_->AddNotification(notification_);
    } else if (action_ == ACTION_DELETE) {
      notifier_->RemoveNotification(notification_.security_origin(),
                                    notification_.id());
    } else {
      assert(false);
    }
  }
 private:
  Action action_;
  Notifier *notifier_;
  GearsNotification notification_;
};

Notifier::Notifier()
  : running_(false),
    to_restart_(false) {
}

Notifier::~Notifier() {
}

bool Notifier::Initialize() {
  user_activity_monitor_.reset(UserActivityMonitor::Create());
  user_activity_monitor_->AddObserver(this);
  notification_manager_.reset(
      new NotificationManager(user_activity_monitor_.get(), this, this));

  GearsNotification::RegisterAsSerializable();
  IpcMessageQueue *ipc_message_queue = IpcMessageQueue::GetSystemQueue();
  if (!ipc_message_queue) {
    return false;
  }
  ipc_message_queue->RegisterHandler(kDesktop_AddNotification, this);
  ipc_message_queue->RegisterHandler(kDesktop_RemoveNotification, this);

  if (!RegisterProcess()) {
    return false;
  }

  // Load saved notifications if needed.
  if (!notification_manager_->LoadNotifications()) {
    LOG(("Saved notifications not loaded (most likely they didn't exist).\n"));
  }

  running_ = true;

  return true;
}

void Notifier::Terminate() {
  // Restart the instance if needed.
  if (to_restart_) {
    NotifierProcess::StartProcess(kRestartCmdLineSwitch, NULL, true);
  }
}

void Notifier::Restart() {
  // Protect from multiple restart.
  if (to_restart_) {
    return;
  }

  // Unregister notifier process so that we will not receive messages.
  UnregisterProcess();

  // Save the notifiations.
  notification_manager_->SaveNotifications();

  to_restart_ = true;
  RequestQuit();
}

// This call comes on a worker thread that services the inter-process
// communication mechanism. So we need to make copies of all receipts and
// ship over by FedEx.
void Notifier::HandleIpcMessage(IpcProcessId source_process_id,
                                int message_type,
                                const IpcMessageData *message_data) {
  switch (message_type) {
    case kDesktop_AddNotification: {
      const GearsNotification *notification =
          static_cast<const GearsNotification*>(message_data);
      (new NotificationTask(this,
                            *notification,
                            NotificationTask::ACTION_ADD))->Post();
      break;
    }

    case kDesktop_RemoveNotification: {
      const GearsNotification *notification =
          static_cast<const GearsNotification*>(message_data);
      (new NotificationTask(this,
                            *notification,
                            NotificationTask::ACTION_DELETE))->Post();
      break;
    }

    case kDesktop_RestartNotifierImmediately:
      Restart();
      break;

    default:
      assert(false);
      break;
  }
}

bool Notifier::IsRestartNeeded() const {
  return false;
}

bool Notifier::CanShutdownOnLackOfActivity() const {
  // We will shutdown the notifier on lack of activity that is defined as
  // the following:
  // 1) No notification is showing or pending to show.
  // 2) The user is idle or away.
  // 3) Condition 1) and 2) have been met for a period of time, i.e. 30 minutes.
  //    (This condition is enforced by shutdown_on_lack_of_activity_timer_.) 
  return notification_manager_.get() &&
         !notification_manager_->showing_notifications() &&
         !notification_manager_->has_pending_notifications() &&
         user_activity_monitor_.get() &&
         (user_activity_monitor_->user_mode() == USER_IDLE_MODE ||
          user_activity_monitor_->user_mode() == USER_AWAY_MODE);
}

void Notifier::CheckShutdownOnLackOfActivity() {
  if (CanShutdownOnLackOfActivity()) {
    // Since condition 1 and 2 for shutdown have been met, start a timer to see
    // if they are still met after a period of time (condition 3).
    if (!shutdown_on_lack_of_activity_timer_.get()) {
      shutdown_on_lack_of_activity_timer_.reset(
          new TimedCall(kInactivityToShutdownIntervalMs,
                        false,
                        &Notifier::ShutdownOnLackOfActivity,
                        this));
    }
  } else {
    shutdown_on_lack_of_activity_timer_.reset();
  }
}

void Notifier::OnBalloonSpaceChanged() {
  CheckShutdownOnLackOfActivity();
}

void Notifier::OnUserActivityChange() {
  CheckShutdownOnLackOfActivity();
}

void Notifier::ShutdownOnLackOfActivity(void *arg) {
  assert(arg);

  Notifier *this_ptr = reinterpret_cast<Notifier*>(arg);

  this_ptr->shutdown_on_lack_of_activity_timer_.reset();
  if (this_ptr->CanShutdownOnLackOfActivity()) {
    LOG(("Shut down notifier on lack of activity\n"));
    this_ptr->UnregisterProcess();
    this_ptr->RequestQuit();
  }
}

void Notifier::AddNotification(const GearsNotification &notification) {
  LOG(("Add notification %S - %S ($S)\n",
       notification.security_origin().url().c_str(),
       notification.id().c_str(),
       notification.title().c_str()));
  notification_manager_->Add(notification);

  // Run the check so that it sees there has been activity.
  CheckShutdownOnLackOfActivity();
}

void Notifier::RemoveNotification(const SecurityOrigin &security_origin,
                                  const std::string16 &id) {
  LOG(("Remove notification %S - %S\n", security_origin.url().c_str(),
       id.c_str()));
  notification_manager_->Delete(security_origin, id);

  // Run the check so that it sees there has been activity.
  CheckShutdownOnLackOfActivity();
}

#endif  // OFFICIAL_BUILD
