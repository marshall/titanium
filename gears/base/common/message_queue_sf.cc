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

#include <assert.h>
#include <deque>
#include <map>
#include <pthread.h>

#include "gears/base/common/message_queue.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/safari/scoped_cf.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

static const ThreadLocals::Slot kThreadLocalKey = ThreadLocals::Alloc();

// A concrete implementation for Safari.
class SFThreadMessageQueue : public ThreadMessageQueue {
 public:
  virtual bool InitThreadMessageQueue();
  virtual ThreadId GetCurrentThreadId();
  virtual bool Send(ThreadId thread_handle,
                    int message_type,
                    MessageData *message_data);
  
  // System callback for receiving a message on a Mach port.
  static void OnThreadMessage(CFMachPortRef port, void *msg, CFIndex size,
                              void *info);
                              
  // Install OS hook to call ThreadEndHook() on thread exit.
  void InitThreadEndHook();
  static void ThreadEndHook(void* value);
                          
 private:
  void HandleThreadMessage(int message_type, MessageData *message_data);
  
  struct MachMessage {
    mach_msg_header_t header;
  };
};

// Holder for a message type/data pair.
struct MessageEvent {
  MessageEvent() {}
  MessageEvent(int message_type, MessageData *message_data)
      : message_type(message_type), message_data(message_data) {}
  int message_type;
  linked_ptr<MessageData> message_data;
};

// Owns the Mach port reference and message queue for a specific
// thread. 
struct ThreadMessagePort {
  CFMachPortRef port;
  std::deque<MessageEvent> events;
  
  ~ThreadMessagePort() {
    CFRelease(port);
  }
};

// control access to message_ports_.
static Mutex message_ports_mutex_;

// Mapping between Thread ids and the Mach message ports/message queues we use
// to send them messages.
typedef std::map<ThreadId, linked_ptr<ThreadMessagePort> > 
            ThreadIDToMessagePortMap;
static  ThreadIDToMessagePortMap *message_ports_;

// Create an instance on the heap, note that we can't use a static
// global, because during application shutdown that may be destructed while 
// threads are still running and referencing it.
static SFThreadMessageQueue *g_instance = new SFThreadMessageQueue;

// static
ThreadMessageQueue *ThreadMessageQueue::GetInstance() {
  return g_instance;
}

bool SFThreadMessageQueue::InitThreadMessageQueue() {
  MutexLock lock(&message_ports_mutex_);
  if (!message_ports_) {
    message_ports_ = new ThreadIDToMessagePortMap;
  }
  
  ThreadId thread_id = GetCurrentThreadId();
  if (message_ports_->find(thread_id) == message_ports_->end()) {
    // Create a mach port to communicate between the threads
    CFMachPortContext context;
    context.version = 0;
    context.info = this;
    context.retain = NULL;
    context.release = NULL;
    context.copyDescription = NULL;
    Boolean should_free = false;    // Don't free context.info on dealloc
    CFMachPortRef machport_ref = CFMachPortCreate(kCFAllocatorDefault, 
                                                  OnThreadMessage, 
                                                  &context, &should_free);
    scoped_CFMachPort mach_port(machport_ref);

    if (!mach_port.get() || should_free) {
      LOG(("Couldn't make mach port\n"));
      return false;
    }

    // Create a RL source and add it to the current thread we're executing on.
    scoped_CFRunLoopSource 
      run_loop_source(CFMachPortCreateRunLoopSource(kCFAllocatorDefault, 
                                                    mach_port.get(), 0));
    CFRunLoopAddSource(CFRunLoopGetCurrent(), run_loop_source.get(),
                       kCFRunLoopCommonModes);
        
    // Add cleanup hook.
    InitThreadEndHook();
    
    linked_ptr<ThreadMessagePort> thread_data(new ThreadMessagePort);
    thread_data->port = mach_port.release();
    
    // Update our thread_id -> mach_port mapping.
    (*message_ports_)[thread_id] = thread_data;

  }
  return true;
}

ThreadId SFThreadMessageQueue::GetCurrentThreadId() {
  return pthread_self();
}

void SFThreadMessageQueue::HandleThreadMessage(int message_type,
                                               MessageData *message_data) {
  RegisteredHandler handler;
  if (GetRegisteredHandler(message_type, &handler)) {
    handler.Invoke(message_type, message_data);
  }
}

