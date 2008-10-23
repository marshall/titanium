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

#if USING_CCTESTS
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef OS_ANDROID
  // The notification API has not been implemented for Android.
#else
#include "gears/desktop/desktop_test.h"

#include <queue>

#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/string16.h"
#include "gears/desktop/notification_message_orderer.h"
#include "gears/notifier/notification.h"
#include "gears/notifier/notifier_proxy.h"

bool g_ok = true;
std::string16 *g_error = NULL;
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
do { \
  if (!(b)) { \
    g_ok =false; \
    LOG(("TestNotificationMessageOrdering failed at line %d\n", __LINE__)); \
    *g_error += STRING16(L"TestNotificationMessageOrdering failed due to "); \
    *g_error += STRING16(L## #b); \
    *g_error += STRING16(L"<br/>"); \
  } \
} while(false)

class NotifierProxyMock : public NotifierProxyInterface {
 public:
  NotifierProxyMock() {}
  ~NotifierProxyMock() {
    TEST_ASSERT(expected_notification_count() == 0);
  }

  virtual void PostNotification(int message_type,
                                GearsNotification *notification) {
    TEST_ASSERT(expected_notification_count() > 0);
    if (expected_notification_count() > 0) {
      TEST_ASSERT(notification->title() == expected_titles.front());
      expected_titles.pop();
    }
    delete notification;
  }

  void AddExpectedTitle(const char16 *title) {
    expected_titles.push(title);
  }

  int expected_notification_count() const {
    return expected_titles.size();
  }
 private:
  std::queue<std::string16> expected_titles;
  DISALLOW_EVIL_CONSTRUCTORS(NotifierProxyMock);
};


GearsNotification *CreateNotification(const char16 *origin_url,
                                      const char16 *id,
                                      const char16 *title) {
  GearsNotification *notification = new GearsNotification();
  SecurityOrigin origin;
  TEST_ASSERT(origin.InitFromUrl(origin_url));
  notification->set_security_origin(origin);
  notification->set_id(id);
  notification->set_title(title);
  return notification;
}

bool TestNotificationMessageOrdering(std::string16 *error) {
  assert(error);
  g_error = error;
  g_ok = true;

  {
    // Test that post for the same notification id happen in order
    // (and do not block posts for other ids).
    NotifierProxyMock *proxy = new NotifierProxyMock();  // owned by orderer
    NotificationMessageOrderer orderer(proxy);

    //   Add pending n1 (reservation = 1)
    GearsNotification *notification1_reservation1 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t1"));
    int64 reservation1 = orderer.AddReservation(*notification1_reservation1);

    //   Add pending n1 (reservation = 2)
    GearsNotification *notification1_reservation2 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t2"));
    int64 reservation2 = orderer.AddReservation(*notification1_reservation2);

    //   Add pending n2 (reservation = 3)
    GearsNotification *notification2_reservation3 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n2"),
                           STRING16(L"n2_t3"));
    int64 reservation3 = orderer.AddReservation(*notification2_reservation3);

    //   Post n2 (reservation = 3)
    //      n2 should go to the proxy.
    proxy->AddExpectedTitle(STRING16(L"n2_t3"));
    orderer.PostNotification(kDesktop_AddNotification,
                             notification2_reservation3,
                             reservation3);
    TEST_ASSERT(proxy->expected_notification_count() == 0);

    //   Post n1 (reservation = 2)
    //      n1 (reservation = 2) should go to the proxy.
    proxy->AddExpectedTitle(STRING16(L"n1_t2"));
    orderer.PostNotification(kDesktop_AddNotification,
                             notification1_reservation2,
                             reservation2);
    TEST_ASSERT(proxy->expected_notification_count() == 0);

    //   Post n1 (reservation = 1)
    //      Nothing should happen.
    orderer.PostNotification(kDesktop_AddNotification,
                             notification1_reservation1,
                             reservation1);
    TEST_ASSERT(proxy->expected_notification_count() == 0);
  }


  {
    // Test that removing a pending notification allows others with the
    // same id to go through.
    NotifierProxyMock *proxy = new NotifierProxyMock();  // owned by orderer
    NotificationMessageOrderer orderer(proxy);

    //   Add pending n1 (reservation = 1)
    GearsNotification *notification1_reservation1 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t1"));
    int64 reservation1 = orderer.AddReservation(*notification1_reservation1);
    delete notification1_reservation1;
    notification1_reservation1 = NULL;

    //   Add pending n1 (reservation = 2)
    GearsNotification *notification1_reservation2 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t2"));
    int64 reservation2 = orderer.AddReservation(*notification1_reservation2);

    //   Add pending n1 (reservation = 3)
    GearsNotification *notification1_reservation3 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t3"));
    int64 reservation3 = orderer.AddReservation(*notification1_reservation3);

    //   Post n1 (reservation = 3)
    //      n1 (reservation = 3) should go to the proxy.
    proxy->AddExpectedTitle(STRING16(L"n1_t3"));
    orderer.PostNotification(kDesktop_AddNotification,
                             notification1_reservation3,
                             reservation3);
    TEST_ASSERT(proxy->expected_notification_count() == 0);

    //   Post n1 (reservation = 2)
    //      Nothing should happen. (n1 with reservation = 3 superceeded it.)
    orderer.PostNotification(kDesktop_AddNotification,
                             notification1_reservation2,
                             reservation2);

    //   Remove pending n1 (reservation = 1)
    //      Nothing should happen. (n1 with reservation = 3 superceeded it.)
    orderer.RemoveReservation(reservation1);
  }


  {
    // Test that the remove message removes pending notifications
    // with the same id (and only that id).
    NotifierProxyMock *proxy = new NotifierProxyMock();  // owned by orderer
    NotificationMessageOrderer orderer(proxy);

    //   Add pending n1 (reservation = 1)
    GearsNotification *notification1_reservation1 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t1"));
    int64 reservation1 = orderer.AddReservation(*notification1_reservation1);

    //   Add pending n2 (reservation = 2)
    GearsNotification *notification2_reservation2 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n2"),
                           STRING16(L"n2_t2"));
    int64 reservation2 = orderer.AddReservation(*notification2_reservation2);

    //   Post removal message for n1.
    //     The remove message should be sent to the proxy.
    GearsNotification *notification1 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_remove"));
    proxy->AddExpectedTitle(STRING16(L"n1_remove"));
    orderer.PostNotification(kDesktop_RemoveNotification,
                             notification1,
                             NotificationMessageOrderer::kInvalidReservation);
    TEST_ASSERT(proxy->expected_notification_count() == 0);

    //   Post n1 (reservation = 1)
    //      No notification should go to the proxy (because
    //      it was removed).
    orderer.PostNotification(kDesktop_AddNotification,
                             notification1_reservation1,
                             reservation1);

    //   Post n2 (reservation = 2)
    //      n2 (reservation = 2) should go to the proxy.
    proxy->AddExpectedTitle(STRING16(L"n2_t2"));
    orderer.PostNotification(kDesktop_AddNotification,
                             notification2_reservation2,
                             reservation2);
    TEST_ASSERT(proxy->expected_notification_count() == 0);
  }

  {
    // Test posting a notification leaving other pending items alone.
    NotifierProxyMock *proxy = new NotifierProxyMock();  // owned by orderer
    NotificationMessageOrderer orderer(proxy);

    //   Add pending n1 (reservation = 1)
    GearsNotification *notification1_reservation1 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t1"));
    int64 reservation1 = orderer.AddReservation(*notification1_reservation1);

    //   Add pending n1 (reservation = 2)
    GearsNotification *notification1_reservation2 =
        CreateNotification(STRING16(L"http://www.google.com/"),
                           STRING16(L"n1"),
                           STRING16(L"n1_t2"));
    int64 reservation2 = orderer.AddReservation(*notification1_reservation2);

    //   Post n1 (reservation = 1)
    //      Nothing should happen.  n1 with reservation 2 has superceeded it.)
    orderer.PostNotification(kDesktop_AddNotification,
                             notification1_reservation1,
                             reservation1);

    //   Post n1 (reservation = 2)
    //      n1 (reservation = 2) should go to the proxy.
    proxy->AddExpectedTitle(STRING16(L"n1_t2"));
    orderer.PostNotification(kDesktop_AddNotification,
                             notification1_reservation2,
                             reservation2);
    TEST_ASSERT(proxy->expected_notification_count() == 0);
  }

  g_error = NULL;
  return g_ok;
}

#endif  // OS_ANDROID
#endif  // OFFICIAL_BUILD
#endif  // USING_CCTESTS
