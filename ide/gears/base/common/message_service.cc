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

#include <assert.h>
#include <set>
#include "gears/base/common/atomic_ops.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/message_service.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/thread_locals.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

static const ThreadLocals::Slot kThreadLocalKey = ThreadLocals::Alloc();

typedef std::set<MessageObserverInterface*> ObserverSet;
typedef std::map<ThreadId, ObserverSet> ThreadObserversMap;

// When NotifyObservers is called, the message service needs to broadcast
// the notification to observers on multiple threads. It uses the thread
// message queue to do so. However instead of creating multiples copies of
// the data our client wishes to broadcast, we send a message which contains
// a shared reference to the callers original data to each thread. In this
// way, we avoid making copies of the callers original data.
// See NotificationMessage.
class SharedNotificationData : public RefCounted {
 public:
  SharedNotificationData() {
#ifdef DEBUG
    g_instance_count_.Ref();
#endif
  }

  SharedNotificationData(const char16 *topic, NotificationData *data)
      : topic_(topic), data_(data) {
#ifdef DEBUG
    g_instance_count_.Ref();
#endif
  }

#ifdef DEBUG
  ~SharedNotificationData() {
    g_instance_count_.Unref();
  }
#endif 

  std::string16 topic_;
  scoped_ptr<NotificationData> data_;
#ifdef DEBUG
  static RefCount g_instance_count_;
#endif
};

#ifdef DEBUG
// Instrumentation to observe that object lifecycles are working as expected
// TODO(michaeln): remove this instrumentation once comfortable that things
// are working as they should be
RefCount SharedNotificationData::g_instance_count_;
#endif


class NotificationMessage : public Serializable {
 public:
  NotificationMessage() : shared_(new SharedNotificationData) {}
  NotificationMessage(SharedNotificationData *shared) : shared_(shared) {}

  scoped_refptr<SharedNotificationData> shared_;

  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_NOTIFICATION;
  }

  virtual bool Serialize(Serializer *out) const {
    if (!shared_) return false;
    out->WriteString(shared_->topic_.c_str());
    return out->WriteObject(shared_->data_.get());
  }

  virtual bool Deserialize(Deserializer *in) {
    NotificationData *data = NULL;
    if (!in->ReadString(&shared_->topic_) ||
        !in->CreateAndReadObject(&data)) {
      return false;
    }
    shared_->data_.reset(data);
    return true;
  }

  static Serializable *New() {
    return new NotificationMessage;
  }

  static void RegisterAsSerializable() {
    Serializable::RegisterClass(SERIALIZABLE_NOTIFICATION, New);
  }  
};


// Helper class used by MessageService. The MessageService creates
// a collection for each unique topic. All observers registered for
// a particular topic are stored in the same collection.
class ObserverCollection {
 public:
  ObserverCollection(MessageService *service)
    : service_(service) {}

  bool Add(MessageObserverInterface *observer);
  bool Remove(MessageObserverInterface *observer);
  void RemoveObserversForThread(ThreadId thread_id);
  bool IsEmpty() const;

  void PostThreadNotifications(SharedNotificationData *shared_data);
  void ProcessThreadNotification(NotificationMessage *notification);

 private:
  MessageService *service_;

  // This class organizes all observers for a topic into distinct sets, with
  // one set containing all of the observers for a particular thread.
  ThreadObserversMap observer_sets_;

  ObserverSet *GetThreadObserverSet(ThreadId thread_id, bool create_if_needed);
  ObserverSet *GetCurrentThreadObserverSet(bool create_if_needed) {
    return GetThreadObserverSet(service_->message_queue_->GetCurrentThreadId(),
                                create_if_needed);
  }

  DISALLOW_EVIL_CONSTRUCTORS(ObserverCollection);
};


static Mutex g_service_lock;
static MessageService *g_service = NULL;

// static
MessageService *MessageService::GetInstance() {
  MutexLock locker(&g_service_lock);
  if (!g_service) {
    g_service = new MessageService(ThreadMessageQueue::GetInstance(),
                                   IpcMessageQueue::GetPeerQueue());
  }
  return g_service;
}


MessageService::MessageService(ThreadMessageQueue *message_queue,
                               IpcMessageQueue *ipc_message_queue)
    : message_queue_(message_queue),
      ipc_message_queue_(ipc_message_queue) {
  message_queue_->RegisterHandler(kMessageService_Notify, this);
  if (ipc_message_queue_) {
    NotificationMessage::RegisterAsSerializable();
    ipc_message_queue_->RegisterHandler(kMessageService_IpcNotify, this);
  }
}


