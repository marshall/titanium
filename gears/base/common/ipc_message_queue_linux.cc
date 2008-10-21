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

#if defined(LINUX) || defined(OS_MACOSX)

#include <deque>
#include <list>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#if defined(OS_MACOSX)
#include <sys/syslimits.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "gears/base/common/common.h"
#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// Constants.

// Bump the version when anything in the header is changed.
static const int kIpcMessageHeaderVersion = 1;

// The size of header plus the data should be less than PIPE_BUF.
#ifdef OS_MACOSX
static const int kPacketCapacity = 450;
#else
static const int kPacketCapacity = 2000;
#endif  // OS_MACOSX

static const int kMaxInboundListCleanupCounter = 20;
static const int kInboundMessageInactiveTimeoutMs = 60 * 1000;  // 1m
static const int kOutboundMessageInactiveTimeoutMs = 60 * 1000; // 1m

struct SignalHandlingInfo {
  int sig_num;
  bool replace_default_only;
};
static const SignalHandlingInfo kTerminateSignalsToCatch[] = {
  { SIGHUP,    false },
  { SIGINT,    false },
  { SIGUSR1,   true  },
  { SIGUSR2,   true  },
  { SIGPIPE,   true  },
  { SIGALRM,   true  },
  { SIGTERM,   false },
  { SIGVTALRM, true  },
  { SIGPROF,   true  },
  { SIGIO,     true  },
#ifndef OS_MACOSX
  { SIGPWR,    false }
#endif  // !OS_MACOSX
};

// Message header to read/write in FIFO.
// If you make any change to this struct, do not forget to bump up
// kIpcMessageHeaderVersion.
struct FifoMessageHeader {
  int header_version;
  IpcProcessId src_process_id;
  int message_type;
  int sequence_number;
  bool last_packet;
  int packet_size;

  FifoMessageHeader() 
    : header_version(0),
      src_process_id(0),
      message_type(-1),
      sequence_number(0),
      last_packet(false),
      packet_size(0) {
  }

  FifoMessageHeader(int src_process_id,
                    int message_type,
                    int sequence_number,
                    bool last_packet,
                    int packet_size)
    : header_version(kIpcMessageHeaderVersion),
      src_process_id(src_process_id),
      message_type(message_type),
      sequence_number(sequence_number),
      last_packet(last_packet),
      packet_size(packet_size) {
  }
};

class OutboundIpcMessage : public RefCounted {
 public:
  OutboundIpcMessage(IpcProcessId dest_process_id,
                     int message_type,
                     IpcMessageData *message_data,
                     IpcMessageQueue::SendCompletionCallback callback,
                     void *callback_param)
    : dest_process_id_(dest_process_id),
      message_type_(message_type),
      last_active_time_(0),
      message_data_(message_data),
      sent_size_(0),
      callback_(callback),
      callback_param_(callback_param) {
    set_last_active_time();
  }

  IpcProcessId dest_process_id() const { return dest_process_id_; }

  int message_type() const { return message_type_; }

  int64 last_active_time() const { return last_active_time_; }

  void set_last_active_time() {
    last_active_time_ = GetCurrentTimeMillis();
  }

  IpcMessageData *message_data() const { return message_data_.get(); }

  int sent_size() const { return sent_size_; }

  void add_sent_size(int packet_size) { sent_size_ += packet_size; }

  std::vector<uint8> *serialized_message_data() {
    if (!serialized_message_data_.get()) {
      assert(message_data_.get());
      serialized_message_data_.reset(new std::vector<uint8>);
      Serializer serializer(serialized_message_data_.get());
      if (!serializer.WriteObject(message_data_.get())) {
        serialized_message_data_.reset(NULL);
      }
    }
    return serialized_message_data_.get();
  }

  IpcMessageQueue::SendCompletionCallback callback() const {
    return callback_;
  }

  void *callback_param() const { return callback_param_; }

 private:
  IpcProcessId dest_process_id_;
  int message_type_;
  int64 last_active_time_;
  scoped_ptr<IpcMessageData> message_data_;
  scoped_ptr< std::vector<uint8> > serialized_message_data_;
  int sent_size_;
  IpcMessageQueue::SendCompletionCallback callback_;
  void *callback_param_;
};

