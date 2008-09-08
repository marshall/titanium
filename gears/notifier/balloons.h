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
// Handles the visible notification (or balloons).

#ifndef GEARS_NOTIFIER_BALLOONS_H__
#define GEARS_NOTIFIER_BALLOONS_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#include <deque>
#include "gears/base/common/basictypes.h"
#include "gears/base/common/string16.h"
#include "gears/notifier/notification.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class BalloonCollection;
class GearsNotification;
class TimedCall;
class UserActivityInterface;

// Glint classes
namespace glint {
class Node;
class RootUI;
}

class BalloonCollectionObserver {
 public:
  // Called when there is more or less space for balloons due to
  // monitor size changes or balloons disappearing.
  virtual void OnBalloonSpaceChanged() = 0;

  // Called when the user chooses to snooze the notification.
  virtual void OnSnoozeNotification(const GearsNotification &notification,
                                    int snooze_seconds) = 0;
};

class BalloonCollectionInterface {
 public:
  virtual ~BalloonCollectionInterface() {}
  // Adds a new balloon for the specified notification. Does not check for
  // already showing notifications with the same security_origin and id.
  // Asserts if there is already a notification with the same
  // security_origin/id.
  virtual void Add(const GearsNotification &notification) = 0;
  // Updates the existing balloon with notification that matches
  // security_origin and id of the specified one. In case there is no
  // balloon showing the matching notification, returns 'false'.
  virtual bool Update(const GearsNotification &notification) = 0;
  // Immediately "expires" any notification with the same security_origin
  // and id from the screen display. Returns 'false' if there is no such
  // notification.
  virtual bool Delete(const SecurityOrigin &security_origin,
                      const std::string16 &id) = 0;

  // Show all balloons.
  virtual void ShowAll() = 0;

  // Hide all balloons.
  virtual void HideAll() = 0;

  // Is there room to add another notification?
  virtual bool HasSpace() const = 0;

  // Number of balloons being shown.
  virtual int count() const = 0;

  // Gets notification of the balloon at the specified index.
  virtual const GearsNotification *notification_at(int i) const = 0;
};

// Represents a Notification on the screen.
class Balloon {
 public:
  explicit Balloon(const GearsNotification &from,
                   BalloonCollection *collection);
  ~Balloon();

  const GearsNotification &notification() const {
    return notification_;
  }

  GearsNotification *mutable_notification() {
    return &notification_;
  }

  glint::Node *root() {
    if (!root_) {
      root_ = CreateTree();
    }
    return root_;
  }

  bool InitializeUI(glint::Node *container);
  void UpdateUI();
  bool InitiateClose(bool user_initiated);
  void OnAnimationCompleted();
  void OnMouseOut();
  void OnMouseIn();

 private:
  enum BalloonState {
    OPENING_BALLOON,
    SHOWING_BALLOON,
    AUTO_CLOSING_BALLOON,
    USER_CLOSING_BALLOON,
    RESTORING_BALLOON,
  };

  glint::Node *CreateTree();
  bool SetTextField(const char *id, const std::string16 &text);
  bool SetImage(const char *id,
                int width,
                int height,
                const void *decoded_image);
  void ShowMenu();
  void PerformAction(const std::string16 &action);
  static void OnCloseButton(const std::string &button_id, void *user_info);
  static void OnMenuButton(const std::string &button_id, void *user_info);
  static bool SetAlphaTransition(glint::Node *node, double transition_duration);
  static bool SetMoveTransition(glint::Node *node);

  GearsNotification notification_;
  glint::Node *root_;
  BalloonCollection *collection_;
  BalloonState state_;
  DISALLOW_EVIL_CONSTRUCTORS(Balloon);
};

typedef std::deque<Balloon*> Balloons;

class BalloonCollection : public BalloonCollectionInterface {
 public:
  explicit BalloonCollection(BalloonCollectionObserver *observer);
  virtual ~BalloonCollection();

  // BalloonCollectionInterface overrides
  virtual void Add(const GearsNotification &notification);
  virtual bool Update(const GearsNotification &notification);
  virtual bool Delete(const SecurityOrigin &security_origin,
                      const std::string16 &id);
  virtual void ShowAll();
  virtual void HideAll();
  virtual bool HasSpace() const;
  virtual int count() const { return static_cast<int>(balloons_.size()); }
  virtual const GearsNotification *notification_at(int i) const {
    return (i < static_cast<int>(balloons_.size()))
           ? &balloons_[i]->notification() : NULL;
  }

  // Adds and removes the balloons to this collection (to both internal
  // list and UI tree).
  bool AddToUI(Balloon *balloon);
  bool RemoveFromUI(Balloon *balloon);

  // Suspends expiration of the balloons.
  void SuspendExpirationTimer();
  // Restores expiration of the balloons.
  void RestoreExpirationTimer();
  // Resets expiration timer - done on changes in balloon set to allow
  // user to observe new state of the notification stack.
  void ResetExpirationTimer();
  // If true, the balloons do not expire and disappear but stay on screen.
  bool expiration_suspended() {
    return expiration_suspended_counter_ > 0;
  }
  void OnTimer(double current_time);

  // Immediately hides all notifications and stops the notification pipeline
  // so the accumulated notifications will be shown when snooze timeout ends.
  void Snooze(int timeout_seconds);

  // Called when a notification is asked to snooze for the specified seconds.
  void OnSnoozeNotification(const GearsNotification &notification,
                            int snooze_seconds);

 private:
  void Clear();  // clears balloons_
  Balloon *FindBalloon(const SecurityOrigin &security_origin,
                       const std::string16 &id,
                       bool and_remove);
  void EnsureRoot();

  Balloons balloons_;
  BalloonCollectionObserver *observer_;
  glint::RootUI *root_ui_;
  int expiration_suspended_counter_;
  double last_time_;
  double elapsed_time_;
  scoped_ptr<TimedCall> snooze_timer_;
  bool has_space_;
  DISALLOW_EVIL_CONSTRUCTORS(BalloonCollection);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_BALLOONS_H__