MessageService::~MessageService() {
}


bool MessageService::AddObserver(MessageObserverInterface *observer,
                                 const char16 *topic) {
  if (!topic || !*topic) return false;
  MutexLock lock(&observer_collections_mutex_);
  ObserverCollection *topic_observers =
                          GetTopicObserverCollection(topic, true);
  return topic_observers->Add(observer);
}


bool MessageService::RemoveObserver(MessageObserverInterface *observer,
                                    const char16 *topic) {
  if (!topic || !*topic) return false;
  MutexLock lock(&observer_collections_mutex_);
  ObserverCollection *topic_observers =
                          GetTopicObserverCollection(topic, false);
  if (!topic_observers) return false;
  bool removed = topic_observers->Remove(observer);
  if (removed) {
    if (topic_observers->IsEmpty()) {
      DeleteTopicObserverCollection(topic);
    }
  }
  return removed;
}


void MessageService::NotifyObservers(const char16 *topic,
                                     NotificationData *data) {
  scoped_refptr<SharedNotificationData>
      shared_data(new SharedNotificationData(topic, data));
  NotifyObserversImpl(shared_data.get(), true);
}


void MessageService::NotifyObserversImpl(SharedNotificationData *shared_data,
                                         bool send_ipc) {
  if (send_ipc && ipc_message_queue_) {
    ipc_message_queue_->SendToAll(kMessageService_IpcNotify,
                                  new NotificationMessage(shared_data),
                                  false);
  }

  MutexLock lock(&observer_collections_mutex_);
  ObserverCollection *topic_observers =
      GetTopicObserverCollection(shared_data->topic_.c_str(), false);
  if (!topic_observers) { 
    return;
  }
  topic_observers->PostThreadNotifications(shared_data);
}


void MessageService::HandleIpcMessage(IpcProcessId source_process_id,
                                      int message_type,
                                      const IpcMessageData *message_data) {
  assert(message_type == kMessageService_IpcNotify);
  assert(source_process_id != ipc_message_queue_->GetCurrentIpcProcessId());
  const NotificationMessage *notification =
                          static_cast<const NotificationMessage*>(message_data);
  NotifyObserversImpl(notification->shared_.get(), false);
}


void MessageService::HandleThreadMessage(int message_type,
                                         MessageData *message_data) {
  MutexLock lock(&observer_collections_mutex_);
  assert(message_type == kMessageService_Notify);
  NotificationMessage *notification =
                          static_cast<NotificationMessage*>(message_data);
  const char16 *topic = notification->shared_->topic_.c_str();
  ObserverCollection *topic_observers =
                          GetTopicObserverCollection(topic, false);
  if (!topic_observers) return;
  topic_observers->ProcessThreadNotification(notification);
  return;
}


ObserverCollection *MessageService::GetTopicObserverCollection(
                                        const char16 *topic,
                                        bool create_if_needed) {
  // assert(mutex_.IsLockedByCurrentThread());
  std::string16 key(topic);
  TopicObserverMap::const_iterator found = observer_collections_.find(key);
  if (found != observer_collections_.end()) {
    return found->second.get();
  } else if (!create_if_needed) {
    return NULL;
  } else {
    ObserverCollection *collection = new ObserverCollection(this);
    observer_collections_[key] = linked_ptr<ObserverCollection>(collection);
    return collection;
  }
  // unreachable
}


void MessageService::DeleteTopicObserverCollection(const char16 *topic) {
  // assert(mutex_.IsLockedByCurrentThread());
  std::string16 key(topic);
  observer_collections_.erase(key);
}


void MessageService::RemoveObserversForThread(ThreadId thread_id) {
  MutexLock lock(&observer_collections_mutex_);
  TopicObserverMap::iterator iter = observer_collections_.begin();
  while (iter != observer_collections_.end()) {
    iter->second.get()->RemoveObserversForThread(thread_id);
    TopicObserverMap::iterator prev = iter++;
    if (prev->second.get()->IsEmpty()) {
      observer_collections_.erase(prev);
    }
  }
}


// static
void MessageService::ThreadEndHook(void* value) {
  ThreadId *id = reinterpret_cast<ThreadId*>(value);
  if (id) {
    GetInstance()->RemoveObserversForThread(*id);
    delete id;
  }
}