class OutboundIpcMessageList : public RefCounted {
 public:
  OutboundIpcMessageList(IpcProcessId dest_process_id)
    : dest_process_id_(dest_process_id) {
  }

  IpcProcessId dest_process_id() const { return dest_process_id_; }

  std::deque< scoped_refptr<OutboundIpcMessage> >* messages() {
    return &messages_;
  }

 private:
  IpcProcessId dest_process_id_;
  std::deque< scoped_refptr<OutboundIpcMessage> > messages_;
};

class InboundIpcMessage : public RefCounted {
 public:
   InboundIpcMessage(IpcProcessId src_process_id, int message_type) 
    : src_process_id_(src_process_id),
      message_type_(message_type),
      last_active_time_(0) {
   }

   IpcProcessId src_process_id() const { return src_process_id_; }

   int message_type() const { return message_type_; }

   std::vector<uint8> *serialized_message_data() {
     return &serialized_message_data_;
   }

   int64 last_active_time() const { return last_active_time_; }

   void set_last_active_time() {
     last_active_time_ = GetCurrentTimeMillis();
   }

 private:
  IpcProcessId src_process_id_;
  int message_type_;
  int64 last_active_time_;
  std::vector<uint8> serialized_message_data_;
};

class LinuxIpcWorkerThread;

class LinuxIpcMessageQueue : public IpcMessageQueue {
 public:
  LinuxIpcMessageQueue();
  ~LinuxIpcMessageQueue();
  bool Init();
  void Terminate();
  void Run();
  static void OnSignalTerminate(int sig_num);

  // IpcMessageQueue overrides
  virtual IpcProcessId GetCurrentIpcProcessId();
  // Does not support sending a message to itself.
  virtual void SendWithCompletion(IpcProcessId dest_process_id,
                                  int message_type,
                                  IpcMessageData *message_data,
                                  SendCompletionCallback callback,
                                  void *callback_param);
  // Not supported.
  virtual void SendToAll(int message_type,
                         IpcMessageData *message_data,
                         bool including_current_process);

  void AddToOutboundList(IpcProcessId dest_process_id,
                         OutboundIpcMessage *outbound_message);

 private:
  // Our worker thread's entry point and running routines
  void ProcessOutboundMessages(bool *still_pending_to_send);
  void ProcessInboundMessages();
  bool SendOneMessage(IpcProcessId dest_process_id,
                      OutboundIpcMessage *message);
  bool ReceiveOneMessage(bool *fatal_error);
  bool ReadMessageHeader(FifoMessageHeader *message_header, bool *fatal_error);
  bool GetPendingMessage(const FifoMessageHeader &message_header,
                         scoped_refptr<InboundIpcMessage> *inbound_message);
  bool ReadMessageBody(const FifoMessageHeader &message_header,
                       InboundIpcMessage* inbound_message);
  bool HandleMessage(const FifoMessageHeader &message_header,
                     InboundIpcMessage* inbound_message);

  static std::string BuildFifoPath(IpcProcessId process_id);

  bool die_;
  IpcProcessId current_process_id_;
  int outbound_signaling_fds_[2];
  std::string inbound_fifo_path_;
  int inbound_fifo_fd_;
  scoped_ptr<LinuxIpcWorkerThread> thread_;
  Mutex thread_sync_mutex_;
  int inbound_list_cleanup_counter_;

  typedef void SigHandler(int);
  SigHandler* old_sig_handlers_[ARRAYSIZE(kTerminateSignalsToCatch)];

  typedef std::list< scoped_refptr<OutboundIpcMessageList> > OutboundList;
  OutboundList outbound_pendings_;

  typedef std::list< scoped_refptr<InboundIpcMessage> > InboundList;
  InboundList inbound_pendings_;

  static LinuxIpcMessageQueue *self_;
};

class LinuxIpcWorkerThread : public Thread {
 public:
  LinuxIpcWorkerThread(LinuxIpcMessageQueue *ipc_message_queue)
      : ipc_message_queue_(ipc_message_queue) {
    assert(ipc_message_queue);
  }

  virtual ~LinuxIpcWorkerThread() {
  }

 private:
  virtual void Run() {
    ipc_message_queue_->Run();
  }