bool SFThreadMessageQueue::Send(ThreadId thread,
                                int message_type,
                                MessageData *message_data) {
  MutexLock lock(&message_ports_mutex_);
  scoped_ptr<MessageData> scoped_message_data(message_data);
  
  // Retrieve mach port associated with thread_id.
  if (!message_ports_) {
    return false;
  }
  ThreadIDToMessagePortMap::iterator it;
  it = message_ports_->find(thread);
  if (it == message_ports_->end()) {
    return false;
  }
  ThreadMessagePort *message_port = it->second.get();
  const CFMachPortRef &mach_port = message_port->port;
  
  // Now post a message to the Mach port.
  MachMessage msg = {0};
  
  // Fill in message data.
  msg.header.msgh_size = sizeof(MachMessage);
  msg.header.msgh_remote_port = CFMachPortGetPort(mach_port);
  msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
                                        MACH_MSG_TYPE_MAKE_SEND_ONCE);

  message_port->events.push_back(MessageEvent(message_type, 
                                scoped_message_data.release()));
  
  mach_msg_return_t ret = mach_msg(&(msg.header),
                                   MACH_SEND_MSG | MACH_SEND_TIMEOUT,
                                   msg.header.msgh_size, 0, 0, 
                                   MACH_MSG_TIMEOUT_NONE, 
                                   MACH_PORT_NULL);
                  
  // If the port queue is full (MACH_SEND_TIMED_OUT) we dont' consider that
  // an error - see comment in OnThreadMessage() for details.
  if (ret != MACH_MSG_SUCCESS && ret != MACH_SEND_TIMED_OUT) {
   message_port->events.pop_back();
   return false;
  }
  return true;
}

// Invoked by Mach on the thread the port was created on.
void SFThreadMessageQueue::OnThreadMessage(CFMachPortRef port, 
                                           void *msg, 
                                           CFIndex size,
                                           void *info) {
  // If we wanted the originating MachMessage object, we could do:
  // MachMessage *thread_msg = static_cast<MachMessage *>(msg);
  
  SFThreadMessageQueue *message_queue = 
                            static_cast<SFThreadMessageQueue *>(info);

  assert(message_queue == g_instance);
  
  // Mach ports have a finite queue length, to get aorund this we maintain our
  // own message queue, and upon receiving a notification, process all
  // messages in the queue, this way even if the mach queue is full, we can
  // continue to "send" messages.
  std::deque<MessageEvent> local_events;
                       
  // Swap contents of message queue into local variable.
  {
    MutexLock lock(&message_ports_mutex_);
    
    // Retrieve data holder for this thread.
    ThreadIDToMessagePortMap::iterator it;
    it = message_ports_->find(g_instance->GetCurrentThreadId());
    if (it == message_ports_->end()) {
      return;
    }
    ThreadMessagePort *message_port = it->second.get();
    
    message_port->events.swap(local_events);
  }

  // Dispatch messages.
  while (!local_events.empty()) {
    MessageEvent event = local_events.front();
    local_events.pop_front();
    
    message_queue->HandleThreadMessage(event.message_type, 
                                       event.message_data.get());
  }
}

void SFThreadMessageQueue::InitThreadEndHook() {
  // We use a ThreadLocal to get called when an OS thread terminates
  // and use that opportunity to remove all observers that remain
  // registered on that thread.
  //
  // We store the thread id in the ThreadLocal variable because on some
  // OSes (linux), the destructor proc is called from a different thread,
  // and on others (windows), the destructor proc is called from the
  // terminating thread. 
  if (!ThreadLocals::HasValue(kThreadLocalKey)) {
    ThreadId *id = new ThreadId(GetCurrentThreadId()); 
    ThreadLocals::SetValue(kThreadLocalKey, id, &ThreadEndHook);
  }
}

// static 
void SFThreadMessageQueue::ThreadEndHook(void* value) {
  ThreadId *id = reinterpret_cast<ThreadId*>(value);
  if (id) {
    MutexLock lock(&message_ports_mutex_); 
    message_ports_->erase(*id); 
    delete id;
  }
}
