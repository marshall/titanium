// Copyright 2007, Google Inc.
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
#ifdef OS_ANDROID
  // The notification API has not been implemented for Android.
#else

#include "gears/desktop/notification_message_orderer.h"

#include "gears/base/common/ipc_message_queue.h"
#include "gears/notifier/notification.h"
#include "gears/notifier/notifier_proxy.h"

const int64 NotificationMessageOrderer::kInvalidReservation = -1;

// Contains information about a pending notification.
class PendingNotification {
 public:
  PendingNotification(const SecurityOrigin &security_origin,
                      const std::string16 &id,
                      int64 reservation)
      : security_origin_(security_origin),
        id_(id),
        reservation_(reservation) {
  }

  const SecurityOrigin &security_origin() const {
    return security_origin_;
  }

  const std::string16 &id() const {
    return id_;
  }

  int64 reservation() const {
    return reservation_;
  }

  bool Matches(const SecurityOrigin &security_origin,
               const std::string16 &id) const {
    return security_origin_.IsSameOrigin(security_origin) &&
        id_ == id;
  }

 private:
  SecurityOrigin security_origin_;
  std::string16 id_;
  int64 reservation_;
  DISALLOW_EVIL_CONSTRUCTORS(PendingNotification);
};


// NotificationMessageOrderer ensures that every message
// about a particular notification (i.e all notifications
// with the same SecurityOrigin/Id) happen in order.
//
// You can think of it as if there is one queue per SecurityOrigin/Id.
// In reality, only the last reservation for a SecurityOrigin/Id is
// kept. Similarly, if a delete message is posted, then any
// reservation for the SecurityOrigin/Id is thrown away.
//
// Minor problem: If the last notification with the last reservation
// fails (and does a RemoveReservation), then the user doesn't get
// the notification, but that is a very unlikely scenario.  Even if
// the old notification got through, it would have old information.

NotificationMessageOrderer::NotificationMessageOrderer()
  : next_reservation_(1) {
  proxy_.reset(new NotifierProxy());
}

NotificationMessageOrderer::NotificationMessageOrderer(
    NotifierProxyInterface *proxy) : next_reservation_(1) {
  assert(proxy);
  proxy_.reset(proxy);
}

NotificationMessageOrderer::~NotificationMessageOrderer() {
  ValidateInvariant();
  while (!pending_notifications_.empty()) {
    std::vector<PendingNotification*>::reverse_iterator it =
        pending_notifications_.rbegin();
    scoped_ptr<PendingNotification> removed(*it);
    pending_notifications_.pop_back();
  }
}

#ifdef DEBUG
// Cerify that each SecurityOrigin/Id only appear once in the queue.
void NotificationMessageOrderer::ValidateInvariant() {
  for (std::vector<PendingNotification*>::iterator it =
           pending_notifications_.begin();
       it != pending_notifications_.end();
       ++it) {
    PendingNotification *pending = *it;
    std::vector<PendingNotification*>::iterator it2 = it;
    ++it2;
    for (;
         it2 != pending_notifications_.end();
         ++it2) {
      assert(!(*it2)->Matches(pending->security_origin(), pending->id()));
    }
  }
}
#endif

int64 NotificationMessageOrderer::AddReservation(
    const GearsNotification &notification) {
  ValidateInvariant();

  // Remove any item in the queue for the given origin/id,
  // since we'll be adding a new one.
  RemovePending(notification.security_origin(),
                notification.id());
  PendingNotification *pending = new PendingNotification(
      notification.security_origin(),
      notification.id(),
      next_reservation_);
  next_reservation_++;
  pending_notifications_.push_back(pending);

  ValidateInvariant();
  return pending->reservation();
}

void NotificationMessageOrderer::RemovePending(
    const SecurityOrigin &security_origin,
    const std::string16 &id) {
  ValidateInvariant();

  for (std::vector<PendingNotification*>::iterator it =
           pending_notifications_.begin();
       it != pending_notifications_.end();
       ++it) {
    if ((*it)->Matches(security_origin, id)) {
      scoped_ptr<PendingNotification> pending(*it);
      pending_notifications_.erase(it);
      // There is only one pending notification per origin/id
      // and we found it, so we're done.
      return;
    }
  }
  ValidateInvariant();
}

void NotificationMessageOrderer::RemoveReservation(int64 reservation) {
  ValidateInvariant();
  for (std::vector<PendingNotification*>::iterator it =
           pending_notifications_.begin();
       it != pending_notifications_.end();
       ++it) {
    if ((*it)->reservation() == reservation) {
      scoped_ptr<PendingNotification> pending(*it);
      pending_notifications_.erase(it);
      return;
    }
  }
}

void NotificationMessageOrderer::PostNotification(
    int message_type,
    GearsNotification *released_notification,
    int64 reservation) {
  assert(message_type);
  assert(released_notification);
  ValidateInvariant();

  scoped_ptr<GearsNotification> notification(released_notification);
  switch (message_type) {
    case kDesktop_RemoveNotification:
      assert(reservation == kInvalidReservation);
      // If we get a removal message about a pending notification,
      // then get rid of the pending status so that the notification
      // will not be sent.  We still should send the remove message
      // in case the notification has been previously sent.
      RemovePending(notification->security_origin(),
                    notification->id());
      proxy_->PostNotification(message_type, notification.release());
      break;

    case kDesktop_AddNotification: {
      assert(reservation != kInvalidReservation);
      // Make sure that add notification messages for with the same
      // (security_origin, id) happen in order.
      for (std::vector<PendingNotification*>::iterator it =
               pending_notifications_.begin();
           it != pending_notifications_.end();
           ++it) {

        if ((*it)->Matches(notification->security_origin(),
                           notification->id())) {
          // There is only one reservation per origin/id.
          // If the reservations don't match, then it isn't here.
          if ((*it)->reservation() != reservation) {
            return;
          }
          scoped_ptr<PendingNotification> pending(*it);
          proxy_->PostNotification(kDesktop_AddNotification,
                                   notification.release());
          pending_notifications_.erase(it);
          ValidateInvariant();
          return;
        }
      }
      break;
    }

    default:
      assert(reservation == kInvalidReservation);
      proxy_->PostNotification(message_type, notification.release());
      break;
  }
}
#endif  // OS_ANDROID
#endif  // OFFICIAL_BUILD
