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

#ifdef USING_CCTESTS

#include "gears/base/common/message_queue.h"
#include "gears/base/common/message_service.h"
#include "gears/base/common/string16.h"

class TestNotification : public NotificationData {
 public:
  TestNotification(const char *s) : data_string_(s) {}
  std::string data_string_;

  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_NULL;
  }

  virtual bool Serialize(Serializer *out) const {
    return false;
  }

  virtual bool Deserialize(Deserializer *in) {
    return false;
  }
};

class TestObserver : public MessageObserverInterface {
 public:
  TestObserver(ThreadMessageQueue *message_queue)
    : message_queue_(message_queue),
      remove_self_(false),
      total_received_(0) {
  }

  ThreadMessageQueue *message_queue_;
  bool remove_self_;
  int total_received_;
  ThreadId last_thread_received_;
  std::string16 last_topic_received_;
  std::string last_data_received_;

  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data) {
    const TestNotification *test_data =
              static_cast<const TestNotification*>(data);
    ++total_received_;
    last_thread_received_ = message_queue_->GetCurrentThreadId();
    last_topic_received_ = topic;
    last_data_received_ = test_data->data_string_;
    if (remove_self_) {
      service->RemoveObserver(this, topic);
    }
  }
};

bool TestMessageService(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestMessageService - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestMessageService - failed. "); \
    return false; \
  } \
}

  const char16 *kTopic1 = STRING16(L"topic1");
  const char16 *kTopic2 = STRING16(L"topic2");
  const char16 *kTopic3 = STRING16(L"topic3");
  const char16 *kTopic4 = STRING16(L"topic4");

  // TODO(michaeln): We need an decent abstractions Thread and ThreadId
  const ThreadId kThreadId1((ThreadId)1);
  const ThreadId kThreadId2((ThreadId)2);
  const ThreadId kThreadId3((ThreadId)3);

  MockThreadMessageQueue mock_message_queue;
  MessageService message_service(&mock_message_queue, NULL);

  TestObserver observer(&mock_message_queue);
  mock_message_queue.SetMockCurrentThreadId(kThreadId1);
  // We should only be able to add the same observer once per topic per thread
  TEST_ASSERT(message_service.AddObserver(&observer, kTopic1));
  TEST_ASSERT(!message_service.AddObserver(&observer, kTopic1));
  // We should only be able to remove from the same thread
  mock_message_queue.SetMockCurrentThreadId(kThreadId2);
  TEST_ASSERT(!message_service.RemoveObserver(&observer, kTopic1));
  mock_message_queue.SetMockCurrentThreadId(kThreadId1);
  TEST_ASSERT(message_service.RemoveObserver(&observer, kTopic1));
  // Should not be able to remove an observer that is not registered
  TEST_ASSERT(!message_service.RemoveObserver(&observer, kTopic1));
  // A removed observer should not receive notifications
  message_service.NotifyObservers(kTopic1, new TestNotification("deaf ears"));
  mock_message_queue.DeliverMockMessages();
  TEST_ASSERT(observer.total_received_ == 0);

  // Test RemoveObserversForThread
  mock_message_queue.SetMockCurrentThreadId(kThreadId2);
  TEST_ASSERT(message_service.AddObserver(&observer, kTopic1));
  message_service.RemoveObserversForThread(kThreadId1);
  message_service.RemoveObserversForThread(kThreadId2);
  message_service.RemoveObserversForThread(kThreadId3);
  message_service.NotifyObservers(kTopic1, new TestNotification("deaf ears"));
  mock_message_queue.DeliverMockMessages();
  TEST_ASSERT(observer.total_received_ == 0);
  
  // Add a single observer for topic1 on thread1
  TestObserver observer1(&mock_message_queue);
  mock_message_queue.SetMockCurrentThreadId(kThreadId1);
  TEST_ASSERT(message_service.AddObserver(&observer1, kTopic1));

  // Add a pair of observers for topic2 on thread2
  TestObserver observer2a(&mock_message_queue);
  TestObserver observer2b(&mock_message_queue);
  observer2b.remove_self_ = true;
  mock_message_queue.SetMockCurrentThreadId(kThreadId2);
  TEST_ASSERT(message_service.AddObserver(&observer2a, kTopic2))
  TEST_ASSERT(message_service.AddObserver(&observer2b, kTopic2));

  // Add three observers for topic3, all on different threads
  TestObserver observer3a(&mock_message_queue);
  TestObserver observer3b(&mock_message_queue);
  TestObserver observer3c(&mock_message_queue);
  mock_message_queue.SetMockCurrentThreadId(kThreadId1);
  TEST_ASSERT(message_service.AddObserver(&observer3a, kTopic3));
  mock_message_queue.SetMockCurrentThreadId(kThreadId2);
  TEST_ASSERT(message_service.AddObserver(&observer3b, kTopic3));
  mock_message_queue.SetMockCurrentThreadId(kThreadId3);
  TEST_ASSERT(message_service.AddObserver(&observer3c, kTopic3));

  // send some notifications
  message_service.NotifyObservers(kTopic1, new TestNotification("1.1"));
  message_service.NotifyObservers(kTopic2, new TestNotification("2.1"));
  message_service.NotifyObservers(kTopic2, new TestNotification("2.2"));
  message_service.NotifyObservers(kTopic3, new TestNotification("3.1"));
  message_service.NotifyObservers(kTopic3, new TestNotification("3.2"));
  message_service.NotifyObservers(kTopic3, new TestNotification("3.3"));
  message_service.NotifyObservers(kTopic4, new TestNotification("deaf ears"));
  mock_message_queue.DeliverMockMessages();

  // examine what was recieved on what thread

  TEST_ASSERT(observer1.total_received_ == 1);
  TEST_ASSERT(observer1.last_thread_received_ == kThreadId1);
  TEST_ASSERT(observer1.last_topic_received_ == kTopic1);
  TEST_ASSERT(observer1.last_data_received_ == "1.1");

  TEST_ASSERT(observer2a.total_received_ == 2);
  TEST_ASSERT(observer2a.last_thread_received_ == kThreadId2);
  TEST_ASSERT(observer2a.last_topic_received_ == kTopic2);
  TEST_ASSERT(observer2a.last_data_received_ == "2.2");

  TEST_ASSERT(observer2b.total_received_ == 1);
  TEST_ASSERT(observer2b.last_thread_received_ == kThreadId2);
  TEST_ASSERT(observer2b.last_topic_received_ == kTopic2);
  TEST_ASSERT(observer2b.last_data_received_ == "2.1");

  TEST_ASSERT(observer3a.total_received_ == 3);
  TEST_ASSERT(observer3a.last_thread_received_ == kThreadId1);
  TEST_ASSERT(observer3a.last_topic_received_ == kTopic3);
  TEST_ASSERT(observer3a.last_data_received_ == "3.3");

  TEST_ASSERT(observer3b.total_received_ == 3);
  TEST_ASSERT(observer3b.last_thread_received_ == kThreadId2);
  TEST_ASSERT(observer3b.last_topic_received_ == kTopic3);
  TEST_ASSERT(observer3b.last_data_received_ == "3.3");

  TEST_ASSERT(observer3c.total_received_ == 3);
  TEST_ASSERT(observer3c.last_thread_received_ == kThreadId3);
  TEST_ASSERT(observer3c.last_topic_received_ == kTopic3);
  TEST_ASSERT(observer3c.last_data_received_ == "3.3");

  return true;
}

#endif  // USING_CCTESTS
