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

// TODO(jianli): add sequence number to the message in order to check if the
// messages are delivered in the sendering order.

#if (defined(WIN32) && !defined(WINCE)) || defined(LINUX) || defined(OS_MACOSX)

#ifdef USING_CCTESTS

#include "gears/base/common/ipc_message_queue_test.h"

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOCATION __FILE__ ", line " TOSTRING(__LINE__)
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("failed at " LOCATION)); \
    assert(error); \
    if (!error->empty()) *error += STRING16(L", "); \
    *error += STRING16(L"failed at "); \
    std::string16 location; \
    UTF8ToString16(LOCATION, &location); \
    *error += location; \
    SlaveProcess::SendAll(ipc_message_queue, kQuit); \
    return false; \
  } \
}

//-----------------------------------------------------------------------------
// Functions defined in ipc_message_queue_***.cc to facilitate testing
//-----------------------------------------------------------------------------

void TestingIpcMessageQueue_GetCounters(IpcMessageQueueCounters *counters,
                                        bool reset);
#ifdef WIN32
void TestingIpcMessageQueueWin32_DieWhileHoldingWriteLock(
        IpcMessageQueue *ipc_message_queue, IpcProcessId id);
void TestingIpcMessageQueueWin32_DieWhileHoldingRegistryLock(
        IpcMessageQueue *ipc_message_queue);
void TestingIpcMessageQueueWin32_SleepWhileHoldingRegistryLock(
        IpcMessageQueue *ipc_message_queue);
#endif  // WIN32

//-----------------------------------------------------------------------------
// Constants, functions, and classes used by test code in both the master
// and slave processes
//-----------------------------------------------------------------------------

static const int kIpcTestWaitTimeoutMs = 30000;       // 30s
static const int kIpcQuitStraglersTimeoutMs = 2000;   // 2s

#undef IPC_STRESS

#ifdef IPC_STRESS
static const int kManyPings = 10000;
static const int kManyBigPings = 1000;
static const int kManySlavesCount = 30;  // max supported is 31
#else
static const int kManyPings = 1000;
static const int kManyBigPings = 100;
static const int kManySlavesCount = 10;
#endif  // IPC_STRESS

// we want this larger than the capacity of the packet
// (~8K for WIN32, ~2K for Linux, 450 for Mac)
#ifdef WIN32
static const int kBigPingLength = 18000;
#elif defined(LINUX) && !defined(OS_MACOSX)
static const int kBigPingLength = 5000;
#elif defined(OS_MACOSX)
static const int kBigPingLength = 1000;
#endif  // WIN32

static const char16 *kHello = STRING16(L"hello");
static const char16 *kPing = STRING16(L"ping");
static const char16 *kQuit = STRING16(L"quit");
static const char16 *kBigPing = STRING16(L"bigPing");
static const char16 *kSendManyPings = STRING16(L"sendManyPings");
static const char16 *kSendManyBigPings = STRING16(L"sendManyBigPings");
static const char16 *kError = STRING16(L"error");
static const char16 *kDoChitChat = STRING16(L"doChitChat");
static const char16 *kChit = STRING16(L"chit");
static const char16 *kChat = STRING16(L"chat");
static const char16 *kDidChitChat = STRING16(L"didChitChat");
static const char16 *kDieWhileHoldingWriteLock = STRING16(L"dieWithWriteLock");
static const char16 *kDieWhileHoldingRegistryLock =
                         STRING16(L"dieWithRegistryLock");
static const char16* kQuitWhileHoldingRegistryLock =
                         STRING16(L"quitWithRegistryLock");


const char16 *GetHelloMessage() {
  return kHello;
}

//-----------------------------------------------------------------------------
// IPC Test Message
//-----------------------------------------------------------------------------

// Allocates block of memory using malloc and initializes the memory
// with an easy to verify pattern.
static uint8 *MallocTestData(int length) {
  uint8 *data = static_cast<uint8*>(malloc(length));
  for(int i = 0; i < length; ++i) {
    data[i] = static_cast<uint8>(i % 256);
  }
  return data;
}

