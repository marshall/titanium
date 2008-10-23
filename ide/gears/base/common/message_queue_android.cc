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

#ifdef OS_ANDROID

#include <assert.h>
#include <deque>
#include <map>
#include <unistd.h>

#include "gears/base/common/message_queue.h"

#include "gears/base/android/java_jni.h"
#include "gears/base/common/event.h"
#include "gears/base/common/thread.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/npapi/module.h"
#include "third_party/linked_ptr/linked_ptr.h"
#include "third_party/npapi/nphostapi.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

static const ThreadLocals::Slot kThreadLocalKey = ThreadLocals::Alloc();

class ThreadSpecificQueue;

// A concrete (and naive) implementation that uses Event.
class AndroidThreadMessageQueue : public ThreadMessageQueue {
 public:
  AndroidThreadMessageQueue();
  ThreadId MainThreadId();

  // ThreadMessageQueue
  virtual bool InitThreadMessageQueue();

  virtual ThreadId GetCurrentThreadId();

  virtual bool Send(ThreadId thread_handle,
                    int message_type,
                    MessageData *message);

  void HandleThreadMessage(int message_type, MessageData *message_data);

 private:

  void InitThreadEndHook();
  static void ThreadEndHook(void* value);
  static ThreadSpecificQueue* GetQueue(ThreadId thread_id);

  static Mutex queue_mutex_;  // Protects the message_queues_ map.
  typedef std::map<ThreadId, linked_ptr<ThreadSpecificQueue> > QueueMap;
  static QueueMap message_queues_;
  ThreadId  main_thread_id_;

  friend class ThreadSpecificQueue;
  friend class AndroidMessageLoop;
};

// A message queue for a thread.
class ThreadSpecificQueue {
 public:
  ThreadSpecificQueue(ThreadId thread_id) : thread_id_(thread_id) { }
  // Sends the message to the thread owning this queue.
  void SendMessage(int message_type, MessageData* message_data);
  // Dispatches messages in a loop until exit_type is received.
  void GetAndDispatchMessages(int exit_type);
  // Waits for one or messages to arrive and dispatches them to the
  // appropriate handlers.
  void RunOnce();
  // Dispatches them to the appropriate handlers.
  void DispatchOnce();

 protected:
  struct Message {
    Message(int type, MessageData* data)
        : message_type(type),
          message_data(data) {
    }
    int message_type;
    linked_ptr<MessageData> message_data;
  };

 private:
  DISALLOW_EVIL_CONSTRUCTORS(ThreadSpecificQueue);

  // The ThreadId owning this queue.
  ThreadId thread_id_;
  // Protects event_queue_.
  Mutex event_queue_mutex_;
  // Queue of messages for this thread.
  std::deque<Message> event_queue_;
  // Event signalled when the queue is filled.
  Event event_;
  // Keep track of whether a call to NPN_PluginThreadAsyncCall is
  // in-flight, to prevent spamming the browser's message queue.
  static Mutex async_mutex_;
  static bool async_in_flight_;

  // Atomically move all messages from the queue to the return structure.
  void PopMessages(std::deque<Message> *messages);
  // Asynchronous callback on the main thread after
  // NPN_PluginThreadAsyncCall. This calls RunOnce() on the main
  // thread's message queue.
  static void AsyncCallback(void* instance);
};

// The thread message queue singleton.
static AndroidThreadMessageQueue g_instance;


//------------------------------------------------------------------------------
// AndroidThreadMessageQueue

Mutex AndroidThreadMessageQueue::queue_mutex_;
AndroidThreadMessageQueue::QueueMap AndroidThreadMessageQueue::message_queues_;

AndroidThreadMessageQueue::AndroidThreadMessageQueue() {
  main_thread_id_ = GetCurrentThreadId();
}

ThreadId AndroidThreadMessageQueue::MainThreadId() {
  return main_thread_id_;
}

// static
ThreadMessageQueue *ThreadMessageQueue::GetInstance() {
  return &g_instance;
}

bool AndroidThreadMessageQueue::InitThreadMessageQueue() {
  MutexLock lock(&queue_mutex_);
  ThreadId thread_id = GetCurrentThreadId();
  if (message_queues_.find(thread_id) == message_queues_.end()) {
    ThreadSpecificQueue* queue = new ThreadSpecificQueue(thread_id);
    if (thread_id != MainThreadId()) {
      InitThreadEndHook();
    }
    message_queues_[thread_id] = linked_ptr<ThreadSpecificQueue>(queue);
  }
  return true;
}

ThreadId AndroidThreadMessageQueue::GetCurrentThreadId() {
  return pthread_self();
}

bool AndroidThreadMessageQueue::Send(ThreadId thread,
                                     int message_type,
                                     MessageData *message_data) {
  MutexLock lock(&queue_mutex_);

  // Find the queue for the target thread.
  AndroidThreadMessageQueue::QueueMap::iterator queue;
  queue = message_queues_.find(thread);
  if (queue == message_queues_.end()) {
    delete message_data;
    return false;
  }
  queue->second->SendMessage(message_type, message_data);
  return true;
}

