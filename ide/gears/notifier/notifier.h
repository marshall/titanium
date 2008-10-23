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

// Notifier can be restarted when a new version is installed. There are two
// types of restart: immediate restart and delayed restart. Immediate restart
// should only be chosen if a new version containing urgent fix, like security
// update, is installed. For all other scenarios, the delayed restart should be
// preferred.
//
// To request the immediate restart, the installer should send an IPC message
// "kDesktop_RestartNotifierImmediately" to the notifier process after it 
// installes the new version.
//
// For delayed restart, the installer does not need to do anything. The running
// notifier process will automatically restart itself and pick up the new
// version when both of the following conditions are satisfied:
// * The user is idle or away.
// * No notification is showing on the screen.
//
// The following describes what happens when the restart is performed:
// Current Process:
// 1) Unregister the notifier process such that no IPC message will be received
//    and handled any more.
// 2) Save all queued and showing notifications into a temp file.
// 3) Start new process.
// 4) Quit current process.
// New Process:
// 1) Wait till parent process dies.
// 2) Do normal initialization.
// 3) Load saved notifications and add them appropriately.

#ifndef GEARS_NOTIFIER_NOTIFIER_H__
#define GEARS_NOTIFIER_NOTIFIER_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/string16.h"
#include "gears/notifier/notification_manager.h"
#include "gears/notifier/user_activity.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class BalloonCollectionObserver;
class GearsNotification;
class NotificationManager;
class SecurityOrigin;
class TimedCall;

// Interface to provide delayed restart support.
class DelayedRestartInterface {
 public:
  // IsRestartNeeded should be called periodically to check if a new version
  // is available.
  virtual bool IsRestartNeeded() const = 0;

  // Restart should be called when it is the right time to restart the
  // executable.
  // This can be called more than once, though only the first one takes effect.
  virtual void Restart() = 0;
};

class Notifier : public IpcMessageQueue::HandlerInterface,
                 public DelayedRestartInterface,
                 public BalloonCollectionObserver,
                 public UserActivityObserver {
 public:
  Notifier();
  virtual ~Notifier();

  virtual bool Initialize();
  virtual int Run() = 0;
  virtual void Terminate();
  virtual void RequestQuit() = 0;

  // IpcMessageQueue::HandlerInterface interface.
  virtual void HandleIpcMessage(IpcProcessId source_process_id,
                                int message_type,
                                const IpcMessageData *message_data);

  // DelayedRestartInterface interface.
  virtual bool IsRestartNeeded() const;
  virtual void Restart();

  // BalloonCollectionObserver interface.
  virtual void OnBalloonSpaceChanged();
  virtual void OnSnoozeNotification(const GearsNotification &notification,
                                    int snooze_seconds) {}

  // UserActivityObserver interface.
  virtual void OnUserActivityChange();

  void AddNotification(const GearsNotification &notification);
  void RemoveNotification(const SecurityOrigin &security_origin,
                          const std::string16 &id);

 protected:
  // Register the Notifier process so that other processes can find it.
  virtual bool RegisterProcess() = 0;

  // Unregister the Notifier process.
  virtual bool UnregisterProcess() = 0;

  bool running_;
  bool to_restart_;

 private:
  // Can we shutdown notifier on lack of activity?
  bool CanShutdownOnLackOfActivity() const;

  // Perform the check for shutdown on lack of activity.
  void CheckShutdownOnLackOfActivity();

  // TimedCall callback.
  static void ShutdownOnLackOfActivity(void *arg);

  scoped_ptr<UserActivityMonitor> user_activity_monitor_;
  scoped_ptr<NotificationManager> notification_manager_;
  scoped_ptr<TimedCall> shutdown_on_lack_of_activity_timer_;
  DISALLOW_EVIL_CONSTRUCTORS(Notifier);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_NOTIFIER_H__
