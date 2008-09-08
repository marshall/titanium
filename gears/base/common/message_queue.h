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

#ifndef GEARS_BASE_COMMON_MESSAGE_QUEUE_H__
#define GEARS_BASE_COMMON_MESSAGE_QUEUE_H__

#include <map>
#include <set>
#include <vector>
#include "gears/base/common/deletable.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/thread.h"
#include "third_party/linked_ptr/linked_ptr.h"

// Base class for data that can be sent using ThreadMessageQueue.
// Contains only a virtual dtor, to invoke the destructor of derived classes.
typedef Deletable MessageData;

// Message type identifiers are listed here to avoid collisions
enum ThreadMessageTypes {
  kMessageService_Notify = 1,
  kAsyncRouter_Call,
  kAndroidLoop_Exit,
};

// A facility for sending and receiving messages asynchronously 
// across thread boundaries. 
class ThreadMessageQueue {
 public:
  // Returns a pointer to the singleton
  static ThreadMessageQueue *GetInstance();

  // Message handlers implement this interface. All messages of a
  // given message_type will be directed the registered handler
  // for that message_type.
  class HandlerInterface {
   public:
    virtual void HandleThreadMessage(int message_type,
                                     MessageData *message_data) = 0;
  };

  // Registers an instance as a handler callback. There is
  // no means to unregister a handler, so this should only be used
  // for singleton instances if message handler class.
  void RegisterHandler(int message_type,
                       HandlerInterface *handler_instance);

  // Initializes the message queue for the current thread. If the
  // queue is already initialized, this is a no-op. Returns true
  // if the queue is successfully initialized.
  virtual bool InitThreadMessageQueue() = 0;

  // Returns the thread id of the currently executing thread.
  virtual ThreadId GetCurrentThreadId() = 0;

  // Posts a message to the indicated thread.  Returns true if the
  // message is successfully queued for delivery. In all cases,
  // ownership of the message data is transferred to the message queue.
  // Upon return from this method, callers should no longer touch the
  // message data.
  virtual bool Send(ThreadId thread_id,
                    int message_type,
                    MessageData *message_data) = 0;
 protected:
  ThreadMessageQueue() {}
  virtual ~ThreadMessageQueue() {}

  struct RegisteredHandler {
    RegisteredHandler() : instance(NULL) {}
    RegisteredHandler(HandlerInterface *inst) : instance(inst) {}
    HandlerInterface *instance;

    void Invoke(int message_type, MessageData *message_data) {
      if (instance)
        instance->HandleThreadMessage(message_type, message_data);
    }
  };

  bool GetRegisteredHandler(int message_type, RegisteredHandler *handler);

  // Protects handlers_ during inserts and finds.
  Mutex handler_mutex_;
  std::map<int, RegisteredHandler> handlers_;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(ThreadMessageQueue);
};

#ifdef OS_ANDROID
// A facility for running a message loop for the calling thread.
class AndroidMessageLoop {
 public:
  // Starts the message loop for the calling thread.
  static void Start();
  // Run the message loop once. If the queue is empty when called,
  // this blocks until at least one message is received. This
  // processes all messages in the queue and then returns.
  // TODO(jripley): fix the following scenario:
  // The main thread is waiting in the main browser loop.
  // The main thread queue's event is signaled.
  // A message is dispatched on the main thread via
  // AsyncCallback, so the main thread queue's event is
  // left in signaled state.
  // Later RunOnce is called on the main thread.
  // RunOnce will find the event signaled and will
  // exit without having processed any messages.
  // Possible fix: Add Event::Clear() and call it
  // from AsyncCallback.
  static void RunOnce();
  // Stops the message loop for the given thread. The target thread
  // must be looping in Start().
  static void Stop(ThreadId thread_id);

 private:
  AndroidMessageLoop();
  DISALLOW_EVIL_CONSTRUCTORS(AndroidMessageLoop);
};
#endif

#ifdef USING_CCTESTS
// A mock implementation of the ThreadMessageQueue that
// can be used for testing.
class MockThreadMessageQueue : public ThreadMessageQueue {
 public:
  MockThreadMessageQueue() : current_thread_id_(0) {}

  virtual bool InitThreadMessageQueue() {
    initialized_threads_.insert(current_thread_id_);
    return true;
  }

  virtual ThreadId GetCurrentThreadId() {
    return current_thread_id_;
  }

  virtual bool Send(ThreadId thread_id,
                    int message_type,
                    MessageData *message_data);

  void SetMockCurrentThreadId(ThreadId thread_id) {
    current_thread_id_ = thread_id;
  }

  // Delivers messages that have been sent via the Send method
  // since the last time DeliverMockMessages was called.
  void DeliverMockMessages();

 private:
  ThreadId current_thread_id_;
  std::set<ThreadId> initialized_threads_;
  std::vector<ThreadId> pending_message_thread_ids_;
  std::vector<int> pending_message_types_;
  std::vector<linked_ptr<MessageData> > pending_messages_;

  DISALLOW_EVIL_CONSTRUCTORS(MockThreadMessageQueue);
};
#endif  // USING_CCTESTS

#endif  // GEARS_BASE_COMMON_MESSAGE_QUEUE_H__
