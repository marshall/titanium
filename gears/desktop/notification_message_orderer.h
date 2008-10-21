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

#ifndef GEARS_DESKTOP_NOTIFICATION_MESSAGE_ORDERER_H_
#define GEARS_DESKTOP_NOTIFICATION_MESSAGE_ORDERER_H_
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#include <vector>
#include "gears/base/common/basictypes.h"
#include "gears/base/common/string16.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class GearsNotification;
class NotifierProxyInterface;
class PendingNotification;
class SecurityOrigin;

// Ensures that messages about notifications with the same origin/id are sent
// in order (even though there are async operations that happen with them).
class NotificationMessageOrderer {
 public:
  NotificationMessageOrderer();

  // Used by test code to substitute a proxy.
  explicit NotificationMessageOrderer(NotifierProxyInterface *mock_proxy);
  ~NotificationMessageOrderer();

  // Reserves a spot for a notification.  This should only be done for
  // add messages.  (Other messages are assumed to be synchronous.)
  // Returns a token which is how to refer to the reservation.
  int64 AddReservation(const GearsNotification &notification);

  // Releases the reserved spot.
  void RemoveReservation(int64 token);

  // Sends the notification.
  // Note: This takes ownership of "released_notification".
  // token is value returned by AddPendingNotification, it should be
  // kInvalidToken for other messages.
  void PostNotification(int message_type,
                        GearsNotification *released_notification,
                        int64 reservation);

  static const int64 kInvalidReservation;

 private:
  void RemovePending(const SecurityOrigin &security_origin,
                     const std::string16 &id);

#ifdef DEBUG
  void ValidateInvariant();
#else
  void ValidateInvariant() {}
#endif  // DEBUG

  scoped_ptr<NotifierProxyInterface> proxy_;
  std::vector<PendingNotification*> pending_notifications_;
  int64 next_reservation_;
  DISALLOW_EVIL_CONSTRUCTORS(NotificationMessageOrderer);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_DESKTOP_NOTIFICATION_MESSAGE_ORDERER_H_