static bool VerifyTestData(uint8 *data, int length) {
  if (!data) return false;
  for(int i = 0; i < length; ++i) {
    if (data[i] != static_cast<uint8>(i % 256))
      return false;
  }
  return true;
}

static bool VerifySequenceNumber(IpcProcessId source_process_id,
                                 int sequence_number) {
  static std::map<IpcProcessId, int> last_sequence_number_map;

  bool ok = false;
  if (sequence_number == -1) {
    // Sequence number is not provided. Skip the verification.
    return true;
  } else if (sequence_number == 0) {
    // If the sequence number is 0, it indicates the first message from the
    // source process. We do not check if there is any other message arrived
    // before this one since if this happens, the next message will have out-of-
    // ordered sequence number. This helps us to work around the problem that
    // the process ID could be reused.
    ok = true;
  } else {
    std::map<IpcProcessId, int>::const_iterator iter =
        last_sequence_number_map.find(source_process_id);
    if (iter != last_sequence_number_map.end() &&
        iter->second + 1 == sequence_number) {
      ok = true;
    }
  }

  last_sequence_number_map[source_process_id] = sequence_number;
  return ok;
}

IpcTestMessage::IpcTestMessage(const char16 *str, int bytes_length)
    : string_(str), bytes_length_(bytes_length),
      bytes_(MallocTestData(bytes_length)) {
}

bool IpcTestMessage::Serialize(Serializer *out) const {
  out->WriteInt(sequence_number_);

  out->WriteString(string_.c_str());

  IpcMessageQueueCounters counters;
  TestingIpcMessageQueue_GetCounters(&counters, false);
  out->WriteBytes(&counters, sizeof(counters));

  out->WriteInt(bytes_length_);
  if (bytes_length_) {
    out->WriteBytes(bytes_.get(), bytes_length_);
  }
  return true;
}

bool IpcTestMessage::Deserialize(Deserializer *in) {
  if (!in->ReadInt(&sequence_number_) ||
      !in->ReadString(&string_) ||
      !in->ReadBytes(&counters_, sizeof(counters_)) ||
      !in->ReadInt(&bytes_length_)) {
    return false;
  }
  if (bytes_length_) {
    bytes_.reset(static_cast<uint8*>(malloc(bytes_length_)));
    return in->ReadBytes(bytes_.get(), bytes_length_);
  } else {
    bytes_.reset(NULL);
    return true;
  }
}


//-----------------------------------------------------------------------------
// Message handler for master process
//-----------------------------------------------------------------------------

void MasterMessageHandler::HandleIpcMessage(
                                IpcProcessId source_process_id,
                                int message_type,
                                const IpcMessageData *message_data) {
  MutexLock locker(&lock_);
  if (save_messages_) {
    const IpcTestMessage *test_message =
              static_cast<const IpcTestMessage*>(message_data);
    ++num_received_messages_;

    std::string narrow_string;
    String16ToUTF8(test_message->string().c_str(), &narrow_string);
    LOG(("master received %s\n", narrow_string.c_str()));

    source_ids_.push_back(source_process_id);
    message_strings_.push_back(test_message->string());
    last_received_counters_ = test_message->counters();

    if (!VerifySequenceNumber(source_process_id,
                              test_message->sequence_number())) {
      ++num_invalid_sequence_number_counts_;
      LOG(("invalid sequence number\n"));
    }

    if (test_message->string() == kBigPing &&
        !VerifyTestData(test_message->bytes(),
                        test_message->bytes_length())) {
      ++num_invalid_big_pings_;
      LOG(("invalid big ping data\n"));
    }
  }
}

bool MasterMessageHandler::HasMessagesOrTimedout() {
  return num_received_messages_ == num_messages_waiting_to_receive_ ||
         (GetCurrentTimeMillis() - wait_for_messages_start_time_) >
            kIpcTestWaitTimeoutMs;
}

void MasterMessageHandler::ClearSavedMessages() {
  MutexLock locker(&lock_);
  source_ids_.clear();
  message_strings_.clear();
  num_received_messages_ = 0;
  num_invalid_big_pings_ = 0;
  TestingIpcMessageQueue_GetCounters(NULL, true);
}