void AndroidThreadMessageQueue::HandleThreadMessage(int message_type,
                                                    MessageData *message_data) {
  RegisteredHandler handler;
  if (GetRegisteredHandler(message_type, &handler)) {
    handler.Invoke(message_type, message_data);
  }
}

void AndroidThreadMessageQueue::InitThreadEndHook() {
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
  if (!ThreadLocals::HasValue(kThreadLocalKey)) {
    ThreadId *id = new ThreadId(GetCurrentThreadId());
    ThreadLocals::SetValue(kThreadLocalKey, id, &ThreadEndHook);
  }
}

ThreadSpecificQueue* AndroidThreadMessageQueue::GetQueue(ThreadId id) {
  ThreadSpecificQueue* queue = NULL;
  {
    MutexLock lock(&queue_mutex_);
    // Find the queue for this thread.
    QueueMap::iterator queue_iter;
    queue_iter = message_queues_.find(id);
    assert (queue_iter != message_queues_.end());
    queue = queue_iter->second.get();
  }
  assert(queue);
  return queue;
}

// static
void AndroidThreadMessageQueue::ThreadEndHook(void* value) {
  ThreadId *id = reinterpret_cast<ThreadId*>(value);
  if (id) {
    MutexLock lock(&queue_mutex_);
    message_queues_.erase(*id);
    delete id;
  }
}

//------------------------------------------------------------------------------
// AndroidMessageLoop

// static
void AndroidMessageLoop::Start() {
  // Start the loop on the current thread.
  ThreadId thread_id =
      AndroidThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  AndroidThreadMessageQueue::GetQueue(thread_id)->GetAndDispatchMessages(
      kAndroidLoop_Exit);
}

// static
void AndroidMessageLoop::RunOnce() {
  // Run the loop once on the current thread.
  ThreadId thread_id =
      AndroidThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  AndroidThreadMessageQueue::GetQueue(thread_id)->RunOnce();
}

// static
void AndroidMessageLoop::Stop(ThreadId thread_id) {
  // Stop the target thread's loop.
  AndroidThreadMessageQueue::GetQueue(thread_id)->SendMessage(kAndroidLoop_Exit,
                                                              NULL);
}

//------------------------------------------------------------------------------
// ThreadSpecificQueue

Mutex ThreadSpecificQueue::async_mutex_;
bool ThreadSpecificQueue::async_in_flight_ = false;

void ThreadSpecificQueue::SendMessage(int message_type,
                                      MessageData* message_data) {
  // Put a message in the queue. Note that the Message object
  // takes ownership of message_data.
  {
    MutexLock lock(&event_queue_mutex_);
    event_queue_.push_back(Message(message_type, message_data));
  }
  event_.Signal();
  if (thread_id_ == g_instance.MainThreadId()) {
    // If sending to the main thread, also put a message into the
    // browser's message loop so this mechanism still works if the
    // main thread is sat waiting in the browser's idle loop. It is
    // harmless if the queue clears before this callback occurs - it
    // just runs an empty queue.
    MutexLock lock(&async_mutex_);
    if (!async_in_flight_) {
      // At most only one outstanding NPN_PluginThreadAsyncCall
      // scheduled on the browser's message queue.
      async_in_flight_ = true;
      NPN_PluginThreadAsyncCall(NULL, AsyncCallback, this);
    }
  }
}

void ThreadSpecificQueue::AsyncCallback(void* instance) {
  // Callback from the browser's message queue.
  ThreadSpecificQueue* queue =
      reinterpret_cast<ThreadSpecificQueue *>(instance);
  {
    MutexLock lock(&async_mutex_);
    async_in_flight_ = false;
  }
  queue->DispatchOnce();
}

void ThreadSpecificQueue::PopMessages(std::deque<Message> *messages) {
  // Get and removes all messages from the queue.
  messages->clear();
  {
    MutexLock lock(&event_queue_mutex_);
    event_queue_.swap(*messages);
  }
}

void ThreadSpecificQueue::GetAndDispatchMessages(int exit_type) {
  bool done = false;
  while(!done) {
    event_.Wait();
    // Move existing messages to a local queue.
    std::deque<Message> local_event_queue;
    PopMessages(&local_event_queue);
    // Dispatch the local events
    while (!local_event_queue.empty()) {
      Message msg = local_event_queue.front();
      local_event_queue.pop_front();
      if (msg.message_type == exit_type) {
        done = true;
        break;
      }
      g_instance.HandleThreadMessage(msg.message_type, msg.message_data.get());
    }
  }
}

void ThreadSpecificQueue::RunOnce() {
  event_.Wait();
  DispatchOnce();
}

