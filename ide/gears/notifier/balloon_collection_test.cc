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
#if USING_CCTESTS
#include "gears/base/common/security_model.h"
#include "gears/base/common/string16.h"
#include "gears/notifier/balloons.h"
#include "gears/notifier/notification.h"
#include "third_party/gtest/include/gtest/gtest.h"

class BalloonCollectionObserverMock : public BalloonCollectionObserver {
 public:
  BalloonCollectionObserverMock() : space_changed_(0) {
  }

  // BalloonCollectionObserver methods.
  void OnBalloonSpaceChanged() {
    ++space_changed_;
  }

  void OnSnoozeNotification(const GearsNotification &notification,
                            int snooze_seconds) {
  }

  // Other methods.
  int space_changed() {
    return space_changed_;
  }

 private:
  int space_changed_;
  DISALLOW_EVIL_CONSTRUCTORS(BalloonCollectionObserverMock);
};

TEST(BalloonCollectionTest, BasicFunctionality) {
  BalloonCollectionObserverMock observer;
  BalloonCollection balloons(&observer);

  GearsNotification notification1;
  SecurityOrigin security_origin;
  security_origin.InitFromUrl(STRING16(L"http://gears.google.com/MyService"));
  notification1.set_security_origin(security_origin);
  notification1.set_id(STRING16(L"1"));
  balloons.Add(notification1);

  bool found = balloons.Update(notification1);
  EXPECT_TRUE(found) << "Update didn't find the original notification";

  notification1.set_id(STRING16(L"2"));
  found = balloons.Update(notification1);
  EXPECT_FALSE(found) << "Update found wrong notification";

  balloons.Add(notification1);

  bool deleted = balloons.Delete(notification1.security_origin(),
                                 notification1.id());
  EXPECT_TRUE(deleted) << "Delete didn't find the notification";

  deleted = balloons.Delete(notification1.security_origin(),
                            notification1.id());
  EXPECT_FALSE(deleted) << "Delete can delete a notification twice!";

  EXPECT_EQ(4, observer.space_changed())
      << "Incorrect number of calls to OnBalloonSpaceChanged";
}

#endif  // USING_CCTESTS
#endif  // OFFICIAL_BUILD
