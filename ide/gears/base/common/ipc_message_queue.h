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

#ifndef GEARS_BASE_COMMON_IPC_MESSAGE_QUEUE_H__
#define GEARS_BASE_COMMON_IPC_MESSAGE_QUEUE_H__

#include <map>
#include "gears/base/common/mutex.h"
#include "gears/base/common/serialization.h"

typedef Serializable IpcMessageData;

// Ideally should come from a process abstraction layer of some sort
typedef uint32 IpcProcessId;

// Ipc message type identifiers are listed here to avoid collisions
enum IpcMessageTypes {
  kIpcQueue_TestMessage = 1,
  kMessageService_IpcNotify,

  // For desktop notification. They can not be changed for cross-version
  // compatibility.
  kDesktop_AddNotification = 1000,
  kDesktop_RemoveNotification,
  kDesktop_RestartNotifierImmediately 
};

// A facility for sending and receiving messages asynchronously 
// across process boundaries. Each process has a single ipc message
// queue into which other process can place messages.
class IpcMessageQueue {
 public:
  // Returns a pointer to the singleton for doing IPC among peers. May return 
  // NULL if this ipc facility is not required for a particular build target.
  static IpcMessageQueue *GetPeerQueue();

  // Returns a pointer to the singleton for doing IPC in the system. May return 
  // NULL if this ipc facility is not required for a particular build target.
  static IpcMessageQueue *GetSystemQueue();

  // Message handlers implement this interface. All messages of a
  // given message_type will be directed the registered handler
  // for that message_type.
  class HandlerInterface {
   public:
    virtual void HandleIpcMessage(IpcProcessId source_process_id,
                                  int message_type,
                                  const IpcMessageData *message_data) = 0;
  };

  // Registers an instance as a handler callback. There is
  // no means to unregister a handler, so this should only be used
  // for singleton instances of message handler class.
  void RegisterHandler(int message_type,
                       HandlerInterface *handler_instance) {
    MutexLock lock(&handler_mutex_);
    handlers_[message_type] = handler_instance;
  }

  // Returns the id of the currently executing process.
  virtual IpcProcessId GetCurrentIpcProcessId() = 0;

  // The callback to inform about the send result.
  typedef void (*SendCompletionCallback)(bool is_successful,
                                         IpcProcessId dest_process_id,
                                         int message_type,
                                         const IpcMessageData *message_data,
                                         void *param);

  // Sends a message to the indicated process.
  void Send(IpcProcessId dest_process_id,
            int message_type,
            IpcMessageData *message_data) {
    SendWithCompletion(dest_process_id, message_type, message_data, NULL, NULL);
  }

  virtual void SendWithCompletion(IpcProcessId dest_process_id,
                                  int message_type,
                                  IpcMessageData *message_data,
                                  SendCompletionCallback callback,
                                  void *callback_param) = 0;

  // Sends a messages to all processes with an IpcMessageQueue with the
  // possible exception of the current process.
  virtual void SendToAll(int message_type,
                         IpcMessageData *message_data,
                         bool including_current_process) = 0;

 protected:
  IpcMessageQueue() {}
  virtual ~IpcMessageQueue() {}

  void CallRegisteredHandler(IpcProcessId source_process_id,
                             int message_type,
                             const IpcMessageData *message_data) {
    HandlerInterface *handler = NULL;
    if (GetRegisteredHandler(message_type, &handler) && handler) {
      handler->HandleIpcMessage(source_process_id, message_type, message_data);
    }
  }

  bool GetRegisteredHandler(int message_type, HandlerInterface **handler) {
    MutexLock lock(&handler_mutex_);
    std::map<int, HandlerInterface*>::iterator handler_loc;
    handler_loc = handlers_.find(message_type);
    if (handler_loc == handlers_.end())
      return false;
    *handler = handler_loc->second;
    return true;
  }

  Mutex handler_mutex_;
  std::map<int, HandlerInterface*> handlers_;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(IpcMessageQueue);
};

#ifdef USING_CCTESTS
// For testing
struct IpcMessageQueueCounters {
  int send_to_all;
  int send_to_one;
  int queued_outbound;
  int sent_outbound;
  int read_inbound;
  int dispatched_inbound;
};
#endif


#endif  // GEARS_BASE_COMMON_IPC_MESSAGE_QUEUE_H__