  LinuxIpcMessageQueue *ipc_message_queue_;
  DISALLOW_EVIL_CONSTRUCTORS(LinuxIpcWorkerThread);
};

LinuxIpcMessageQueue *LinuxIpcMessageQueue::self_ = NULL;

LinuxIpcMessageQueue::LinuxIpcMessageQueue()
  : die_(true),
    current_process_id_(0),
    inbound_fifo_fd_(-1),
    inbound_list_cleanup_counter_(0) {
  current_process_id_ = getpid();
  outbound_signaling_fds_[0] = outbound_signaling_fds_[1] = -1;
  for (size_t i = 0; i < ARRAYSIZE(old_sig_handlers_); ++i) {
    old_sig_handlers_[i] = SIG_IGN;
  }
}

LinuxIpcMessageQueue::~LinuxIpcMessageQueue() {
  Terminate();
}

std::string LinuxIpcMessageQueue::BuildFifoPath(IpcProcessId process_id) {
  std::string fifo_path("/tmp/" PRODUCT_SHORT_NAME_ASCII);
  fifo_path.append(IntegerToString(static_cast<int32>(process_id)));
  fifo_path.append(".ipc");
  fifo_path.append(IntegerToString(kIpcMessageHeaderVersion));
  return fifo_path;
}

bool LinuxIpcMessageQueue::Init() {
  LOG(("Initializing LinuxIpcMessageQueue for process %d\n", getpid()));

  // Create a pipe used for the signal for the main thread to tell the worker
  // thread that the outbound data is to be processed.
  int rv = pipe(outbound_signaling_fds_);
  if (rv == -1) {
    LOG(("Failed to create an outbound signaling pipe. errno=%d\n", errno));
    return false;
  }

  // Set the pipe fd as non-blocking.
  for (int i = 0; i < 2; ++i) {
    int flags = fcntl(outbound_signaling_fds_[i], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(outbound_signaling_fds_[i], F_SETFL, flags);
  }

  // Create a FIFO-special file.
  inbound_fifo_path_ = BuildFifoPath(current_process_id_);

  rv = mkfifo(inbound_fifo_path_.c_str(), 0600);
  if (rv == -1) {
    if (errno != EEXIST) {
      LOG(("Creating inbound fifo %s failed with errno=%d\n",
           inbound_fifo_path_.c_str(), errno));
      return false;
    }

    // If the FIFO already exists, delete and recreate it.
    LOG(("Fifo %s exists. Recreate it.\n", inbound_fifo_path_.c_str()));
    unlink(inbound_fifo_path_.c_str());
    rv = mkfifo(inbound_fifo_path_.c_str(), 0600);
    if (rv == -1) {
      LOG(("Recreating inbound fifo %s failed with errno=%d\n",
           inbound_fifo_path_.c_str(), errno));
      return false;
    }
  }

  // Note: we have to open the FIFO file in non-blocking RDWR mode instead of
  // non-blocking RDONLY mode because otherwise the select operation will always
  // return but no data is available to read after all the data has been read.
  //
  // When a FIFO has no writer, an EOF condition is placed on it. From now on,
  // the FIFO will always be considered readable even though 0 byte is read,
  // which means EOF.
  inbound_fifo_fd_ = open(inbound_fifo_path_.c_str(), O_RDWR | O_NONBLOCK);
  if (inbound_fifo_fd_ == -1) {
    LOG(("Opening inbound fifo %s failed with errno=%d\n",
         inbound_fifo_path_.c_str(),errno));
    return false;
  }

  die_ = false;

  // Set up signal handling for those signals that could cause a process to
  // terminate.
  struct sigaction sig_action, old_sig_action;
  sig_action.sa_handler = LinuxIpcMessageQueue::OnSignalTerminate;
  sigemptyset(&sig_action.sa_mask);
  sig_action.sa_flags = 0;
  for (size_t i = 0; i < ARRAYSIZE(kTerminateSignalsToCatch); ++i) {
    if (kTerminateSignalsToCatch[i].replace_default_only) {
      if (sigaction(kTerminateSignalsToCatch[i].sig_num,
                    NULL,
                    &old_sig_action) == -1) {
        LOG(("Getting old handler for signal %d failed\n",
             kTerminateSignalsToCatch[i]));
        continue;
      }
      if (old_sig_action.sa_handler != SIG_DFL) {
        continue;
      }
    }

    if (sigaction(kTerminateSignalsToCatch[i].sig_num,
                  &sig_action,
                  &old_sig_action) == -1) {
      LOG(("Setting up handler for signal %d failed\n",
           kTerminateSignalsToCatch[i]));
    } else {
      if (old_sig_action.sa_handler != SIG_IGN) {
        old_sig_handlers_[i] = old_sig_action.sa_handler;
      }
    }
  }
  self_ = this;

  // Start a worker thread.
  thread_.reset(new LinuxIpcWorkerThread(this));
  if (!thread_->Start()) {
    return false;
  }

  return true;
}

void LinuxIpcMessageQueue::OnSignalTerminate(int sig_num) {
  if (self_) {
    unlink(self_->inbound_fifo_path_.c_str());

    SigHandler* handler = NULL;
    for (size_t i = 0; i < ARRAYSIZE(kTerminateSignalsToCatch); ++i) {
      if (sig_num == kTerminateSignalsToCatch[i].sig_num) {
        handler = self_->old_sig_handlers_[i];
        break;
      }
    }

    if (handler == SIG_DFL) {
      exit(-1);
    } else if (handler != SIG_IGN) {
      handler(sig_num);
      assert(false);    // assert to make sure the old handler exits.
    }
  }
}

// Once LinuxIpcMessageQueue is terminated, it cannot be reused.
void LinuxIpcMessageQueue::Terminate() {
  LOG(("Terminating LinuxIpcMessageQueue of process %d\n",
       current_process_id_));

  // Signal and wait for the worker thread to finish.
  die_ = true;
  if (thread_.get() && thread_->IsRunning()) {
    char ch = 'q';
    if (write(outbound_signaling_fds_[1], &ch, 1) < 1) {
      LOG(("Writing to the signaling pipe failed with errno=%d\n", errno));
    }
    LOG(("Waiting for worker thread of process %u to join\n",
         current_process_id_));
    thread_->Join();
  }

  // Clean up.
  if (inbound_fifo_fd_ != -1) {
    close(inbound_fifo_fd_);
  }
  if (!inbound_fifo_path_.empty()) {
    unlink(inbound_fifo_path_.c_str());
  }
  if (outbound_signaling_fds_[0]) {
    close(outbound_signaling_fds_[0]);
    close(outbound_signaling_fds_[1]);
  }
}

void LinuxIpcMessageQueue::Run() {
  bool still_pending_to_send = false;
  while (!die_) {
    // Wait until there is either inbound data or outbound data available.
    //
    // TODO (jianli): consider adding a fd to facilitate the smooth sending and
    // receiving of big messages.
    // 1) The sender sends the fd in the message header.
    // 2) The receiver signals via the fd when it consumes the packet.
    // 3) The sender adds the fd to the select set.
    fd_set select_fd_set;
    FD_ZERO(&select_fd_set);
    FD_SET(outbound_signaling_fds_[0], &select_fd_set);
    FD_SET(inbound_fifo_fd_, &select_fd_set);
    int max_fd = std::max(outbound_signaling_fds_[0], inbound_fifo_fd_);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    int rv = select(max_fd + 1, &select_fd_set, NULL, NULL, 
                    still_pending_to_send ? &timeout : NULL);
    if (rv == -1) {
      LOG(("select failed with errno=%d\n", errno));
      break;
    }

    if (still_pending_to_send ||
        FD_ISSET(outbound_signaling_fds_[0], &select_fd_set)) {
      still_pending_to_send = false;
      ProcessOutboundMessages(&still_pending_to_send);
    }

    if (FD_ISSET(inbound_fifo_fd_, &select_fd_set)) {
      ProcessInboundMessages();
    }
  }
}

void LinuxIpcMessageQueue::ProcessOutboundMessages(
        bool *still_pending_to_send) {
  assert(still_pending_to_send);

  std::vector< scoped_refptr<OutboundIpcMessage> > successful_messages;
  std::vector< scoped_refptr<OutboundIpcMessage> > failed_messages;

  {
    MutexLock locker(&thread_sync_mutex_);
  
    // Read all the signaling chars.
    char ch = 0;
    while (read(outbound_signaling_fds_[0], &ch, 1) > 0) {
      if (ch == 'q') {
        die_ = true;
        return;
      }
    }
  
    // Process the outbound list.
    for (OutboundList::iterator iter = outbound_pendings_.begin();
         iter != outbound_pendings_.end();
         ) {
      assert(!iter->get()->messages()->empty());
      OutboundIpcMessage *message = iter->get()->messages()->front().get();
  
      // If either all the message is sent successfully or the message is failed
      // to send, remove it from the pending queue.
      bool res = SendOneMessage(iter->get()->dest_process_id(), message);
      if (!res ||
          message->sent_size() >= 
              static_cast<int>(message->serialized_message_data()->size())) {
        if (res) {
          successful_messages.push_back(message);
        } else {
          failed_messages.push_back(message);
        }
        iter->get()->messages()->pop_front();
        if (iter->get()->messages()->empty()) {
          iter = outbound_pendings_.erase(iter);
          continue;
        }
      }
      ++iter;
    }
  
    *still_pending_to_send = !outbound_pendings_.empty();
  }

  // Inform the result listener. This has to be done outside the mutex-protected
  // block to avoid the mutex reentrancy problem if the listener callback
  // decide to resend the message.
  for (size_t i = 0; i < successful_messages.size(); ++i) {
    if (successful_messages[i]->callback()) {
      (*successful_messages[i]->callback())(
            true,
            successful_messages[i]->dest_process_id(),
            successful_messages[i]->message_type(),
            successful_messages[i]->message_data(),
            successful_messages[i]->callback_param());
    }
  }
  for (size_t i = 0; i < failed_messages.size(); ++i) {
    if (failed_messages[i]->callback()) {
      (*failed_messages[i]->callback())(
            false,
            failed_messages[i]->dest_process_id(),
            failed_messages[i]->message_type(),
            failed_messages[i]->message_data(),
            failed_messages[i]->callback_param());
    }
  }
}

// Send one message to a destination process.
// Return true if one of the following happens:
//   * all message is sent successfully.
//   * partial message is sent successfully.
//   * no space available.
// Return false otherwise when encountering any error prevent from sending,
// i.e. the destination process dies.
bool LinuxIpcMessageQueue::SendOneMessage(IpcProcessId dest_process_id,
                                          OutboundIpcMessage *message) {
  assert(message);

  // Check if the fifo has already been created.
  std::string outbound_fifo_path = BuildFifoPath(dest_process_id);
  if (access(outbound_fifo_path.c_str(), F_OK) == -1) {
    LOG(("FIFO of process %u is not created\n", dest_process_id));
    return false;
  }

  // Check if the destination process is still live.
  if (kill(dest_process_id, 0) == -1) {
    LOG(("Process %u is not valid\n", dest_process_id));
    unlink(outbound_fifo_path.c_str());
    return false;
  }

  // Construct the data.
  std::vector<uint8> outbound_data;
  bool last_packet = true;
  int total_size = message->serialized_message_data()->size();
  int packet_size = total_size - message->sent_size();
  if (packet_size > kPacketCapacity) {
    packet_size = kPacketCapacity;
    last_packet = false;
  }
  int sequence_number = (message->sent_size() + packet_size - 1) /
                        kPacketCapacity;
  FifoMessageHeader message_header(current_process_id_,
                                   message->message_type(),
                                   sequence_number,
                                   last_packet,
                                   packet_size);
  outbound_data.resize(sizeof(message_header) + packet_size);
  memcpy(&outbound_data.at(0), &message_header, sizeof(message_header));
  memcpy(&outbound_data.at(0) + sizeof(message_header),
         &message->serialized_message_data()->at(message->sent_size()),
         packet_size);

  // Check if the data is not larger than PIPE_BUF, which is the maximum number
  // of bytes that are guaranteed not to be interleaved with data from other
  // processes doing writes on the same FIFO.
  // Do this check only in debug build since we are sending the message in
  // packets that are guaranteed to have smaller size than PIPE_BUF.
#if DEBUG
  if (outbound_data.size() > PIPE_BUF) {
    LOG(("Outbound data has size %u greater than required %u",
         outbound_data.size(), PIPE_BUF));
    return false;
  }
#endif

  // Open the FIFO file.
  int outbound_fifo_fd = open(outbound_fifo_path.c_str(),
                              O_WRONLY | O_NONBLOCK);
  if (outbound_fifo_fd == -1) {
    LOG(("Opening outbound fifo %s failed with errno=%d\n",
         outbound_fifo_path.c_str(), errno));
    return false;
  }

  // Write all the data atomically.
  bool need_retry = false;
  bool sent_successfully = false;
  int num = write(outbound_fifo_fd, &outbound_data.at(0), outbound_data.size());
  if (num == -1) {
    if (errno == EAGAIN) {
      // No space is available. Should retry.
      LOG(("No space is available for FIFO %u\n", dest_process_id));
      need_retry = true;
    } else {
      // For all other errors, discard the message.
      LOG(("Writing to FIFO %u failed with errno=%d\n",
           dest_process_id, errno));
    }
  } else {
    if (num == static_cast<int>(outbound_data.size())) {
      LOG(("Sent a message with type %d and range %d-%d %s"
           "from process %u to process %u\n",
           message->message_type(),
           message->sent_size(),
           message->sent_size() + packet_size - 1,
           last_packet ? "(last) " : "",
           current_process_id_,
           dest_process_id));
      message->add_sent_size(packet_size);
      sent_successfully = true;
    } else {
      // Is this possible?
      assert(false);
    }
  }

  // Close the FIFO file.
  close(outbound_fifo_fd);

  if (sent_successfully) {
    message->set_last_active_time();
  } else if (need_retry) {
    // If we do not make any progress to send out one packet, fail.
    if (message->last_active_time() - GetCurrentTimeMillis() >
        kOutboundMessageInactiveTimeoutMs) {
      need_retry = false;
    }
  }

  return sent_successfully || need_retry;
}

void LinuxIpcMessageQueue::ProcessInboundMessages() {
  // Keep reading messages until there is an error or no more data.
  while (!die_) {
    bool fatal_error = true;
    if (!ReceiveOneMessage(&fatal_error)) {
      if (fatal_error) {
        die_ = true;
      }
      break;
    }
  }

  // Remove those incomplete messages from the cache list if no new packet has
  // been received for specific amount of time.
  inbound_list_cleanup_counter_++;
  if (inbound_list_cleanup_counter_ > kMaxInboundListCleanupCounter) {
    inbound_list_cleanup_counter_ = 0;
    for (InboundList::iterator iter = inbound_pendings_.begin();
         iter != inbound_pendings_.end();
         ++iter) {
      if (iter->get()->last_active_time() - GetCurrentTimeMillis() >
          kInboundMessageInactiveTimeoutMs) {
        LOG(("Removed incomplete timed out message with type %d and range 0-%u "
             "sent from process %u to process %u\n",
             iter->get()->message_type(),
             iter->get()->serialized_message_data()->size() - 1,
             iter->get()->src_process_id(),
             current_process_id_));
        iter = inbound_pendings_.erase(iter);
        --iter;
      }
    }
  }
}

// Receive one message. Reture true if a message has been received successfully.
// Return false otherwise.
bool LinuxIpcMessageQueue::ReceiveOneMessage(bool *fatal_error) {
  assert(fatal_error);

  *fatal_error = true;
  FifoMessageHeader message_header;
  if (!ReadMessageHeader(&message_header, fatal_error)) {
    return false;
  }
  scoped_refptr<InboundIpcMessage> inbound_message;
  if (!GetPendingMessage(message_header, &inbound_message)) {
    return false;
  }
  if (!ReadMessageBody(message_header, inbound_message.get())) {
    return false;
  }
  return HandleMessage(message_header, inbound_message.get());
}

// Read the message header.
bool LinuxIpcMessageQueue::ReadMessageHeader(FifoMessageHeader *message_header,
                                             bool *fatal_error) {
  assert(message_header);
  assert(fatal_error);

  *fatal_error = true;
  int num = read(inbound_fifo_fd_, message_header, sizeof(FifoMessageHeader));
  if (num == -1) {
    if (errno == EAGAIN) {
      // No data to read. Not a fatal error.
      *fatal_error = false;
    } else {
      // Report errors other than EAGAIN (no data available).
      LOG(("Reading message header from FIFO %u failed with errno=%d\n", 
           current_process_id_, errno));
    }
    return false;
  } else if (num == 0) {
    // No data to read. Not a fatal error.
    *fatal_error = false;
    return false;
  } else if (num < static_cast<int>(sizeof(FifoMessageHeader))) {
    LOG(("Not enough message header read from FIFO %u\n",
         current_process_id_));
    return false;
  }

  // Sanity check.
  if (message_header->header_version != kIpcMessageHeaderVersion) {
    LOG(("Message header version from FIFO %u mismatches\n",
         current_process_id_));
    return false;
  }
  if (message_header->packet_size > kPacketCapacity) {
    LOG(("Message header from FIFO %u failed with sanity check\n",
         current_process_id_));
    return false;
  }

  return true;
}

// Get the pending message.
bool LinuxIpcMessageQueue::GetPendingMessage(
        const FifoMessageHeader &message_header,
        scoped_refptr<InboundIpcMessage> *inbound_message) {
  assert(inbound_message);

  // Try to find the pending message in the cached list.
  for (InboundList::iterator iter = inbound_pendings_.begin();
       iter != inbound_pendings_.end();
       ++iter) {
    if (message_header.src_process_id == iter->get()->src_process_id()) {
      // If sequence number is 0, remove the expired incomplete message from the
      // cache list. This is to handle the case that the process ID is reused
      // and the previous process with same ID did not finish sending the
      // complete message.
      if (message_header.sequence_number == 0) {
        LOG(("Reset message data read from FIFO %u\n", current_process_id_));
        inbound_pendings_.erase(iter);
        break;
      }

      // Try to validate some header properties.
      bool validate_error = false;
      if (message_header.message_type != iter->get()->message_type()) {
        LOG(("Message type %d read from FIFO %u different from prev one %d\n,",
             message_header.message_type, current_process_id_,
             iter->get()->message_type()));
        validate_error = true;
      }
      int old_sequence_number = 
          (iter->get()->serialized_message_data()->size() - 1) / 
          kPacketCapacity;
      if (message_header.sequence_number != old_sequence_number + 1) {
        LOG(("Sequence num %d read from FIFO %u mismatched previous one %d\n,",
             message_header.sequence_number, current_process_id_,
             old_sequence_number));
        validate_error = true;
      }
      if (validate_error) {
        inbound_pendings_.erase(iter);
        return false;
      }

      // Get the cached pending message.
      inbound_message->reset(iter->get());

      // If it is the last packet, take it off from the cache list.
      if (message_header.last_packet) {
        inbound_pendings_.erase(iter);
      }

      return true;
    }
  }

  // Validate for the new message.
  if (message_header.sequence_number != 0) {
    LOG(("Sequence num of new message read from FIFO %u is not 0\n",
         current_process_id_));
    return false;
  }

  // Create a new message. If we receive other than the last packet, put it
  // in the cache list.
  inbound_message->reset(new InboundIpcMessage(message_header.src_process_id,
                                               message_header.message_type));
  if (!message_header.last_packet) {
    inbound_pendings_.push_back(*inbound_message);
  }

  return true;
}

// Read the message body.
bool LinuxIpcMessageQueue::ReadMessageBody(
        const FifoMessageHeader &message_header,
        InboundIpcMessage* inbound_message) {
  assert(inbound_message);

  int old_size = inbound_message->serialized_message_data()->size();
  int new_size = old_size + message_header.packet_size;
  inbound_message->serialized_message_data()->resize(new_size);
  int num = read(inbound_fifo_fd_,
                 &inbound_message->serialized_message_data()->at(old_size),
                 message_header.packet_size);
  if (num == -1 || num < message_header.packet_size) {
    if (num == -1) {
      LOG(("Reading message body from FIFO %u failed with errno=%d\n", 
           current_process_id_, errno));
    } else {
      LOG(("Not enough message body read from FIFO %u\n", current_process_id_));
    }

    // Remove the message from the cache list. Do not need to do it if we fail
    // to read the last packet, it has already been taken off from the cache
    // list.
    if (!message_header.last_packet) {
      for (InboundList::iterator iter = inbound_pendings_.begin();
           iter != inbound_pendings_.end();
           ++iter) {
        if (message_header.src_process_id == iter->get()->src_process_id()) {
          inbound_pendings_.erase(iter);
          break;
        }
      }
    }

    return false;
  }

  inbound_message->set_last_active_time();

  LOG(("Received a message with type %d and range %d-%d %s"
       "from process %u to process %u\n",
       message_header.message_type,
       old_size,
       new_size - 1,
       message_header.last_packet ? "(last) " : "",
       message_header.src_process_id,
       current_process_id_));

  return true;
}

// Deserialize and handle the message if it is complete.
bool LinuxIpcMessageQueue::HandleMessage(
        const FifoMessageHeader &message_header,
        InboundIpcMessage* inbound_message) {
  assert(inbound_message);

  // Deserialize the message if we receive the last packet.
  if (message_header.last_packet && !die_) {
    Deserializer deserializer(
        &inbound_message->serialized_message_data()->at(0),
        inbound_message->serialized_message_data()->size());
    IpcMessageData *message = NULL;
    if (deserializer.CreateAndReadObject(&message) && message != NULL) {
      // Call the handler.
      CallRegisteredHandler(message_header.src_process_id,
                            message_header.message_type,
                            message);
      delete message;
    } else {
      LOG(("Failed to deserialize the message data read from FIFO %u\n",
           current_process_id_));
    }
  }

  return true;
}

IpcProcessId LinuxIpcMessageQueue::GetCurrentIpcProcessId() {
  return current_process_id_;
}

void LinuxIpcMessageQueue::SendWithCompletion(IpcProcessId dest_process_id,
                                              int message_type,
                                              IpcMessageData *message_data,
                                              SendCompletionCallback callback,
                                              void *callback_param) {
  // Does not support sending a message to itself.
  assert(dest_process_id != static_cast<IpcProcessId>(getpid()));

  if (die_) {
    delete message_data;
    return;
  }

  AddToOutboundList(dest_process_id, new OutboundIpcMessage(dest_process_id,
                                                            message_type,
                                                            message_data,
                                                            callback,
                                                            callback_param));
}

void LinuxIpcMessageQueue::SendToAll(int message_type,
                                     IpcMessageData *message_data,
                                     bool including_current_process) {
  // Not supported.
  assert(false);
}

void LinuxIpcMessageQueue::AddToOutboundList(
        IpcProcessId dest_process_id, OutboundIpcMessage *outbound_message) {
  assert(outbound_message);

  MutexLock locker(&thread_sync_mutex_);

  for (OutboundList::iterator iter = outbound_pendings_.begin();
       iter != outbound_pendings_.end();
       ++iter) {
    if (dest_process_id == iter->get()->dest_process_id()) {
      iter->get()->messages()->push_back(outbound_message);
      return;
    }
  }

  scoped_refptr<OutboundIpcMessageList> outbound_message_list;
  outbound_message_list = new OutboundIpcMessageList(dest_process_id);
  outbound_message_list->messages()->push_back(outbound_message);
  outbound_pendings_.push_back(outbound_message_list);

  // Signal the worker thread that there is outbound data to be processed.
  char ch = 'o';
  if (write(outbound_signaling_fds_[1], &ch, 1) < 1) {
    LOG(("Writing to the signaling pipe failed with errno=%d\n", errno));
  }
}

namespace {
Mutex g_system_queue_instance_lock;
scoped_ptr<LinuxIpcMessageQueue> g_system_queue_instance;
}

// static
IpcMessageQueue *IpcMessageQueue::GetSystemQueue() {
  MutexLock locker(&g_system_queue_instance_lock);
  if (!g_system_queue_instance.get()) {
    LinuxIpcMessageQueue *instance = new LinuxIpcMessageQueue();
    if (!instance->Init()) {
      LOG(("IpcMessageQueue initialization failed.\n"));
    }
    g_system_queue_instance.reset(instance);
  }
  return g_system_queue_instance.get();
}

// static
IpcMessageQueue *IpcMessageQueue::GetPeerQueue() {
  // Not supported.
  return NULL;
}

#ifdef USING_CCTESTS
void TestingIpcMessageQueue_GetCounters(IpcMessageQueueCounters *counters,
                                        bool reset) {
  // Not supported. Always set to zero.
  if (counters)
    memset(&counters, 0, sizeof(IpcMessageQueueCounters));
}
#endif  // USING_CCTESTS

#endif  // defined(LINUX) || defined(OS_MACOSX)