void MessageService::InitThreadEndHook() {
  // We use a ThreadLocal to get called when an OS thread terminates
  // and use that opportunity to remove all observers that remain
  // registered on that thread.
  //
  // We store the thread id in the ThreadLocal variable because on some
  // OSes (linux), the destructor proc is called from a different thread,
  // and on others (windows), the destructor proc is called from the
  // terminating thread.
  //
  // Also, we only do this for the actual singleton instance of the
  // MessageService class as opposed to instances created for unit testing.
  if (GetInstance() == this) {
    if (!ThreadLocals::HasValue(kThreadLocalKey)) {
      ThreadId *id = new ThreadId(message_queue_->GetCurrentThreadId());
      ThreadLocals::SetValue(kThreadLocalKey, id, &ThreadEndHook);
    }
  }
}


bool ObserverCollection::Add(MessageObserverInterface *observer) {
  // assert(serveice_->mutex_.IsLockedByCurrentThread());
  ObserverSet *set = GetCurrentThreadObserverSet(true);
  if (set->find(observer) != set->end())
    return false;  // observer is already registered on this thread
  set->insert(observer);
  return true;
}


bool ObserverCollection::Remove(MessageObserverInterface *observer) {
  // assert(serveice_->mutex_.IsLockedByCurrentThread());
  ThreadId thread_id = service_->message_queue_->GetCurrentThreadId();
  ObserverSet *set = GetThreadObserverSet(thread_id, false);
  if (!set) return false;
  ObserverSet::iterator found = set->find(observer);
  if (found == set->end())
    return false;  // observer is not registered on this thread
  set->erase(found);
  if (set->empty())
    observer_sets_.erase(thread_id);
  return true;
}


bool ObserverCollection::IsEmpty() const {
  // assert(serveice_->mutex_.IsLockedByCurrentThread());
  return observer_sets_.empty();
}


ObserverSet *ObserverCollection::GetThreadObserverSet(ThreadId thread_id,
                                                      bool create_if_needed) {
  ThreadObserversMap::iterator found = observer_sets_.find(thread_id);
  if (found != observer_sets_.end()) {
    return &found->second;
  } else if (!create_if_needed) {
    return NULL;
  } else {
    assert(service_->message_queue_->GetCurrentThreadId() == thread_id);
    service_->InitThreadEndHook();
    service_->message_queue_->InitThreadMessageQueue();
    observer_sets_[thread_id] = ObserverSet();
    return &observer_sets_[thread_id];
  }
}


void ObserverCollection::RemoveObserversForThread(ThreadId thread_id) {
  // assert(service_->observer_collections_mutex_.IsLockedByCurrentThread());
  observer_sets_.erase(thread_id);
}


void ObserverCollection::PostThreadNotifications(
                             SharedNotificationData *shared_data) {
  // assert(serveice_->mutex_.IsLockedByCurrentThread());
  // Send one message for each thread containing observers of this topic
  ThreadObserversMap::iterator iter;
  for (iter = observer_sets_.begin(); iter != observer_sets_.end(); ++iter) {
    service_->message_queue_->Send(iter->first, kMessageService_Notify,
                                   new NotificationMessage(shared_data));
  }
}


void ObserverCollection::ProcessThreadNotification(
                             NotificationMessage *notification) {
  // assert(serveice_->mutex_.IsLockedByCurrentThread());
  const char16 *topic = notification->shared_->topic_.c_str();
  const NotificationData *data = notification->shared_->data_.get();

  // Dispatch this notification to all topic observers in this thread.
  //
  // Note, this ObserverCollection instance can be modified or deleted
  // during the iteration below because we unlock and relock the service's
  // mutex around the calls to observer->OnNotify. We guard against this be
  // making local copies of instance data we need, and by testing for whether 
  // or not the collection has been deleted or a particular observer has been
  // removed from the collection prior to calling OnNotify.

  ObserverSet *set = GetCurrentThreadObserverSet(false);
  if (!set) return;

  MessageService *service = service_;
  ObserverSet observer_set_copy = *set;

  ObserverSet::iterator iter;
  for (iter = observer_set_copy.begin();
       iter != observer_set_copy.end();
       ++iter) {
    // ensure this collection and the set for this thread has not been deleted
    ObserverCollection *should_be_us =
                            service->GetTopicObserverCollection(topic, false);
    if (should_be_us != this) {
      return;
    }
    ObserverSet *should_be_same_set = GetCurrentThreadObserverSet(false);
    if (should_be_same_set != set) {
      return;
    }

    // ensure this observer has not been removed
    if (set->find(*iter) != set->end()) {
      service->observer_collections_mutex_.Unlock();
      (*iter)->OnNotify(service, topic, data);
      service->observer_collections_mutex_.Lock();    
    }
  }
}