void MasterMessageHandler::SetSaveMessages(bool save) {
  MutexLock locker(&lock_);
  save_messages_ = save;
}

// If source is 0, count the messages from all sources.
int MasterMessageHandler::CountSavedMessages(IpcProcessId source,
                                             const char16 *str) {
  MutexLock locker(&lock_);
  int count = 0;
  for (int i = 0; i < num_received_messages_; ++i) {
    bool match = true;
    if (source != 0) {
      match = (source_ids_[i] == source);
    }
    if (match && str) {
      match = (message_strings_[i] == str);
    }
    if (match) {
      ++count;
    }
  }
  return count;
}


//-----------------------------------------------------------------------------
// Testing IPC System Queue
//-----------------------------------------------------------------------------

static MasterMessageHandler g_system_ipc_master_handler;

bool TestIpcSystemQueue(std::string16 *error) {
  assert(error);

  SlaveProcess::ClearAll();

  // Register our message class and handler.
  MasterMessageHandler &g_master_handler = g_system_ipc_master_handler;

  IpcTestMessage::RegisterAsSerializable();
  IpcMessageQueue *ipc_message_queue = IpcMessageQueue::GetSystemQueue();
  TEST_ASSERT(ipc_message_queue);
  ipc_message_queue->RegisterHandler(kIpcQueue_TestMessage, &g_master_handler);

  g_master_handler.SetSaveMessages(true);
  g_master_handler.ClearSavedMessages();

  // Start two slave processes, a and b.
  SlaveProcess process_a, process_b;
  TEST_ASSERT(process_a.Start(false));
  TEST_ASSERT(process_a.WaitTillRegistered(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(process_b.Start(false));
  TEST_ASSERT(process_b.WaitTillRegistered(kIpcTestWaitTimeoutMs));

  // Each should send a "hello" message on startup, we wait for that to know
  // that they are ready to receive IpcTestMessages.
  TEST_ASSERT(g_master_handler.WaitForMessages(2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_a.id(), kHello) == 1);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_b.id(), kHello) == 1);

  // Send a "ping" to both processes and wait for expected responses.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->Send(process_a.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kPing));
  ipc_message_queue->Send(process_b.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kPing));
  TEST_ASSERT(g_master_handler.WaitForMessages(2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_a.id(), kPing) == 1);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_b.id(), kPing) == 1);

  // Send a "bigPing" to both processes and wait for expected responses.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->Send(process_a.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kBigPing, kBigPingLength));
  ipc_message_queue->Send(process_b.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kBigPing, kBigPingLength));
  TEST_ASSERT(g_master_handler.WaitForMessages(2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_a.id(), kBigPing) ==
              1);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_b.id(), kBigPing) ==
              1);

  // Tell them to quit, wait for them to do so, and verify they are no
  // longer in the registry.
  ipc_message_queue->Send(process_a.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kQuit));
  ipc_message_queue->Send(process_b.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kQuit));
  TEST_ASSERT(process_a.WaitForExit(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(process_b.WaitForExit(kIpcTestWaitTimeoutMs));

  // Start a new process c.
  g_master_handler.ClearSavedMessages();
  SlaveProcess process_c;
  TEST_ASSERT(process_c.Start(false));
  TEST_ASSERT(process_c.WaitTillRegistered(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(g_master_handler.WaitForMessages(1));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kHello) == 1);

  // Flood it with many small ping messages and wait for the expected responses.
  g_master_handler.ClearSavedMessages();
  for (int i = 0; i < kManyPings; ++i) {
    ipc_message_queue->Send(process_c.id(),
                            kIpcQueue_TestMessage,
                            new IpcTestMessage(kPing));
  }
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyPings));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kPing) ==
              kManyPings);

  // Flood it with a smaller number of large messages.
  g_master_handler.ClearSavedMessages();
  for (int i = 0; i < kManyBigPings; ++i) {
    ipc_message_queue->Send(process_c.id(),
                            kIpcQueue_TestMessage,
                            new IpcTestMessage(kBigPing, kBigPingLength));
  }
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyBigPings));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.num_invalid_big_pings() == 0);

  // Start a new process d.
  g_master_handler.ClearSavedMessages();
  SlaveProcess process_d;
  TEST_ASSERT(process_d.Start(false));
  TEST_ASSERT(process_d.WaitTillRegistered(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(g_master_handler.WaitForMessages(1));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kHello) == 1);

  // Tell c and d to send us many small pings at the same time.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->Send(process_c.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyPings));
  ipc_message_queue->Send(process_d.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyPings));
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyPings * 2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kPing) ==
             kManyPings);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kPing) ==
              kManyPings);

  // Tell c and d to send us many big pings at the same time.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->Send(process_c.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyBigPings));
  ipc_message_queue->Send(process_d.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyBigPings));
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyBigPings * 2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.num_invalid_big_pings() == 0);

  // Tell c to send us many big pings and d to send us many small pings.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->Send(process_c.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyBigPings));
  ipc_message_queue->Send(process_d.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyPings));
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyBigPings + kManyPings));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kPing) ==
              kManyPings);
  TEST_ASSERT(g_master_handler.num_invalid_big_pings() == 0);

  // Tell 'c' and 'd' to die.
  ipc_message_queue->Send(process_c.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kQuit));
  ipc_message_queue->Send(process_d.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kQuit));
  TEST_ASSERT(process_c.WaitForExit(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(process_d.WaitForExit(kIpcTestWaitTimeoutMs));

  TEST_ASSERT(g_master_handler.CountSavedMessages(0, kError) == 0);
  TEST_ASSERT(g_master_handler.num_invalid_sequence_number_counts() == 0);

  LOG(("TestIpcSystemQueue - passed\n"));
  g_master_handler.SetSaveMessages(false);
  g_master_handler.ClearSavedMessages();
  return true;
}

//-----------------------------------------------------------------------------
// Testing IPC Peer Queue
//-----------------------------------------------------------------------------

static MasterMessageHandler g_peer_ipc_master_handler;

bool TestIpcPeerQueue(std::string16 *error) {
  assert(error);

  SlaveProcess::ClearAll();

  // Register our message class and handler.
  MasterMessageHandler &g_master_handler = g_peer_ipc_master_handler;
  
  IpcTestMessage::RegisterAsSerializable();
  IpcMessageQueue *ipc_message_queue = IpcMessageQueue::GetPeerQueue();
  TEST_ASSERT(ipc_message_queue);
  ipc_message_queue->RegisterHandler(kIpcQueue_TestMessage, &g_master_handler);

  // Tell any straglers from a previous run to quit.
  g_master_handler.SetSaveMessages(false);
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kQuit),
                               false);
  TEST_ASSERT(WaitForRegisteredProcesses(1, kIpcQuitStraglersTimeoutMs));

  g_master_handler.SetSaveMessages(true);
  g_master_handler.ClearSavedMessages();

  // These test assume that the current process is the only process with an
  // ipc queue that is currently executing.
  IpcProcessId process_ids1[] = { ipc_message_queue->GetCurrentIpcProcessId() };
  TEST_ASSERT(ValidateRegisteredProcesses(ARRAYSIZE(process_ids1),
                                          process_ids1));

  // Start two slave processes, a and b.
  SlaveProcess process_a, process_b;
  TEST_ASSERT(process_a.Start(true));
  TEST_ASSERT(process_a.WaitTillRegistered(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(process_b.Start(true));
  TEST_ASSERT(process_b.WaitTillRegistered(kIpcTestWaitTimeoutMs));

  // The registry should now contain three processes.
  IpcProcessId process_ids2[] = { ipc_message_queue->GetCurrentIpcProcessId(),
                                  process_a.id(),
                                  process_b.id() };
  TEST_ASSERT(ValidateRegisteredProcesses(ARRAYSIZE(process_ids2),
                                          process_ids2));

  // Each should send a "hello" message on startup, we wait for that to know
  // that they are ready to receive IpcTestMessages.
  TEST_ASSERT(g_master_handler.WaitForMessages(2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_a.id(), kHello) == 1);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_b.id(), kHello) == 1);

  // Broadcast a "ping" and wait for expected responses.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kPing),
                               false);
  TEST_ASSERT(g_master_handler.WaitForMessages(2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_a.id(), kPing) == 1);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_b.id(), kPing) == 1);

  // Broadcast a "bigPing" and wait for expected responses.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kBigPing, kBigPingLength),
                               false);
  TEST_ASSERT(g_master_handler.WaitForMessages(2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_a.id(), kBigPing) ==
              1);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_b.id(), kBigPing) ==
              1);

  // Tell them to quit, wait for them to do so, and verify they are no
  // longer in the registry.
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kQuit),
                               false);
  TEST_ASSERT(process_a.WaitForExit(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(process_b.WaitForExit(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(WaitForRegisteredProcesses(1, kIpcTestWaitTimeoutMs));

  // Start a new process c
  g_master_handler.ClearSavedMessages();
  SlaveProcess process_c;
  TEST_ASSERT(process_c.Start(true));
  TEST_ASSERT(process_c.WaitTillRegistered(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(g_master_handler.WaitForMessages(1));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kHello) == 1);

  // Flood it with many small ping messages and wait for the expected responses.
  g_master_handler.ClearSavedMessages();
  for (int i = 0; i < kManyPings; ++i) {
    ipc_message_queue->Send(process_c.id(),
                            kIpcQueue_TestMessage,
                            new IpcTestMessage(kPing));
  }
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyPings));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kPing) ==
              kManyPings);

  // Flood it with a smaller number of large messages.
  g_master_handler.ClearSavedMessages();
  for (int i = 0; i < kManyBigPings; ++i) {
    ipc_message_queue->Send(process_c.id(),
                            kIpcQueue_TestMessage,
                            new IpcTestMessage(kBigPing, kBigPingLength));
  }
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyBigPings));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.num_invalid_big_pings() == 0);

  // Start a new process d.
  g_master_handler.ClearSavedMessages();
  SlaveProcess process_d;
  TEST_ASSERT(process_d.Start(true));
  TEST_ASSERT(process_d.WaitTillRegistered(kIpcTestWaitTimeoutMs));
  TEST_ASSERT(g_master_handler.WaitForMessages(1));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kHello) == 1);

  // Tell c and d to send us many small pings at the same time.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kSendManyPings),
                               false);
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyPings * 2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kPing) ==
             kManyPings);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kPing) ==
              kManyPings);

  // Tell c and d to send us many big pings at the same time.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kSendManyBigPings),
                               false);
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyBigPings * 2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.num_invalid_big_pings() == 0);

  // Tell c to send us many big pings and d to send us many small pings.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->Send(process_c.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyBigPings));
  ipc_message_queue->Send(process_d.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kSendManyPings));
  TEST_ASSERT(g_master_handler.WaitForMessages(kManyBigPings + kManyPings));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kBigPing) ==
              kManyBigPings);
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_d.id(), kPing) ==
              kManyPings);
  TEST_ASSERT(g_master_handler.num_invalid_big_pings() == 0);

  // Tell 'd' to die while it has the write lock on our queue.
  ipc_message_queue->Send(process_d.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kDieWhileHoldingWriteLock));
  TEST_ASSERT(process_d.WaitForExit(kIpcTestWaitTimeoutMs));

  // Ping 'c' and make sure we still get a response on our queue.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->Send(process_c.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kPing));
  TEST_ASSERT(g_master_handler.WaitForMessages(1));
  TEST_ASSERT(g_master_handler.CountSavedMessages(process_c.id(), kPing) == 1);

  // Tell 'c' to die while it has the registry lock.
  ipc_message_queue->Send(process_c.id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kDieWhileHoldingRegistryLock));
  TEST_ASSERT(process_c.WaitForExit(kIpcTestWaitTimeoutMs));

  // Test that a running process, ours, will cleanup the registry after others
  // have crashed without removing themselves from the registry.
  TEST_ASSERT(WaitForRegisteredProcesses(1, kIpcTestWaitTimeoutMs));

  // Start many processes.
  g_master_handler.ClearSavedMessages();
  SlaveProcess many_slaves[kManySlavesCount];
  for (int i = 0; i < kManySlavesCount; ++i) {
    TEST_ASSERT(many_slaves[i].Start(true));
  }
  for (int i = 0; i < kManySlavesCount; ++i) {
    TEST_ASSERT(many_slaves[i].WaitTillRegistered(kIpcTestWaitTimeoutMs));
  }
  TEST_ASSERT(g_master_handler.WaitForMessages(kManySlavesCount));
  TEST_ASSERT(g_master_handler.CountSavedMessages(0, kHello) ==
              kManySlavesCount);

  // Have them chit-chat with one another.
  g_master_handler.ClearSavedMessages();
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kDoChitChat),
                               false);
  TEST_ASSERT(g_master_handler.WaitForMessages(kManySlavesCount * 2));
  TEST_ASSERT(g_master_handler.CountSavedMessages(0, kChit) ==
              kManySlavesCount);
  TEST_ASSERT(g_master_handler.CountSavedMessages(0, kDidChitChat) ==
              kManySlavesCount);
  TEST_ASSERT(ValidateRegisteredProcesses(kManySlavesCount + 1, NULL));
  for (int i = 0; i < kManySlavesCount; ++i) {
    TEST_ASSERT(2 == g_master_handler.CountSavedMessages(
                                          many_slaves[i].id(), NULL));
  }

  // Tell the first to quit in a would be dead lock situation.
  ipc_message_queue->Send(many_slaves[0].id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(kQuitWhileHoldingRegistryLock));

  // Tell the remainder to quit normally.
  ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                               new IpcTestMessage(kQuit),
                               false);
  for (int i = 0; i < kManySlavesCount; ++i) {
    TEST_ASSERT(many_slaves[i].WaitForExit(kIpcTestWaitTimeoutMs));
  }

  TEST_ASSERT(WaitForRegisteredProcesses(1, kIpcTestWaitTimeoutMs));
  TEST_ASSERT(ValidateRegisteredProcesses(ARRAYSIZE(process_ids1),
                                          process_ids1));

  TEST_ASSERT(g_master_handler.CountSavedMessages(0, kError) == 0);
  TEST_ASSERT(g_master_handler.num_invalid_sequence_number_counts() == 0);

  LOG(("TestIpcPeerQueue - passed\n"));
  g_master_handler.SetSaveMessages(false);
  g_master_handler.ClearSavedMessages();
  return true;
}


