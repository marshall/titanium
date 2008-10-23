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
#include <vector>

#include "gears/base/common/message_queue.h"


// The subset of the actual ThreadMessageQueue implementation
// common to all platforms, handler registration.

void ThreadMessageQueue::RegisterHandler(int message_type,
                                         HandlerInterface *instance) {
  MutexLock lock(&handler_mutex_);
  handlers_[message_type] = RegisteredHandler(instance);
}

bool ThreadMessageQueue::GetRegisteredHandler(int message_type,
                                              RegisteredHandler *handler) {
  MutexLock lock(&handler_mutex_);
  std::map<int, RegisteredHandler>::iterator handler_loc;
  handler_loc = handlers_.find(message_type);
  if (handler_loc == handlers_.end())
    return false;
  *handler = handler_loc->second;
  return true;
}


#ifdef USING_CCTESTS
// The MockThreadMessageQueue implementation

bool MockThreadMessageQueue::Send(ThreadId thread_id,
                                  int message_type,
                                  MessageData *message_data) {
  if (initialized_threads_.find(thread_id) == 
      initialized_threads_.end()) {
    delete message_data;
    return false;
  }
  pending_message_thread_ids_.push_back(thread_id);
  pending_message_types_.push_back(message_type);
  pending_messages_.push_back(linked_ptr<MessageData>(message_data));
  return true;
}

void MockThreadMessageQueue::DeliverMockMessages() {
  size_t count = pending_message_thread_ids_.size();
  assert(count == pending_messages_.size());
  for (size_t i = 0; i < count; ++i) {
    ThreadId thread_id = pending_message_thread_ids_[i];
    int message_type = pending_message_types_[i];
    MessageData *message_data = pending_messages_[i].get();
    SetMockCurrentThreadId(thread_id);
    RegisteredHandler handler;
    if (GetRegisteredHandler(message_type, &handler)) {
      handler.Invoke(message_type, message_data);
    }
  }
  pending_message_thread_ids_.clear();
  pending_message_types_.clear();
  pending_messages_.clear();
}

#endif  // USING_CCTESTS
