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

#ifndef GEARS_OBSERVER_SERVICE_COMMON_MESSAGE_SERVICE_H__
#define GEARS_OBSERVER_SERVICE_COMMON_MESSAGE_SERVICE_H__

#include <map>
#include "gears/base/common/basictypes.h" // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/serialization.h"
#include "gears/base/common/string16.h"
#include "third_party/linked_ptr/linked_ptr.h"

class MessageService;
class ObserverCollection;
class SharedNotificationData;

typedef Serializable NotificationData;

// Clients implement this interface to receive notification
// messages published for a topic.
class MessageObserverInterface {
 public:
  // The notification callback. It is valid to add/remove observers,
  // including 'this', from within the callback method.
  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data) = 0;
};


// The mediator in a publisher/subscriber model.
// TODO(michaeln): Perhaps add an optional SecurityOrigin* parameter
// to AddObserver, RemoveObserver, and NotifyObservers. And build
// a Gears module that surfaces this interface to script. Internal
// c++ callers would pass NULL for security origin to segregate script
// based usage from our internal usage.
class MessageService : public ThreadMessageQueue::HandlerInterface,
                              IpcMessageQueue::HandlerInterface {
 public:
  // Returns a pointer to the MessageService singleton
  static MessageService *GetInstance();

  // Adds the observer to the set of observers that will be notified
  // for a given topic. The observers's OnNotify method will be called 
  // on the current thread. Returns false if this observer is already
  // regsitered for this topic on the current thread.
  bool AddObserver(MessageObserverInterface *observer, const char16 *topic);

  // Removes the observer from the set of observers for a given topic.
  // This method must be called on the same thread that AddObserver
  // was called on. Returns false if this observer is not regsitered
  // for this topic on the current thread.
  bool RemoveObserver(MessageObserverInterface *observer, const char16 *topic);

  // Asynchronously delivers a notification to all observers for the
  // given topic. Each observer's OnNotify method will be invoked on
  // the thread on which it was added. For windows developers, you can
  // think of this thread as the observer's apartment thread.
  // Ownership of the data is transferred to the message service.
  // Upon return from this method, callers should no longer touch data.
  void NotifyObservers(const char16 *topic, NotificationData *data);

 private:
  // The intent is for this class to be a singleton, but for testing
  // purposes, the constructor and destructor are made available.
  // (see message_service_test.cc).
  MessageService(ThreadMessageQueue *message_queue,
                 IpcMessageQueue *ipc_message_queue);
  ~MessageService();
  friend bool TestMessageService(std::string16 *error);

  friend class ObserverCollection;
  typedef std::map<std::string16, linked_ptr<ObserverCollection> >
              TopicObserverMap;

  void NotifyObserversImpl(SharedNotificationData *shared_data,
                           bool send_ipc);

  ObserverCollection *GetTopicObserverCollection(const char16 *topic,
                                                 bool create_if_needed);
  void DeleteTopicObserverCollection(const char16 *topic);

  void RemoveObserversForThread(ThreadId id);

  // ThreadMessageQueue::HandlerInterface override
  virtual void HandleThreadMessage(int message_type,
                                   MessageData *message_data);

  // IpcMessageQueue::HandlerInterface override
  virtual void HandleIpcMessage(IpcProcessId source_process_id,
                                int message_type,
                                const IpcMessageData *message_data);

  static void ThreadEndHook(void* value);
  void InitThreadEndHook();

  Mutex observer_collections_mutex_;
  TopicObserverMap observer_collections_;
  ThreadMessageQueue *message_queue_;
  IpcMessageQueue *ipc_message_queue_;
  DISALLOW_EVIL_CONSTRUCTORS(MessageService);
};

#endif  // GEARS_OBSERVER_SERVICE_COMMON_MESSAGE_SERVICE_H__