//-----------------------------------------------------------------------------
// Slave process test code
//-----------------------------------------------------------------------------

std::list<IpcProcessId> SlaveProcess::slave_processes_;

void SlaveProcess::SendAll(IpcMessageQueue *ipc_message_queue,
                           const char16 *message) {
  assert(message);

  if (ipc_message_queue) {
    for (std::list<IpcProcessId>::const_iterator iter =
              slave_processes_.begin();
         iter != slave_processes_.end();
         ++iter) {
      ipc_message_queue->Send(*iter,
                              kIpcQueue_TestMessage,
                              new IpcTestMessage(message));
    }
  }
}

void SlaveProcess::ClearAll() {
  slave_processes_.clear();
}

void SlaveMessageHandler::SendIpcMessage(IpcMessageQueue *ipc_message_queue,
                                         IpcProcessId source_process_id,
                                         IpcTestMessage *message) {
  assert(ipc_message_queue);
  assert(message);

  int sequence_number = 0;
  static std::map<IpcProcessId, int> last_sequence_number_map;
  std::map<IpcProcessId, int>::const_iterator iter =
      last_sequence_number_map.find(source_process_id);
  if (iter != last_sequence_number_map.end()) {
    sequence_number = iter->second + 1;
  }
  last_sequence_number_map[source_process_id] = sequence_number;

  message->set_sequence_number(sequence_number);
  ipc_message_queue->Send(source_process_id, kIpcQueue_TestMessage, message);
}

