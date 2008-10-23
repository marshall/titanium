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

#ifndef GEARS_BASE_COMMON_IPC_MESSAGE_QUEUE_TEST_H__
#define GEARS_BASE_COMMON_IPC_MESSAGE_QUEUE_TEST_H__

#ifdef USING_CCTESTS

#include <list>
#include <set>
#include <vector>

#include "gears/base/common/basictypes.h"
#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/serialization.h"
#include "gears/base/common/string16.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// Helper function to get the hello message used by both master and slave
// process.
const char16 *GetHelloMessage();

// Wait until the specified number of processes have been registered. This is
// only used in testing peer IPC queue.
bool WaitForRegisteredProcesses(int n, int timeout);

// Validate the registered processes are what we expect. This is only used in
// testing peer IPC queue.
bool ValidateRegisteredProcesses(int n, const IpcProcessId *process_ids);

// The message class we send to/from slave processes.
// The sequence number is only embedded for verifcation in the message sent 
// from slave process to master process.
class IpcTestMessage : public Serializable {
 public:
  IpcTestMessage() : sequence_number_(0), bytes_length_(0) {}
  IpcTestMessage(const char16 *str)
      : sequence_number_(-1), string_(str), bytes_length_(0) {}
  IpcTestMessage(const char16 *str, int bytes_length);

  // Serializable methods
  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_IPC_TEST_MESSAGE;
  }
  virtual bool Serialize(Serializer *out) const;
  virtual bool Deserialize(Deserializer *in);

  // Accessors
  int sequence_number() const { return sequence_number_; }
  void set_sequence_number(int sequence_number) {
    sequence_number_ = sequence_number;
  }
  const std::string16 &string() const { return string_; }
  int bytes_length() const { return bytes_length_; }
  uint8 *bytes() const { return bytes_.get(); }
  const IpcMessageQueueCounters &counters() const { return counters_; }

  // Static methods
  static Serializable *New() {
    return new IpcTestMessage;
  }
  static void RegisterAsSerializable() {
    Serializable::RegisterClass(SERIALIZABLE_IPC_TEST_MESSAGE, New);
  }

 private:
  int sequence_number_;
  std::string16 string_;
  int bytes_length_;
  scoped_ptr_malloc<uint8> bytes_;
  IpcMessageQueueCounters counters_;

  DISALLOW_EVIL_CONSTRUCTORS(IpcTestMessage);
};


// The handler we register to receive kIpcQueue_TestMessage's sent
// to us from our slave processes.
class MasterMessageHandler : public IpcMessageQueue::HandlerInterface {
 public:
  MasterMessageHandler()
    : save_messages_(false),
      num_received_messages_(0),
      num_invalid_big_pings_(0),
      num_invalid_sequence_number_counts_(0),
      wait_for_messages_start_time_(0),
      num_messages_waiting_to_receive_(0) {
  }

  // IpcMessageQueue::HandlerInterface methods
  virtual void HandleIpcMessage(IpcProcessId source_process_id,
                                int message_type,
                                const IpcMessageData *message_data);

  bool WaitForMessages(int n);

  bool HasMessagesOrTimedout();

  void ClearSavedMessages();

  void SetSaveMessages(bool save);

  int CountSavedMessages(IpcProcessId source, const char16 *str);

  // Accessors
  int num_invalid_big_pings() const { return num_invalid_big_pings_; }
  int num_invalid_sequence_number_counts() const {
    return num_invalid_sequence_number_counts_;
  }

 private:
  Mutex lock_;
  bool save_messages_;
  int num_received_messages_;
  int num_invalid_big_pings_;
  int num_invalid_sequence_number_counts_;
  std::vector<IpcProcessId> source_ids_;
  std::vector<std::string16> message_strings_;
  IpcMessageQueueCounters last_received_counters_;
  int64 wait_for_messages_start_time_;
  int num_messages_waiting_to_receive_;

  DISALLOW_EVIL_CONSTRUCTORS(MasterMessageHandler);
};

// The handler we register to receive kIpcQueue_TestMessage's sent
// to us from our main process.
class SlaveMessageHandler : public IpcMessageQueue::HandlerInterface {
 public:
  SlaveMessageHandler() : done_(false),
                          chits_received_(0),
                          chats_received_(0),
                          parent_process_id_(0),
                          as_peer_(false) {}

  // IpcMessageQueue::HandlerInterface methods
  virtual void HandleIpcMessage(IpcProcessId source_process_id,
                                int message_type,
                                const IpcMessageData *message_data);

  IpcMessageQueue *GetIpcMessageQueue() const;

  // Accessors
  bool done() const { return done_; }
  void set_done(bool done) { done_ = done; }

  IpcProcessId parent_process_id() const { return parent_process_id_; }
  void set_parent_process_id(IpcProcessId parent_process_id) {
    parent_process_id_ = parent_process_id;
  }

  void set_as_peer(bool as_peer) { as_peer_ = as_peer; }

  // Static methods
  static void TerminateSlave();

 private:
  static void SendIpcMessage(IpcMessageQueue *ipc_message_queue,
                             IpcProcessId source_process_id,
                             IpcTestMessage *message);

  bool done_;
  int chits_received_;
  int chats_received_;
  std::set<IpcProcessId> chit_sources_;
  std::set<IpcProcessId> chat_sources_;
  IpcProcessId parent_process_id_;
  bool as_peer_;

  DISALLOW_EVIL_CONSTRUCTORS(SlaveMessageHandler);
};

// The slave process, invoked by master process.
class SlaveProcess {
 public:
  SlaveProcess() : id_(0), as_peer_(false) {}

  // Starts a slave process.
  bool Start(bool as_peer);

  // Waits until the slave process has been registered.
  bool WaitTillRegistered(int timeout);

  // Waits until the slave process terminates.
  bool WaitForExit(int timeout);

  // Sends a message to all slave processes.
  static void SendAll(IpcMessageQueue *ipc_message_queue,
                      const char16 *message);

  // Clear all tracked processes.
  static void ClearAll();

  // Accessors
  IpcProcessId id() const { return id_; }

 private:
  IpcProcessId id_;
  bool as_peer_;

  static std::list<IpcProcessId> slave_processes_;

  DISALLOW_EVIL_CONSTRUCTORS(SlaveProcess);
};

#endif  // USING_CCTESTS

#endif  // GEARS_BASE_COMMON_IPC_MESSAGE_QUEUE_TEST_H__
