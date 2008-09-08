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
#include <map>
#ifdef WIN32
#include <windows.h> // must manually #include before nsIEventQueueService.h
#endif

#include <gecko_sdk/include/nspr.h>  // for PR_*
#include <gecko_sdk/include/nsCOMPtr.h>
#if BROWSER_FF3
#include <gecko_internal/nsThreadUtils.h>
#else
#include <gecko_internal/nsIEventQueueService.h>
#endif
#include "gears/base/common/message_queue.h"
#include "gears/base/common/thread_locals.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

static const ThreadLocals::Slot kThreadLocalKey = ThreadLocals::Alloc();

struct MessageEvent;
#if BROWSER_FF3
typedef std::map<ThreadId, nsCOMPtr<nsIThread> > ThreadMap;
#elif BROWSER_FF2
typedef std::map<ThreadId, PRThread *> ThreadMap;
#endif

// A concrete implementation that uses Firefox's nsIEventQueue
class FFThreadMessageQueue : public ThreadMessageQueue {
 public:
  FFThreadMessageQueue() : next_id_(0) {}
  virtual bool InitThreadMessageQueue();
  virtual ThreadId GetCurrentThreadId();
  virtual bool Send(ThreadId thread_handle,
                    int message_type,
                    MessageData *message_data);
 private:
#if BROWSER_FF3
  struct MessageEvent : public nsRunnable {
    MessageEvent(int message_type, MessageData *message_data)
        : message_type(message_type), message_data(message_data) {}

    NS_IMETHOD Run() {
      FFThreadMessageQueue::OnReceiveMessageEvent(this);
      return NS_OK;
    }

    int message_type;
    scoped_ptr<MessageData> message_data;
  };
  struct TlsData {
    ThreadId id;

    TlsData(ThreadId thread_id) : id(thread_id) {}
  };
#else
  struct MessageEvent {
    MessageEvent(int message_type, MessageData *message_data)
        : message_type(message_type), message_data(message_data) {}

    PLEvent pl_event;  // must be first in the struct
    int message_type;
    scoped_ptr<MessageData> message_data;
  };
  struct TlsData {
    ThreadId id;
    PRThread *pr_thread;

    TlsData(ThreadId thread_id)
        : id(thread_id), pr_thread(PR_GetCurrentThread())
    {}
  };
#endif

  static void *OnReceiveMessageEvent(MessageEvent *event);
  static void OnDestroyMessageEvent(MessageEvent *event);

  static void ThreadEndHook(void* value);
  void InitThreadEndHook(ThreadId thread_id);

  ThreadMap threads_;
  Mutex threads_mutex_;
  ThreadId next_id_;
};

// Create an instance on the heap, note that we can't use a static
// global, because during application shutdown that may be destructed while 
// threads are still running and referencing it.
static FFThreadMessageQueue *g_instance = new FFThreadMessageQueue();

// static
ThreadMessageQueue *ThreadMessageQueue::GetInstance() {
  return g_instance;
}

// static
void FFThreadMessageQueue::ThreadEndHook(void* value) {
  TlsData *data = reinterpret_cast<TlsData*>(value);
  if (data) {
#if BROWSER_FF2
    // We revoke all events here so that we can reuse this thread in
    // Firefox 2
    nsresult nr;
    nsCOMPtr<nsIEventQueueService> event_queue_service =
        do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &nr);
    if (NS_SUCCEEDED(nr)) {
      nsCOMPtr<nsIEventQueue> event_queue;
      nr = event_queue_service->GetThreadEventQueue(
          data->pr_thread,
          getter_AddRefs(event_queue));
      if (NS_SUCCEEDED(nr)) {
        event_queue->RevokeEvents(nsnull);
      }
    }
#endif
    MutexLock lock(&g_instance->threads_mutex_);
    g_instance->threads_.erase(data->id);
    delete data;
  }
}

void FFThreadMessageQueue::InitThreadEndHook(ThreadId thread_id) {
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
      TlsData *data = new TlsData(thread_id);
      ThreadLocals::SetValue(kThreadLocalKey, data, &ThreadEndHook);
    }
  }
}

bool FFThreadMessageQueue::InitThreadMessageQueue() {
  if (ThreadLocals::HasValue(kThreadLocalKey)) {
    return true;
  }


#if BROWSER_FF3
  nsCOMPtr<nsIThread> thread;
  if (NS_FAILED(NS_GetCurrentThread(getter_AddRefs(thread)))) {
    return false;
  }
#elif BROWSER_FF2
  PRThread *thread = PR_GetCurrentThread();
#endif

  MutexLock lock(&threads_mutex_);
  ThreadId thread_id = ++next_id_;
  threads_[thread_id] = thread;
  InitThreadEndHook(thread_id);

  return true;
}

ThreadId FFThreadMessageQueue::GetCurrentThreadId() {
  TlsData *data =
      reinterpret_cast<TlsData*>(ThreadLocals::GetValue(kThreadLocalKey));
  if (data) {
    return data->id;
  } else {
    assert(false);
    return -1;
  }
}

// static
void *FFThreadMessageQueue::OnReceiveMessageEvent(MessageEvent *event) {
  RegisteredHandler handler;
  if (g_instance->GetRegisteredHandler(event->message_type, &handler)) {
    handler.Invoke(event->message_type, event->message_data.get());
  }
  return NULL;
}

// static
void FFThreadMessageQueue::OnDestroyMessageEvent(MessageEvent *event) {
  delete event;
}

bool FFThreadMessageQueue::Send(ThreadId thread,
                                int message_type,
                                MessageData *message_data) {
  MutexLock lock(&threads_mutex_);
  ThreadMap::iterator dest_thread = threads_.find(thread);
  if (dest_thread == threads_.end()) {
    delete message_data;
    return false;
  }

#if BROWSER_FF3
  nsCOMPtr<nsIRunnable> event = new MessageEvent(message_type, message_data);
  dest_thread->second->Dispatch(event, NS_DISPATCH_NORMAL);
#else
  nsresult nr;
  nsCOMPtr<nsIEventQueueService> event_queue_service =
      do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &nr);
  if (NS_FAILED(nr)) {
    delete message_data;
    return false;
  }

  nsCOMPtr<nsIEventQueue> event_queue;
  nr = event_queue_service->GetThreadEventQueue(dest_thread->second,
                                                getter_AddRefs(event_queue));
  if (NS_FAILED(nr)) {
    delete message_data;
    return false;
  }

  MessageEvent *event = new MessageEvent(message_type, message_data);
  if (NS_FAILED(event_queue->InitEvent(
          &event->pl_event, nsnull,
          reinterpret_cast<PLHandleEventProc>(OnReceiveMessageEvent),
          reinterpret_cast<PLDestroyEventProc>(OnDestroyMessageEvent)))) {
    delete event;
    return false;
  }
  if (NS_FAILED(event_queue->PostEvent(&event->pl_event))) {
    // Cleaning up the event at this point requires access to private
    // functions, so just clear the message data to leak less.
    event->message_data.reset(NULL);
    return false;
  }
#endif
  return true;
}