void SlaveMessageHandler::HandleIpcMessage(
                              IpcProcessId source_process_id,
                              int message_type,
                              const IpcMessageData *message_data) {
  assert(message_type == kIpcQueue_TestMessage);
  const IpcTestMessage *test_message = static_cast<const IpcTestMessage*>
                                          (message_data);

  std::string narrow_string;
  String16ToUTF8(test_message->string().c_str(), &narrow_string);
  LOG(("Slave received %s\n", narrow_string.c_str()));

  IpcMessageQueue *ipc_message_queue = GetIpcMessageQueue();

  if (test_message->string() == kPing) {
    SendIpcMessage(ipc_message_queue,
                   source_process_id,
                   new IpcTestMessage(kPing));

  } else if (test_message->string() == kQuit) {
    TerminateSlave();

#ifdef WIN32
  } else if (test_message->string() == kQuitWhileHoldingRegistryLock) {
    done_ = true;
    TestingIpcMessageQueueWin32_SleepWhileHoldingRegistryLock(
        ipc_message_queue);

  } else if (test_message->string() == kDieWhileHoldingWriteLock) {
    TestingIpcMessageQueueWin32_DieWhileHoldingWriteLock(ipc_message_queue,
                                                         source_process_id);

  } else if (test_message->string() == kDieWhileHoldingRegistryLock) {
    TestingIpcMessageQueueWin32_DieWhileHoldingRegistryLock(ipc_message_queue);
#endif  // WIN32

  } else if (test_message->string() == kBigPing) {
    if (test_message->bytes_length() == kBigPingLength &&
        VerifyTestData(test_message->bytes(),
                       test_message->bytes_length())) {
      SendIpcMessage(ipc_message_queue,
                     source_process_id, 
                     new IpcTestMessage(kBigPing, kBigPingLength));
    } else {
      SendIpcMessage(ipc_message_queue,
                     source_process_id,
                     new IpcTestMessage(kError));
    }


  } else if (test_message->string() == kSendManyPings) {
    for (int i = 0; i < kManyPings; ++i) {
      SendIpcMessage(ipc_message_queue,
                     source_process_id,
                     new IpcTestMessage(kPing));
    }

  } else if (test_message->string() == kSendManyBigPings) {
    for (int i = 0; i < kManyBigPings; ++i) {
      SendIpcMessage(ipc_message_queue,
                     source_process_id,
                     new IpcTestMessage(kBigPing, kBigPingLength));
    }
  }

  else if (test_message->string() == kDoChitChat) {
    ipc_message_queue->SendToAll(kIpcQueue_TestMessage,
                                 new IpcTestMessage(kChit),
                                 false);

  } else if (test_message->string() == kChit) {
    ++chits_received_;
    chit_sources_.insert(source_process_id);
    ipc_message_queue->Send(source_process_id,
                            kIpcQueue_TestMessage,
                            new IpcTestMessage(kChat));
    if (chits_received_ > kManySlavesCount - 1) {
      ipc_message_queue->Send(parent_process_id_,
                              kIpcQueue_TestMessage,
                              new IpcTestMessage(kError));
    } else if ((chats_received_ == kManySlavesCount - 1) &&
               (chits_received_ == kManySlavesCount - 1) &&
               (chat_sources_.size() == kManySlavesCount - 1) &&
               (chit_sources_.size() == kManySlavesCount - 1)) {
      ipc_message_queue->Send(parent_process_id_,
                              kIpcQueue_TestMessage,
                              new IpcTestMessage(kDidChitChat));
    }

  } else if (test_message->string() == kChat) {
    ++chats_received_;
    chat_sources_.insert(source_process_id);
    if (chits_received_ > kManySlavesCount - 1) {
      ipc_message_queue->Send(parent_process_id_,
                              kIpcQueue_TestMessage,
                              new IpcTestMessage(kError));
    } else if ((chats_received_ == kManySlavesCount - 1) &&
               (chits_received_ == kManySlavesCount - 1) &&
               (chat_sources_.size() == kManySlavesCount - 1) &&
               (chit_sources_.size() == kManySlavesCount - 1)) {
      ipc_message_queue->Send(parent_process_id_,
                              kIpcQueue_TestMessage,
                              new IpcTestMessage(kDidChitChat));
    }

  }
}

#endif  // USING_CCTESTS

#endif  // (WIN32 && !WINCE) || LINUX || OS_MACOSX