void ThreadSpecificQueue::DispatchOnce() {
  // Move existing messages to a local queue.
  std::deque<Message> local_event_queue;
  PopMessages(&local_event_queue);
  // Dispatch the local events
  while (!local_event_queue.empty()) {
    Message msg = local_event_queue.front();
    local_event_queue.pop_front();
    g_instance.HandleThreadMessage(msg.message_type, msg.message_data.get());
  }
}

//------------------------------------------------------------------------------
// BrowserThreadMessageQueue

#ifdef USING_CCTESTS

const int kMessageType1 = 100;
const int kMessageType2 = 101;
const int kMessageType3 = 102;

class TestMessage : public MessageData {
 public:
  TestMessage(ThreadId sender, int data) : sender_(sender), data_(data) {}
  ~TestMessage() {}
  ThreadId Sender() {return sender_;}
  int Data() {return data_;}
 private:
  ThreadId sender_;
  int data_;
};

class BackgroundMessageHandler
    : public ThreadMessageQueue::HandlerInterface {
 public:
  explicit BackgroundMessageHandler(ThreadId sender)
      : sender_(sender),
        count_(0) {}

  int Count() {return count_;}
  // ThreadMessageQueue::HandlerInterface override
  virtual void HandleThreadMessage(int message_type,
                                   MessageData *message_data) {
    assert(message_data);
    TestMessage* message = static_cast<TestMessage*>(message_data);
    assert(sender_ == message->Sender());
    // Accumulate the data.
    count_ += message->Data();
    LOG(("Got data %d", count_));
    if (count_ == 600) {
      // Tell the main thread we got all data.
      LOG(("Sending to main thread"));
      message = new TestMessage(
          ThreadMessageQueue::GetInstance()->GetCurrentThreadId(), count_);
      ThreadMessageQueue::GetInstance()->Send(sender_,
                                              kMessageType3,
                                              message);
    }
  }

 private:
  ThreadId sender_;
  int count_;
};

class MainMessageHandler
    : public ThreadMessageQueue::HandlerInterface {
 public:
  MainMessageHandler() : done_(false) { }
  // ThreadMessageQueue::HandlerInterface override
  virtual void HandleThreadMessage(int message_type,
                                   MessageData *message_data) {
    TestMessage* message = static_cast<TestMessage*>(message_data);
    assert (message->Data() == 600);
    // The worker must have received all data. Let's stop it.
    LOG(("Got %d from worker thread", message->Data()));
    AndroidMessageLoop::Stop(message->Sender());
    done_ = true;
  }
  bool IsDone() const { return done_; }

 private:
  bool done_;
};

class TestThread : public Thread {
 public:
  virtual void Run() {
    AndroidThreadMessageQueue* queue = static_cast<AndroidThreadMessageQueue*>(
        ThreadMessageQueue::GetInstance());

    BackgroundMessageHandler handler1(queue->MainThreadId());
    BackgroundMessageHandler handler2(queue->MainThreadId());
    ThreadMessageQueue::GetInstance()->RegisterHandler(
        kMessageType1, &handler1);
    ThreadMessageQueue::GetInstance()->RegisterHandler(
        kMessageType2, &handler2);

    // Run the message loop
    AndroidMessageLoop::Start();
    LOG(("Test thread shutting down."));
  }
};

// Global message handler
MainMessageHandler global_handler;

bool TestThreadMessageQueue(std::string16* error) {
  AndroidThreadMessageQueue* queue = static_cast<AndroidThreadMessageQueue*>(
      ThreadMessageQueue::GetInstance());

  // Init the message queue for the main thread
  queue->InitThreadMessageQueue();
  queue->RegisterHandler(kMessageType3, &global_handler);

  // Start the worker.
  scoped_ptr<TestThread> thread(new TestThread);
  ThreadId worker_thread_id = thread->Start();

  // Send three messages, sleep, send another three.
  ThreadId main_thread_id = queue->MainThreadId();
  TestMessage* message = new TestMessage(main_thread_id, 1);
  queue->Send(worker_thread_id, kMessageType1, message);
  message = new TestMessage(main_thread_id, 2);
  queue->Send(worker_thread_id, kMessageType1, message);
  message = new TestMessage(main_thread_id, 3);
  queue->Send(worker_thread_id, kMessageType1, message);
  sleep(1);
  message = new TestMessage(main_thread_id, 100);
  queue->Send(worker_thread_id, kMessageType2, message);
  message = new TestMessage(main_thread_id, 200);
  queue->Send(worker_thread_id, kMessageType2, message);
  message = new TestMessage(main_thread_id, 300);
  queue->Send(worker_thread_id, kMessageType2, message);

  while (!global_handler.IsDone()) {
    AndroidMessageLoop::RunOnce();
  }
  LOG(("Message from worker received."));
  thread->Join();
  LOG(("Test thread joined successfuly, test completed."));
  return true;
}

#endif  // USING_CCTESTS
#endif  // OS_ANDROID
