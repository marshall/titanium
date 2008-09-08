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
#include <deque>
#include <map>
#include <string>
#include <windows.h>

#include "gears/base/common/atomic_ops.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/thread_locals.h"
#include "third_party/linked_ptr/linked_ptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"


static const ThreadLocals::Slot kTlsKey = ThreadLocals::Alloc();
class ThreadMessageWindow;

// A concrete implementation that uses HWNDs and PostMessage. There is a
// single instance of this class. When InitThreadMessageQueue is called,
// a ThreadMessageWindow, containing an HWND, is created to receive message
// posted to the calling thread of control. A ThreadLocal is set to detect
// thread termination and destroy the window. A reference to the window is
// also placed into a global map for use by the Send method to lookup the
// destination window by thread id.
class IEThreadMessageQueue : public ThreadMessageQueue {
 public:
  IEThreadMessageQueue() : thread_map_(NULL) {}

  virtual bool InitThreadMessageQueue();
  virtual ThreadId GetCurrentThreadId();
  virtual bool Send(ThreadId thread_handle,
                    int message_type,
                    MessageData *message);
 private:
  friend class ThreadMessageWindow;
  void HandleThreadMessage(int message_type, MessageData *message_data);

  typedef std::map<ThreadId, ThreadMessageWindow*> ThreadMap;
  Mutex thread_map_mutex_;
  ThreadMap *thread_map_;

  struct TlsData {
    TlsData(ThreadId id, ThreadMessageWindow *window)
        : thread_id(id), message_window(window) {}
    ThreadId thread_id;
    scoped_ptr<ThreadMessageWindow> message_window;
  };

  static void ThreadEndHook(void* value);
};

// Owns the window that has the message queue for a thread, and manages the
// transfer of messages across thread boundries.
class ThreadMessageWindow : public CWindowImpl<ThreadMessageWindow> {
 public:
  static const DWORD WM_THREAD_MESSAGE = WM_USER + 1;
  BEGIN_MSG_MAP(ThreadMessageWindow)
    MESSAGE_HANDLER(WM_THREAD_MESSAGE , OnThreadMessage)
  END_MSG_MAP()

  ThreadMessageWindow() {
    if (!Create(kMessageOnlyWindowParent,
                NULL, NULL,
                kMessageOnlyWindowStyle)) {
      assert(false);
    }
  }

  ~ThreadMessageWindow() {
    if (IsWindow()) {
      DestroyWindow();
    }
  }

  LRESULT OnThreadMessage(UINT msg, WPARAM unused_wparam,
                          LPARAM unused_lparam, BOOL &handled);
  void PostThreadMessage(int message_tyoe, MessageData *message_data);
 private:
  struct MessageEvent {
    MessageEvent() {}
    MessageEvent(int message_type, MessageData *message_data)
        : message_type(message_type), message_data(message_data) {}
    int message_type;
    linked_ptr<MessageData> message_data;
  };

  Mutex events_mutex_;
  std::deque<MessageEvent> events_;
};


static IEThreadMessageQueue g_instance;

// static
ThreadMessageQueue *ThreadMessageQueue::GetInstance() {
  return &g_instance;
}


bool IEThreadMessageQueue::InitThreadMessageQueue() {
  if (ThreadLocals::HasValue(kTlsKey)) {
    return true;  // already initialized
  }

  // Note: We have to be careful about dead locks here and in
  // ThreadEndHook below. ThreadEndHook is called via DllMain which
  // is called by the loader with the loader lock being held. So
  // we have to avoid calling anything that may acquire the loader
  // lock while we have our mutex locked. This is why we create
  // the window prior to locking our mutex.
  TlsData *data = new TlsData(GetCurrentThreadId(),
                              new ThreadMessageWindow);
  ThreadLocals::SetValue(kTlsKey, data, &ThreadEndHook);

  MutexLock lock(&thread_map_mutex_);
  if (!thread_map_) {
    thread_map_ = new ThreadMap;
  }
  (*thread_map_)[data->thread_id] = data->message_window.get();
  return true;
}


// static
void IEThreadMessageQueue::ThreadEndHook(void* value) {
  TlsData *data = reinterpret_cast<TlsData*>(value);
  if (data) {
    // Scoped to release the lock prior to window deletion
    {
      MutexLock lock(&g_instance.thread_map_mutex_);
      assert(g_instance.thread_map_);
      g_instance.thread_map_->erase(data->thread_id);
    }

    delete data;
  }
}


ThreadId IEThreadMessageQueue::GetCurrentThreadId() {
  return ::GetCurrentThreadId();
}

bool IEThreadMessageQueue::Send(ThreadId thread_id,
                                int message_type,
                                MessageData *message_data) {
  scoped_ptr<MessageData> scoped_message_data(message_data);
  MutexLock lock(&thread_map_mutex_);
  if (!thread_map_) {
    return false;
  }
  ThreadMap::iterator found = thread_map_->find(thread_id);
  if (found == thread_map_->end()) {
    return false;
  }
  found->second->PostThreadMessage(message_type,
                                   scoped_message_data.release());
  return true;
}

void IEThreadMessageQueue::HandleThreadMessage(int message_type,
                                               MessageData *message_data) {
  RegisteredHandler handler;
  if (GetRegisteredHandler(message_type, &handler)) {
    handler.Invoke(message_type, message_data);
  }
}

LRESULT ThreadMessageWindow::OnThreadMessage(UINT msg, WPARAM unused_wparam,
                                             LPARAM unused_lparam,
                                             BOOL &handled) {
  assert(msg == WM_THREAD_MESSAGE);
                       
  // Swap contents of events queue into local variable.
  std::deque<MessageEvent> local_events;
  {
    MutexLock lock(&events_mutex_);
    events_.swap(local_events);
  }
  assert(!local_events.empty());    

  // Dispatch all pending messages.
  while (!local_events.empty()) {
    MessageEvent &event = local_events.front();
    g_instance.HandleThreadMessage(event.message_type,
                                   event.message_data.get());
    local_events.pop_front();
  }
  handled = TRUE;
  return 0;
}

void ThreadMessageWindow::PostThreadMessage(int message_type,
                                            MessageData *message_data) {
  MutexLock lock(&events_mutex_);
  events_.push_back(MessageEvent(message_type, message_data));
  if (events_.size() == 1) {
    PostMessage(WM_THREAD_MESSAGE, 0, 0);
  }
}
