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

#if defined(WIN32) && !defined(WINCE)

#include <deque>
#include <map>
#include <set>
#include <vector>
#include "gears/base/common/atomic_ops.h"
#include "gears/base/common/circular_buffer.h"
#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/ie/atl_headers.h"
#if !BROWSER_NONE
#include "gears/factory/factory_utils.h"  // for AppendBuildInfo
#endif
#include "third_party/linked_ptr/linked_ptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#include <atlsync.h>
#include <windows.h>

#ifdef USING_CCTESTS
// For testing
static Mutex g_counters_mutex;
static IpcMessageQueueCounters g_counters = {0};
#endif


//-----------------------------------------------------------------------------
// SharedMemory
// TODO(michaeln): move this to a seperate shared_memory_win32.h file?
//-----------------------------------------------------------------------------
class SharedMemory {
 public:
  SharedMemory() : mapping_(NULL) {}
  ~SharedMemory() { Close(); };
  bool Create(const char16 *name, int size);
  bool Open(const char16 *name);
  void Close();

  class MappedView {
   public:
    MappedView() : view_(NULL) {}
    ~MappedView() { CloseView(); }
    bool OpenView(SharedMemory *shared_memory, bool read_write);
    void CloseView();
    uint8 *view() { return view_; }
   private:
    uint8 *view_;
  };

  template<class T>
  class MappedViewOf : public MappedView {
   public:
    T *get() { return reinterpret_cast<T*>(view()); }
    T* operator->() {
      assert(view() != NULL);
      return reinterpret_cast<T*>(view());
    }
  };

 private:
  friend class MappedView;
  HANDLE mapping_;
};


bool SharedMemory::Create(const char16 *name, int size) {
  assert(mapping_ == NULL);
  mapping_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                               0, size, name);
  return mapping_ != NULL;
}


bool SharedMemory::Open(const char16 *name) {
  assert(mapping_ == NULL);
  mapping_ = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
                             FALSE, name);
  return mapping_ != NULL;
}


void SharedMemory::Close() {
  if (mapping_ != NULL) {
    CloseHandle(mapping_);
    mapping_ = NULL;
  }
}


bool SharedMemory::MappedView::OpenView(SharedMemory *shared_memory,
                                        bool read_write) {
  assert(!view_);
  DWORD access = FILE_MAP_READ;
  if (read_write) access |= FILE_MAP_WRITE;
  view_ = reinterpret_cast<uint8*>(
              ::MapViewOfFile(shared_memory->mapping_,
                              access, 0, 0, 0));
  return view_ != NULL;
}


void SharedMemory::MappedView::CloseView() {
  if (view_) {
    ::UnmapViewOfFile(view_);
    view_ = NULL;
  }  
}


//-----------------------------------------------------------------------------
// Kernel objects
// TODO(michaeln): On vista, low-integrity and medium-integrity processes
// cannot communicate using this mechanism. The low-integrity processes cannot
// access objects created by processes with elevated rights. The todo is to
// either create the kernel objects such that they only require low-rights
// in all cases, or to have separate low/med/high ipc registries.
//-----------------------------------------------------------------------------
static const char16 *kRegistryMutex = L"RegistryMutex";
static const char16 *kRegistryMemory = L"RegistryMemory";
static const char16 *kQueueWriteMutex = L"QWriteMutex";
static const char16 *kQueueDataAvailableEvent = L"QDataAvailableEvent";
static const char16 *kQueueSpaceAvailableEvent = L"QSpaceAvailableEvent";
static const char16 *kQueueIoBuffer = L"QIoBuffer";


void GetKernelObjectName(const char16 *object_type,
                         IpcProcessId process_id,
                         bool as_peer,
                         std::string16 *name) {
  name->clear();
  name->append(L"GearsIpc:");
  name->append(IntegerToString16(static_cast<int32>(process_id)));
  name->append(L":");
  name->append(object_type);
#if BROWSER_NONE
  assert(!as_peer);
#else
  if (as_peer) {
    name->append(L":");
    AppendBuildInfo(name);
  }
#endif
}


class IpcMutex : public ATL::CMutex {
 public:
  bool Create(const char16 *name) {
    return ATL::CMutex::Create(NULL, FALSE, name) ? true : false;
  }

  bool Open(const char16 *name) {
    return ATL::CMutex::Open(SYNCHRONIZE | MUTEX_MODIFY_STATE,
                             FALSE, name) ? true : false;
  }
};

// A replacement for ATL::CMutexLock with the addition of WasAbandoned
class IpcMutexLock {
public:
  IpcMutexLock(ATL::CMutex *mutex);
  ~IpcMutexLock();

  void Lock();
  void Unlock();
  
  // Returns true if the mutex was abandoned by the previous owner.
  // Only valid if the caller is the current owner of the mutex.
  bool WasAbandoned() { 
    assert(locked_);
    return was_abandoned_;
  }

 private:
  ATL::CMutex *mutex_;
  bool locked_;
  bool was_abandoned_;
  DISALLOW_EVIL_CONSTRUCTORS(IpcMutexLock);
};

inline IpcMutexLock::IpcMutexLock(ATL::CMutex *mutex)
    : mutex_(mutex), locked_(false), was_abandoned_(false) {
  Lock();
}

inline IpcMutexLock::~IpcMutexLock() {
  if (locked_) {
    Unlock();
  }
}

inline void IpcMutexLock::Lock() {
  assert(!locked_);
  DWORD rc = ::WaitForSingleObject(mutex_->m_h, INFINITE);
  was_abandoned_ = (rc == WAIT_ABANDONED);
  locked_ = true;
}

inline void IpcMutexLock::Unlock() {
  assert(locked_);
  mutex_->Release();
  locked_ = false;
}


class IpcEvent : public ATL::CEvent {
 public:
  // Creates a manual-reset event handle in the unsignalled state
  bool Create(const char16 *name) {
    return ATL::CEvent::Create(NULL, TRUE, FALSE, name) ? true : false;
  }

  bool Open(const char16 *name) {
    return ATL::CEvent::Open(SYNCHRONIZE | EVENT_MODIFY_STATE,
                             FALSE, name) ? true : false;
  }
};


class IpcBuffer {
 public:
  // We allocate an 8K bytes block of shared memory. The capacity
  // is slightly less due to overhead in our circular buffer,
  // an extra data element and storage for head and tail positions.
  static const int kCapacity = (8 * 1024) - sizeof(uint8) - (sizeof(int) * 2);

  // The structure stored in our shared memory, a cicular buffer.
  struct BufferFormat {
    int head;
    int tail;
    uint8 data[kCapacity + 1];  // must be one bigger than capacity
  };
  SharedMemory shared_memory_;
  SharedMemory::MappedViewOf<BufferFormat> mapped_buffer_;

  bool Create(IpcProcessId process_id, bool as_peer) {
    std::string16 name;
    GetKernelObjectName(kQueueIoBuffer, process_id, as_peer, &name);
    if (!shared_memory_.Create(name.c_str(), sizeof(BufferFormat)))
      return false;

    if (!mapped_buffer_.OpenView(&shared_memory_, true)) {
      shared_memory_.Close();
      return false; 
    }
    mapped_buffer_->head = 0;
    mapped_buffer_->tail = 0;
    return true;
  }

  bool Open(IpcProcessId process_id, bool as_peer) {
    std::string16 name;
    GetKernelObjectName(kQueueIoBuffer, process_id, as_peer, &name);
    return shared_memory_.Open(name.c_str());
  }

  BufferFormat *LazyOpenView() {
    if (!mapped_buffer_.get()) {
      mapped_buffer_.OpenView(&shared_memory_, true);
    }
    return mapped_buffer_.get();
  }

  // The 'head' position stored in shared memory is updated when
  // the transaction goes out of scope.
  class ReadTransaction {
   public:
    ReadTransaction(): was_read_(false), mapped_buffer_(NULL),
                       space_available_event_(NULL) {}

    ~ReadTransaction() {
      if (mapped_buffer_) {
        if (was_read_) {
          mapped_buffer_->head = circular_buffer_.head();
        }
        if (space_available()) {
          space_available_event_->Set();
        }
      }
    }

    bool Start(IpcBuffer *io_buffer, IpcEvent *space_available_event) {
      mapped_buffer_ = io_buffer->LazyOpenView();
      if (!mapped_buffer_)
        return false;
      circular_buffer_.set_buffer(mapped_buffer_->data, kCapacity + 1);
      if (!circular_buffer_.set_head(mapped_buffer_->head) ||
          !circular_buffer_.set_tail(mapped_buffer_->tail)) {
        assert(false);  // we have garbage in our shared memory block
        mapped_buffer_ = NULL; 
        return false;
      }
      space_available_event_ = space_available_event;
      return true;
    }

    size_t data_available() {
      assert(mapped_buffer_);
      return circular_buffer_.data_available();
    }

    size_t space_available() {
      assert(mapped_buffer_);
      return circular_buffer_.space_available();
    }
  
    void Read(void *data, size_t size) {
      assert(mapped_buffer_);
      assert(size <= data_available());
      if (size > 0) {
        circular_buffer_.read(data, size);
        was_read_ = true;
      }
    }

   private:
    bool was_read_;
    CircularBuffer circular_buffer_;
    BufferFormat *mapped_buffer_;
    IpcEvent *space_available_event_;
  };

  // The 'tail' position stored in shared memory is updated when
  // the transaction goes out of scope.
  class WriteTransaction {
   public:
    WriteTransaction(): was_written_(false), mapped_buffer_(NULL),
                        data_available_event_(NULL) {}

    ~WriteTransaction() {
      if (mapped_buffer_) {
        if (was_written_) {
          mapped_buffer_->tail = circular_buffer_.tail();
        }
        if (data_available()) {
          data_available_event_->Set();
        }
      }
    }

    bool Start(IpcBuffer *io_buffer, IpcEvent *data_available_event) {
      mapped_buffer_ = io_buffer->LazyOpenView();
      if (!mapped_buffer_)
        return false;
      circular_buffer_.set_buffer(mapped_buffer_->data, kCapacity + 1);
      if (!circular_buffer_.set_head(mapped_buffer_->head) ||
          !circular_buffer_.set_tail(mapped_buffer_->tail)) {
        assert(false);  // we have garbage in our shared memory block
        mapped_buffer_ = NULL; 
        return false;
      }
      data_available_event_ = data_available_event;
      return true;
    }

    size_t data_available() {
      assert(mapped_buffer_);
      return circular_buffer_.data_available();
    }

    size_t space_available() {
      assert(mapped_buffer_);
      return circular_buffer_.space_available();
    }
  
    void Write(const void *data, size_t size) {
      assert(mapped_buffer_);
      assert(size <= space_available());
      if (size > 0) {
        circular_buffer_.write(data, size);
        was_written_ = true;
      }
    }

#ifdef USING_CCTESTS
    // For testing
    void CommitWithoutSignalling() {
      if (was_written_) {
        mapped_buffer_->tail = circular_buffer_.tail();
      }
    }
#endif

   private:
    bool was_written_;
    CircularBuffer circular_buffer_;
    BufferFormat *mapped_buffer_;
    IpcEvent *data_available_event_;
  };
};


//-----------------------------------------------------------------------------
// IpcProcessRegistry
//-----------------------------------------------------------------------------
class IpcProcessRegistry {
 public:
  bool Open(bool as_peer);
  bool Add(IpcProcessId id);
  bool Remove(IpcProcessId id);
  void GetAll(std::vector<IpcProcessId> *list);

 private:
  static const IpcProcessId kInvalidProcessId = 0;
  static const int kMaxProcesses = 31;

  void Repair();

  // The structure stored in our shared memory
  struct Registry {
    int revision;
    IpcProcessId processes[kMaxProcesses];
  };

  IpcMutex mutex_;
  SharedMemory shared_memory_;
  SharedMemory::MappedViewOf<Registry> mapped_registry_;
  Registry cached_registry_;

#ifdef USING_CCTESTS
  bool Verify(bool check_for_current_process);
 public:
  // For testing
  void DieWhileHoldingRegistryLock();
  void SleepWhileHoldingRegistryLock();
#endif
};


bool IpcProcessRegistry::Open(bool as_peer) {
  std::string16 mutex_name;
  GetKernelObjectName(kRegistryMutex, 0, as_peer, &mutex_name);
  if (!mutex_.Create(mutex_name.c_str()))
    return false;

  IpcMutexLock lock(&mutex_);
  std::string16 shared_memory_name;
  GetKernelObjectName(kRegistryMemory, 0, as_peer, &shared_memory_name);
  if (shared_memory_.Open(shared_memory_name.c_str())) {
    if (!mapped_registry_.OpenView(&shared_memory_, true))
      return false;

    if (lock.WasAbandoned()) {
      Repair();
    }
    cached_registry_ = *mapped_registry_.get();
  } else {
    if (!shared_memory_.Create(shared_memory_name.c_str(), sizeof(Registry))) {
      return false;
    }
    if (!mapped_registry_.OpenView(&shared_memory_, true)) {
      return false;
    }
    cached_registry_ = *mapped_registry_.get();

    assert(mapped_registry_->revision == 0);
    assert(mapped_registry_->processes[0] == 0);
  }
#ifdef USING_CCTESTS
  Verify(false);
#endif
  return true;
}


bool IpcProcessRegistry::Add(IpcProcessId id) {
  assert(id != 0);
  assert(!Remove(id));

  if (!mapped_registry_.get())
    return false;

  IpcMutexLock lock(&mutex_);
  if (lock.WasAbandoned()) {
    Repair();
  }

  // Put this id in the first emtpy slot
  for (int i = 0; i < kMaxProcesses; ++i) {
    if (mapped_registry_->processes[i] == kInvalidProcessId) {
      mapped_registry_->revision += 1;
      mapped_registry_->processes[i] = id;
      return true;
    }
  }

  // No empty slots available, look for a dead process and replace
  // it with this id.
  for (int i = 0; i < kMaxProcesses; ++i) {
    ATL::CHandle process_handle;
    process_handle.Attach(::OpenProcess(SYNCHRONIZE, FALSE,
                                        mapped_registry_->processes[i]));
    if (!process_handle) {
      mapped_registry_->revision += 1;  // bump the revision number first
      mapped_registry_->processes[i] = id;
      return true;     
    }
  }

  // The array is full, we cannot add this process id
  assert(false);
  return false;
}


bool IpcProcessRegistry::Remove(IpcProcessId id) {
  assert(id != 0);

  if (!mapped_registry_.get())
    return false;

  IpcMutexLock lock(&mutex_);
  if (lock.WasAbandoned()) {
    Repair();
  }

  // Find our index
  int our_index = -1;
  for (int i = 0; i < kMaxProcesses; ++i) {
    if (mapped_registry_->processes[i] == id) {
      our_index = i;
      break;
    }
  }
  if (our_index == -1)
    return false;

  // Find the last valid index
  int last_valid_index = our_index;
  for (int i = our_index + 1; i < kMaxProcesses; ++i) {
    if (mapped_registry_->processes[i] == kInvalidProcessId)
      break;
    last_valid_index = i;
  }

  // Replace the value at our index with that of the last and shorten
  mapped_registry_->revision += 1;  // bump the revision number first
  if (last_valid_index != our_index) {
    mapped_registry_->processes[our_index] =
                          mapped_registry_->processes[last_valid_index];
    mapped_registry_->processes[last_valid_index] = kInvalidProcessId;
  } else {
    mapped_registry_->processes[our_index] = kInvalidProcessId;
  }

  return true;
}


void IpcProcessRegistry::GetAll(std::vector<IpcProcessId> *out) {
  out->clear();

  if (!mapped_registry_.get())
    return;

  if (cached_registry_.revision != mapped_registry_->revision) {
    IpcMutexLock lock(&mutex_);
    if (lock.WasAbandoned()) {
      Repair();
    }
    cached_registry_ = *mapped_registry_.get();
#ifdef USING_CCTESTS
    Verify(true);
#endif
  }

  for (int i = 0; i < kMaxProcesses; ++i) {
    if (cached_registry_.processes[i] == kInvalidProcessId)
      break;
    out->push_back(cached_registry_.processes[i]);
  }  
}


void IpcProcessRegistry::Repair() {
  int last_valid = -1;
  for (int i = 0; i < kMaxProcesses; ++i) {
    if (mapped_registry_->processes[i] == kInvalidProcessId)
      break;
    last_valid = i;
  }
  if (last_valid == -1)
    return;  // its empty

  // The Remove method can leave a duplicate entry at the last valid position
  // if the process terminates at just the wrong time.
  IpcProcessId possible_dup = mapped_registry_->processes[last_valid];
  for (int i = 0; i < last_valid; ++i) {
    if (mapped_registry_->processes[i] == possible_dup) {
      LOG(("IpcProcessRegistry::Repair - removing duplicate entry"));
      mapped_registry_->revision += 1;
      mapped_registry_->processes[last_valid] = kInvalidProcessId;
      return;
    }
  }
}

#ifdef USING_CCTESTS
bool IpcProcessRegistry::Verify(bool check_for_current_process) {
  IpcProcessId current_process_id = ::GetCurrentProcessId();
  std::set<IpcProcessId> unique;
  bool contains_current_process = false;
  int num_valid = 0;
  int i = 0;
  for (; i < kMaxProcesses; ++i) {
    if (cached_registry_.processes[i] == current_process_id)
      contains_current_process = true;
    if (cached_registry_.processes[i] == kInvalidProcessId)
      break;
    ++num_valid;
    unique.insert(cached_registry_.processes[i]);
  }
  for (++i; i < kMaxProcesses; ++i) {
    if (cached_registry_.processes[i] != kInvalidProcessId) {
      LOG(("IpcProcessRegistry::Verify failed, holes found"));
      assert(false);
      return false;
    }
  }
  if (unique.size() != num_valid) {
    LOG(("IpcProcessRegistry::Verify failed, duplicates found"));
    assert(false);
    return false;
  }
  if (check_for_current_process && !contains_current_process) {
    LOG(("IpcProcessRegistry::Verify failed, missing current process"));
    assert(false);
    return false;
  }
  return true;
}
#endif


//-----------------------------------------------------------------------------
// ShareableIpcMessage
//-----------------------------------------------------------------------------

class ShareableIpcMessage : public RefCounted {
 public:
  ShareableIpcMessage(IpcProcessId dest_process_id,
                      int ipc_message_type,
                      IpcMessageData *ipc_message_data,
                      IpcMessageQueue::SendCompletionCallback callback,
                      void *callback_param)
      : dest_process_id_(dest_process_id),
        ipc_message_type_(ipc_message_type),
        ipc_message_data_(ipc_message_data),
        callback_(callback),
        callback_param_(callback_param) {
  }

  IpcProcessId dest_process_id() const { return dest_process_id_; }

  int ipc_message_type() const { return ipc_message_type_; }

  IpcMessageData *ipc_message_data() const { return ipc_message_data_.get(); }

  std::vector<uint8> *serialized_message_data() {
    assert(ipc_message_data_.get());
    if (!serialized_message_data_.get()) {
      serialized_message_data_.reset(new std::vector<uint8>);
      Serializer serializer(serialized_message_data_.get());
      if (!serializer.WriteObject(ipc_message_data_.get())) {
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
  int ipc_message_type_;
  scoped_ptr<IpcMessageData> ipc_message_data_;
  scoped_ptr< std::vector<uint8> > serialized_message_data_;
  IpcMessageQueue::SendCompletionCallback callback_;
  void *callback_param_;
};


//-----------------------------------------------------------------------------
// InboundQueue and OutboundQueue
//-----------------------------------------------------------------------------

class Win32IpcMessageQueue;

static const int kTimeoutMs = 60000;

class QueueBase {
 protected:
  QueueBase(Win32IpcMessageQueue *owner) : owner_(owner) {}

  Win32IpcMessageQueue *owner_;

  // The set of kernel objects that comprise a queue
  IpcMutex write_mutex_;
  IpcEvent data_available_event_;
  IpcEvent space_available_event_;
  IpcBuffer io_buffer_;  // shared memory

  // The 'wire format' used to represent message packets in our io buffer
  struct MessagePacketHeader {
    IpcProcessId msg_source;
    int msg_type;
    int sequence_number;
    bool last_packet;
    int packet_size;

    MessagePacketHeader()
        : msg_source(0), msg_type(0), sequence_number(0),
          last_packet(true), packet_size(0) {}
    MessagePacketHeader(IpcProcessId source, int type, int size)
        : msg_source(source), msg_type(type), sequence_number(0),
          last_packet(true), packet_size(size) {}
  };
};


class OutboundQueue : public QueueBase {
 public:
  OutboundQueue(Win32IpcMessageQueue *owner)
    : QueueBase(owner),
      process_id_(0),
      has_write_mutex_(false),
      is_waiting_for_write_mutex_(false),
      is_waiting_for_space_available_(false),
      wait_start_time_(0),
      last_active_time_(0),
      in_progress_written_(0),
      in_progress_sequence_(0) {}

  ~OutboundQueue();

  bool Open(IpcProcessId process_id, bool as_peer);
  void AddMessageToQueue(ShareableIpcMessage *message);
  void WaitComplete(HANDLE handle, bool abandoned);
  bool AddWaitHandles(int64 now, std::vector<HANDLE> *handles);

  IpcProcessId process_id() const { return process_id_; }
  size_t pending_message_size() const { return pending_.size(); }
  ShareableIpcMessage *pending_message_at(size_t i) const {
    return pending_[i].get();
  }

 private:
  IpcProcessId process_id_;
  ATL::CHandle process_handle_;
  std::deque< scoped_refptr<ShareableIpcMessage> > pending_;
  bool has_write_mutex_;
  bool is_waiting_for_write_mutex_;
  bool is_waiting_for_space_available_;
  int64 wait_start_time_;
  int64 last_active_time_;
  scoped_refptr<ShareableIpcMessage> in_progress_message_;
  size_t in_progress_written_;
  int in_progress_sequence_;

  void WritePendingMessages();
  bool WriteOneMessage(ShareableIpcMessage *message, bool allow_large_message);
  bool WriteOnePacket(MessagePacketHeader *header,
                      const uint8 *msg_data,
                      bool allow_large_message);
  void MaybeWaitForWriteMutex();
  void WaitForSpaceAvailable();

#ifdef USING_CCTESTS
 public:
  // For testing
  void DieWhileHoldingWriteLock();
#endif
};

class InboundQueue : public QueueBase {
 public:
  InboundQueue(Win32IpcMessageQueue *owner) : QueueBase(owner) {}

  bool Create(IpcProcessId process_id, bool as_peer);
  void AddWaitHandles(int64 now, std::vector<HANDLE> *handles);
  void WaitComplete(HANDLE handle, bool abandoned);

 private:
  MessagePacketHeader last_packet_header_;
  std::vector<uint8> message_data_buffer_;

  void ReadAndDispatchMessages();
  bool ReadOneMessage(IpcProcessId *source, int *message_type,
                      IpcMessageData **message);
  bool ReadOnePacket(MessagePacketHeader *header);
};


//-----------------------------------------------------------------------------
// Win32IpcMessageQueue
//-----------------------------------------------------------------------------
class Win32IpcMessageQueue : public IpcMessageQueue {
 public:
  Win32IpcMessageQueue(bool as_peer)
    : as_peer_(as_peer), die_(false),
      current_process_id_(::GetCurrentProcessId()) {} 

  bool Init();

  // IpcMessageQueue overrides
  virtual IpcProcessId GetCurrentIpcProcessId();
  virtual void SendWithCompletion(IpcProcessId dest_process_id,
                                  int message_type,
                                  IpcMessageData *message_data,
                                  SendCompletionCallback callback,
                                  void *callback_param);
  virtual void SendToAll(int message_type,
                         IpcMessageData *message_data,
                         bool including_current_process);

  // Our worker thread's entry point and message loop
  static unsigned int __stdcall StaticThreadProc(void *start_data);
  void InstanceThreadProc(struct ThreadStartData *start_data);
  void Run();

  // Wait completion handling
  void WaitComplete(HANDLE handle, OutboundQueue *queue, bool abandoned);

  // OutboundQueue management
  OutboundQueue *GetOutboundQueue(IpcProcessId process_id);
  void RemoveOutboundQueue(OutboundQueue *queue);

  friend class InboundQueue;
  friend class OutboundQueue;

  bool as_peer_;
  bool die_;
  IpcProcessRegistry process_registry_;
  IpcProcessId current_process_id_;
  ATL::CHandle thread_;
  scoped_ptr<InboundQueue> inbound_queue_;
  std::map<IpcProcessId, linked_ptr<OutboundQueue> > outbound_queues_;
  Mutex thread_sync_mutex_;
  IpcEvent send_data_event_;
  std::vector< scoped_refptr<ShareableIpcMessage> > successful_messages_;
  std::vector< scoped_refptr<ShareableIpcMessage> > failed_messages_;
};



//-----------------------------------------------------------------------------
// OutboundQueue impl
//-----------------------------------------------------------------------------

OutboundQueue::~OutboundQueue() {
  pending_.clear();
  if (has_write_mutex_) {
    LOG(("OutboundQueue - releasing write_mutex_\n"));
    BOOL ok = ::ReleaseMutex(write_mutex_.m_h);
    assert(ok);
  }
  LOG(("OutboundQueue::~OutboundQueue %d\n", process_id_));
}


void OutboundQueue::MaybeWaitForWriteMutex() {
  if (!has_write_mutex_ && !is_waiting_for_write_mutex_) {
    LOG(("OutboundQueue::StartWaiting - write_mutex_\n"));
    is_waiting_for_write_mutex_ = true;
    wait_start_time_ = ::GetCurrentTimeMillis();
  }
}


void OutboundQueue::WaitForSpaceAvailable() {
  LOG(("OutboundQueue::StartWaiting - space_available_event_\n"));
  assert(has_write_mutex_);
  assert(!is_waiting_for_space_available_);
  is_waiting_for_space_available_ = true;
  wait_start_time_ = ::GetCurrentTimeMillis();
}


bool OutboundQueue::Open(IpcProcessId process_id, bool as_peer) {
  std::string16 write_mutex_name;
  GetKernelObjectName(kQueueWriteMutex, process_id, as_peer, &write_mutex_name);
  std::string16 data_available_event_name;
  GetKernelObjectName(kQueueDataAvailableEvent, process_id, as_peer,
                      &data_available_event_name);
  std::string16 space_available_event_name;
  GetKernelObjectName(kQueueSpaceAvailableEvent, process_id, as_peer,
                      &space_available_event_name);
  process_id_ = process_id;
  process_handle_.Attach(::OpenProcess(SYNCHRONIZE, FALSE, process_id));
  if (!process_handle_ ||
      !write_mutex_.Open(write_mutex_name.c_str()) ||
      !io_buffer_.Open(process_id, as_peer) ||
      !data_available_event_.Open(data_available_event_name.c_str()) ||
      !space_available_event_.Open(space_available_event_name.c_str())) {
    return false;
  }
  LOG(("OutboundQueue::Open %d\n", process_id_));
  return true;
}


void OutboundQueue::AddMessageToQueue(ShareableIpcMessage *message) {
  pending_.push_back(message);
  MaybeWaitForWriteMutex();

#ifdef USING_CCTESTS
  // For testing
  MutexLock lock(&g_counters_mutex);
  ++(g_counters.queued_outbound);
#endif
}


void OutboundQueue::WaitComplete(HANDLE handle, bool abandoned) {
  assert(process_handle_.m_h);

  last_active_time_ = ::GetCurrentTimeMillis();
  if (handle == write_mutex_.m_h) {
    LOG(("OutboundQueue::WaitComplete - write_mutex_ %s\n",
         abandoned ? "abandoned" : ""));
    is_waiting_for_write_mutex_ = false;
    has_write_mutex_ = true;
    WritePendingMessages();
  } else if (handle == space_available_event_.m_h) {
    LOG(("OutboundQueue::WaitComplete - space_available_event_ %s\n",
         abandoned ? "abandoned" : ""));
    assert(has_write_mutex_);
    space_available_event_.Reset();
    is_waiting_for_space_available_ = false;
    WritePendingMessages();
  } else if (handle == process_handle_.m_h) {
    LOG(("OutboundQueue::WaitComplete - process_handle_ %s\n",
         abandoned ? "abandoned" : ""));
    process_handle_.Close();
    owner_->process_registry_.Remove(process_id_);
  } else {
    assert(false);
    process_handle_.Close();
  }
}


bool OutboundQueue::AddWaitHandles(int64 now, std::vector<HANDLE> *handles) {
  if (!process_handle_.m_h)
    return false;

  if (!pending_.empty()) {
    assert(is_waiting_for_space_available_ ^ is_waiting_for_write_mutex_);
    if ((now - wait_start_time_) > kTimeoutMs)
      return false;

    if (is_waiting_for_space_available_)
      handles->push_back(space_available_event_.m_h);
    else if (is_waiting_for_write_mutex_)
      handles->push_back(write_mutex_.m_h);
  } else {
    if ((now - last_active_time_) > kTimeoutMs)
      return false;
  }
  handles->push_back(process_handle_.m_h);
  return true;
}


void OutboundQueue::WritePendingMessages() {
  assert(has_write_mutex_);

  // Write as many pending messages that will fit the available space
  int num_written = 0;
  bool allow_large_message = true;
  while (!pending_.empty()) {
    ShareableIpcMessage *message = pending_.front().get();
    if (WriteOneMessage(message, allow_large_message)) {
      owner_->successful_messages_.push_back(message);
    } else {
      break;
    }
    pending_.pop_front();
    ++num_written;
    allow_large_message = false;
  }

  if (!num_written && !pending_.empty()) {
    // Continue holding the write lock until we've written at least one message
    WaitForSpaceAvailable();
  } else {
    assert(!in_progress_message_);
    LOG(("OutboundQueue - releasing write_mutex_\n"));
    BOOL ok = ::ReleaseMutex(write_mutex_.m_h);
    assert(ok);
    has_write_mutex_ = false;

    if (!pending_.empty()) {
      // We release then reacquire the lock to avoid monopolizing a queue
      LOG(("OutboundQueue::StartWaiting - write_mutex_\n"));
      is_waiting_for_write_mutex_ = true;
      wait_start_time_ = ::GetCurrentTimeMillis();
    }
  }
}


bool OutboundQueue::WriteOneMessage(ShareableIpcMessage *message,
                                    bool allow_large_message) {
  assert(has_write_mutex_);
  assert(message);
  assert(message->serialized_message_data());

  // Form the packet we would like to send, the complete message
  MessagePacketHeader header(
      owner_->current_process_id_, message->ipc_message_type(),
      static_cast<int>(message->serialized_message_data()->size()));
  uint8 *msg_data = &message->serialized_message_data()->at(0);

  // If this message is already in progress, account for data we've already
  // sent in previous packets
  if (in_progress_message_) {
    assert(allow_large_message);
    assert(in_progress_message_ == message);
    msg_data += in_progress_written_;
    header.packet_size -= in_progress_written_;
    header.sequence_number = in_progress_sequence_ + 1;
  }

  if (!WriteOnePacket(&header, msg_data, allow_large_message))
    return false;

  if (header.last_packet) {
    in_progress_message_ = NULL;
    in_progress_written_ = 0;
    in_progress_sequence_ = 0;
  } else {
    // We could not send everything this time through
    assert(!in_progress_message_ || (message == in_progress_message_));
    in_progress_message_ = message;
    in_progress_written_ += header.packet_size;
    in_progress_sequence_ = header.sequence_number;
    return false;
  }

#ifdef USING_CCTESTS
    // For testing
    MutexLock lock(&g_counters_mutex);
    ++(g_counters.sent_outbound);
#endif

  LOG(("OutboundQueue - sent message to %d\n", process_id_));
  return true;
}

bool OutboundQueue::WriteOnePacket(MessagePacketHeader *header,
                                   const uint8 *msg_data,
                                   bool allow_large_message) {
  IpcBuffer::WriteTransaction writer;
  if (!writer.Start(&io_buffer_, &data_available_event_)) {
    process_handle_.Close();
    return false;
  }

  size_t space_needed = sizeof(MessagePacketHeader) + header->packet_size;
  if (space_needed > IpcBuffer::kCapacity) {
    if (!allow_large_message)
      return false;
    space_needed = IpcBuffer::kCapacity;
    header->packet_size = IpcBuffer::kCapacity - sizeof(MessagePacketHeader);
    header->last_packet = false;
  }

  if (space_needed > writer.space_available()) {
    return false;
  }
  
  writer.Write(header, sizeof(MessagePacketHeader));
  if (header->packet_size > 0)
    writer.Write(msg_data, header->packet_size);
  return true;
}


//-----------------------------------------------------------------------------
// InboundQueue impl
//-----------------------------------------------------------------------------

bool InboundQueue::Create(IpcProcessId process_id, bool as_peer) {
  std::string16 write_mutex_name;
  GetKernelObjectName(kQueueWriteMutex, process_id, as_peer, &write_mutex_name);
  std::string16 data_available_event_name;
  GetKernelObjectName(kQueueDataAvailableEvent, process_id, as_peer,
                      &data_available_event_name);
  std::string16 space_available_event_name;
  GetKernelObjectName(kQueueSpaceAvailableEvent, process_id, as_peer,
                      &space_available_event_name);
  if (!write_mutex_.Create(write_mutex_name.c_str()) ||
      !io_buffer_.Create(process_id, as_peer) ||
      !data_available_event_.Create(data_available_event_name.c_str()) ||
      !space_available_event_.Create(space_available_event_name.c_str())) {
    return false;
  }
  return true;
}

void InboundQueue::AddWaitHandles(int64 now, std::vector<HANDLE> *handles) {
  // We perpetually wait for new data being dropped into our queue
  handles->push_back(data_available_event_.m_h);
}

void InboundQueue::WaitComplete(HANDLE handle, bool abandoned) {
  LOG(("InboundQueue::WaitComplete - data_available_event_ %s\n",
       abandoned ? "abandoned" : ""));
  assert(handle == data_available_event_.m_h);
  assert(!abandoned);
  data_available_event_.Reset();
  ReadAndDispatchMessages();
}


void InboundQueue::ReadAndDispatchMessages() {
  IpcProcessId source_process_id;
  int message_type;
  IpcMessageData *message;
  while (ReadOneMessage(&source_process_id, &message_type, &message)) {
#ifdef USING_CCTESTS
    {
      // For testing
      MutexLock lock(&g_counters_mutex);
      ++(g_counters.read_inbound);
      if (message)
        ++(g_counters.dispatched_inbound);
    }
#endif
    if (message) {
      LOG(("InboundQueue - received msg from %d\n", source_process_id));
      owner_->CallRegisteredHandler(source_process_id, message_type, message);
      delete message;
    } else {
      LOG(("InboundQueue - unable to deserialize message_type %d from %d\n",
           message_type, source_process_id));
    }
  } 
}


bool InboundQueue::ReadOneMessage(IpcProcessId *source_process_id,
                                  int *message_type,
                                  IpcMessageData **message) {
  *source_process_id = 0;
  *message_type = 0;
  *message = NULL;

  MessagePacketHeader header;
  while (ReadOnePacket(&header)) {
    if (!header.last_packet) {
      continue;
    }

    if (message_data_buffer_.size() > 0) {
      Deserializer deserializer(&message_data_buffer_[0],
                                message_data_buffer_.size());
      deserializer.CreateAndReadObject(message);
      message_data_buffer_.clear();
    }
    *source_process_id = header.msg_source;
    *message_type = header.msg_type;
    return true;
  }
  return false;
}

bool InboundQueue::ReadOnePacket(MessagePacketHeader *header) {
  assert(header);
  IpcBuffer::ReadTransaction reader;
  if (!reader.Start(&io_buffer_, &space_available_event_)) {
    // Should not occur since we have already opened the file mapping view
    assert(false);
    owner_->die_ = true;
    return false;
  }
  if (!reader.data_available()) {
    return false;
  }

  // Read the packet header from the buffer
  assert(reader.data_available() >= sizeof(MessagePacketHeader));
  reader.Read(header, sizeof(MessagePacketHeader));

  if (header->sequence_number == 0) {
    // Start of a new message
    message_data_buffer_.clear();
  } else {
    // The next packet in a long message, append to our existing message data
    assert(!last_packet_header_.last_packet);
    assert(header->msg_source == last_packet_header_.msg_source);
    assert(header->msg_type == last_packet_header_.msg_type);
    assert(header->sequence_number == last_packet_header_.sequence_number + 1);
  }

  // Read the packet data
  if (header->packet_size > 0) {
    if (reader.data_available() < static_cast<size_t>(header->packet_size)) {
      assert(false); // We have garbage in our shared memory block
      owner_->die_ = true;
      return false;
    }
    size_t current_size = message_data_buffer_.size();
    message_data_buffer_.resize(current_size + header->packet_size);
    reader.Read(&message_data_buffer_[current_size], header->packet_size);
  }

  last_packet_header_ = *header;

  return true;
}


//-----------------------------------------------------------------------------
// An implementation of the IpcMessageQueue API for Win32
//-----------------------------------------------------------------------------

static Mutex g_peer_queue_instance_lock;
static Win32IpcMessageQueue * volatile g_peer_queue_instance = NULL;

// static
IpcMessageQueue *IpcMessageQueue::GetPeerQueue() {
#if defined(BROWSER_IE) && !defined(WINCE)
  if (!g_peer_queue_instance) {
    MutexLock locker(&g_peer_queue_instance_lock);
    if (!g_peer_queue_instance) {
      Win32IpcMessageQueue *instance = new Win32IpcMessageQueue(true);
      if (!instance->Init()) {
        LOG(("IpcMessageQueue initialization failed.\n"));
        instance->die_ = true;
      }
      g_peer_queue_instance = instance;
    }
  }
  return g_peer_queue_instance;
#else
  return NULL;
#endif  // defined(BROWSER_IE) && !defined(WINCE)
}

static Mutex g_system_queue_instance_lock;
static Win32IpcMessageQueue * volatile g_system_queue_instance = NULL;

// static
IpcMessageQueue *IpcMessageQueue::GetSystemQueue() {
  if (!g_system_queue_instance) {
    MutexLock locker(&g_system_queue_instance_lock);
    if (!g_system_queue_instance) {
      Win32IpcMessageQueue *instance = new Win32IpcMessageQueue(false);
      if (!instance->Init()) {
        LOG(("IpcMessageQueue initialization failed.\n"));
        instance->die_ = true;
      }
      g_system_queue_instance = instance;
    }
  }
  return g_system_queue_instance;
}


IpcProcessId Win32IpcMessageQueue::GetCurrentIpcProcessId() {
  return current_process_id_;
}


void Win32IpcMessageQueue::SendToAll(int ipc_message_type,
                                     IpcMessageData *ipc_message_data,
                                     bool including_self) {
  // SendToAll is not supported for the system queue.
  assert(as_peer_);

  MutexLock thread_sync_lock(&thread_sync_mutex_);

  if (die_) {
    delete ipc_message_data;
    return;
  }

  scoped_refptr<ShareableIpcMessage> shareable_message;
  shareable_message = new ShareableIpcMessage(0,
                                              ipc_message_type,
                                              ipc_message_data,
                                              NULL,
                                              NULL);

  std::vector<IpcProcessId> processes;
  process_registry_.GetAll(&processes);
  bool added_to_queue = false;
  for (std::vector<IpcProcessId>::iterator iter = processes.begin();
       iter != processes.end(); iter++) {
    if (including_self || *iter != current_process_id_) {
      OutboundQueue *outbound_queue = GetOutboundQueue(*iter);
      if (outbound_queue) {
        outbound_queue->AddMessageToQueue(shareable_message.get());
        added_to_queue = true;
      } else if (current_process_id_ != *iter) {
        process_registry_.Remove(*iter);
        LOG(("Removing dead processes from registry, %d\n", *iter));
      }
    }
  }

  if (added_to_queue)
    send_data_event_.Set();

#ifdef USING_CCTESTS
  // For testing
  MutexLock lock(&g_counters_mutex);
  ++(g_counters.send_to_all);
#endif
}


void Win32IpcMessageQueue::SendWithCompletion(IpcProcessId dest_process_id,
                                              int ipc_message_type,
                                              IpcMessageData *ipc_message_data,
                                              SendCompletionCallback callback,
                                              void *callback_param) {
  MutexLock thread_sync_lock(&thread_sync_mutex_);

  if (die_) {
    delete ipc_message_data;
    return;
  }

  scoped_refptr<ShareableIpcMessage> shareable_message;
  shareable_message = new ShareableIpcMessage(dest_process_id,
                                              ipc_message_type,
                                              ipc_message_data,
                                              callback,
                                              callback_param);

  OutboundQueue *outbound_queue = GetOutboundQueue(dest_process_id);
  if (outbound_queue) {
    outbound_queue->AddMessageToQueue(shareable_message.get());
  } else {
    failed_messages_.push_back(shareable_message.get());
  }
  send_data_event_.Set();

#ifdef USING_CCTESTS
  // For testing
  MutexLock lock(&g_counters_mutex);
  ++(g_counters.send_to_one);
#endif
}


struct ThreadStartData {
  ThreadStartData(Win32IpcMessageQueue *self) 
    : started_signal_(false),
      started_successfully_(false),
      self(self) {}
  Mutex started_mutex_;
  bool started_signal_;
  bool started_successfully_;
  Win32IpcMessageQueue *self;
};


bool Win32IpcMessageQueue::Init() {
  assert(!die_);
  ThreadStartData start_data(this);
  unsigned int not_used;
  thread_.Attach(reinterpret_cast<HANDLE>(_beginthreadex(
                                              NULL, 0, StaticThreadProc,
                                              &start_data, 0, &not_used)));
  if (!thread_) {
    return false;
  }
  MutexLock locker(&start_data.started_mutex_);
  start_data.started_mutex_.Await(Condition(&start_data.started_signal_));
  return start_data.started_successfully_;
}


// static
unsigned int __stdcall Win32IpcMessageQueue::StaticThreadProc(void *param) {
  if (FAILED(CoInitializeEx(NULL, GEARS_COINIT_THREAD_MODEL))) {
    LOG(("Win32IpcMessageQueue - failed to co-initialize new thread.\n"));
  }
  ThreadStartData *start_data = reinterpret_cast<ThreadStartData*>(param);
  Win32IpcMessageQueue* self = start_data->self;
  self->InstanceThreadProc(start_data);
  CoUninitialize();
  return 0;
}


void Win32IpcMessageQueue::InstanceThreadProc(ThreadStartData *start_data) {
  {
    MutexLock locker(&start_data->started_mutex_);
    inbound_queue_.reset(new InboundQueue(this));
    if (!inbound_queue_->Create(current_process_id_, as_peer_) ||
        !process_registry_.Open(as_peer_) ||
        !process_registry_.Add(current_process_id_) ||
        !send_data_event_.Create(NULL)) {
      start_data->started_signal_ = true;
      start_data->started_successfully_ = false;
      return;
    }
    start_data->started_signal_ = true;
    start_data->started_successfully_ = true;
  }

  Run();

  MutexLock lock(&thread_sync_mutex_);
  assert(die_);
  outbound_queues_.clear();
  inbound_queue_.reset(NULL);
  process_registry_.Remove(current_process_id_);
}


void Win32IpcMessageQueue::Run() {
  std::vector<HANDLE> wait_handles;
  std::vector<OutboundQueue*> waiting_queues;

  bool reset_send_data_event = false;
  while (!die_) {
    std::vector< scoped_refptr<ShareableIpcMessage> > successful_messages;
    std::vector< scoped_refptr<ShareableIpcMessage> > failed_messages;
    {
      MutexLock lock(&thread_sync_mutex_);
      
      if (reset_send_data_event) {
        reset_send_data_event = false;
        send_data_event_.Reset();
      }

      // Build up the array of handles we need to wait on
      wait_handles.clear();
      waiting_queues.clear();
      int64 now = ::GetCurrentTimeMillis();

      // Need to wait for the event to signal the rebuilding of the wait handles
      // due to something to be sent.
      wait_handles.push_back(send_data_event_.m_h);
      waiting_queues.push_back(NULL);

      // The inbound queue perpetually waits for data available.
      inbound_queue_->AddWaitHandles(now, &wait_handles);
      assert(wait_handles.size() == 2);
      waiting_queues.push_back(NULL);  // NULL indicates the inbound queue

      // Each outbound queue perpetually waits on the process handle, and
      // when messages are pending, either the write mutex or space available.
      std::map<IpcProcessId, linked_ptr<OutboundQueue> >::iterator iter;
      iter = outbound_queues_.begin();
      while(iter != outbound_queues_.end()) {
        OutboundQueue *queue = iter->second.get();
        ++iter;  // advance the iterator prior to removal from the set
        if (queue->AddWaitHandles(now, &wait_handles)) {
          while (waiting_queues.size() < wait_handles.size()) {
            waiting_queues.push_back(queue);
          }
        } else {
          // Remove any handles added above, then remove the queue
          wait_handles.resize(waiting_queues.size());
          RemoveOutboundQueue(queue);
        }
      }

      successful_messages.swap(successful_messages_);
      failed_messages.swap(failed_messages_);
    }

    // Inform the result listener about the message sending statuses.
    for (size_t i = 0; i < successful_messages.size(); ++i) {
      if (successful_messages[i]->callback()) {
        (*successful_messages[i]->callback())(
            true,
            successful_messages[i]->dest_process_id(),
            successful_messages[i]->ipc_message_type(),
            successful_messages[i]->ipc_message_data(),
            successful_messages[i]->callback_param());
      }
    }
    successful_messages.clear();

    for (size_t i = 0; i < failed_messages.size(); ++i) {
      if (failed_messages[i]->callback()) {
        (*failed_messages[i]->callback())(
            false,
            failed_messages[i]->dest_process_id(),
            failed_messages[i]->ipc_message_type(),
            failed_messages[i]->ipc_message_data(),
            failed_messages[i]->callback_param());
      }
    }
    failed_messages.clear();

    const DWORD kMaxWaitHandles = MAXIMUM_WAIT_OBJECTS;
    assert(kMaxWaitHandles == 64);
    DWORD num_wait_handles = wait_handles.size();
    if (num_wait_handles > kMaxWaitHandles)
      num_wait_handles = kMaxWaitHandles;

    LOG(("MsgWaitForMultipleObjectsEx %d handles\n", num_wait_handles));
    DWORD rv = WaitForMultipleObjectsEx(
                   num_wait_handles, &wait_handles[0],
                   FALSE, kTimeoutMs, TRUE);

    if ((rv >= WAIT_OBJECT_0) &&
        (rv < WAIT_OBJECT_0 + num_wait_handles)) {
      int index = rv - WAIT_OBJECT_0;
      // If it is signalled due to something to be sent, continue the loop to
      // rebuild the wait handles.
      if (index == 0) {
        reset_send_data_event = true;
        continue;
      }
      WaitComplete(wait_handles[index], 
                   waiting_queues[index],
                   false);
    } else if ((rv >= WAIT_ABANDONED_0) && 
               (rv < WAIT_ABANDONED_0 + num_wait_handles)) {
      int index = rv - WAIT_ABANDONED_0;
      WaitComplete(wait_handles[index], 
                   waiting_queues[index],
                   true);
    } else if (rv == WAIT_TIMEOUT) {
      LOG(("WAIT_TIMEOUT\n"));
    } else if (rv == WAIT_IO_COMPLETION) {
      LOG(("WAIT_IO_COMPLETION\n"));
    } else if (rv == WAIT_FAILED) {
      LOG(("WAIT_FAILED\n", rv));
      die_ = true;
    } else {
      LOG(("Wait finished with unknown return value, %d\n", rv));
      die_ = true;
    }
  }
  LOG(("Win32IpcMessageQueue dying\n"));
}


void Win32IpcMessageQueue::WaitComplete(HANDLE handle,
                                        OutboundQueue *queue,
                                        bool abandoned) {
  LOG(("WaitComplete %d , %d %s\n", handle,
                                    queue ? queue->process_id() : 0,
                                    abandoned ? "abandoned" : ""));
  if (!queue) {
    // NULL indicates the inbound queue
    inbound_queue_->WaitComplete(handle, abandoned);
  } else {
    MutexLock lock(&thread_sync_mutex_);
    queue->WaitComplete(handle, abandoned);
  }
}


OutboundQueue *Win32IpcMessageQueue::GetOutboundQueue(
                                         IpcProcessId process_id) {
  std::map<IpcProcessId, linked_ptr<OutboundQueue> >::iterator iter;
  iter = outbound_queues_.find(process_id);
  if (iter != outbound_queues_.end()) {
    return iter->second.get();
  }
  OutboundQueue *queue = new OutboundQueue(this);
  if (!queue->Open(process_id, as_peer_)) {
    LOG(("OutboundQueue::Open failed for process %d\n", process_id));
    delete queue;
    return NULL;
  }
  outbound_queues_[process_id] = linked_ptr<OutboundQueue>(queue);
  return queue;
}


void Win32IpcMessageQueue::RemoveOutboundQueue(OutboundQueue *queue) {
  // Save the message being removed from the queue so that we can inform the
  // result listener about the failure.
  for (size_t i = 0; i < queue->pending_message_size(); ++i) {
    failed_messages_.push_back(queue->pending_message_at(i));
  }

  outbound_queues_.erase(queue->process_id());
}



#ifdef USING_CCTESTS

void TestingIpcMessageQueueWin32_GetAllProcesses(
        bool as_peer, std::vector<IpcProcessId> *processes) {
  IpcProcessRegistry registry;
  registry.Open(as_peer);
  registry.GetAll(processes);
}

void TestingIpcMessageQueueWin32_SleepWhileHoldingRegistryLock(
        IpcMessageQueue *ipc_message_queue) {
  // Must be called on the IPC IO thread.
  Win32IpcMessageQueue *win32_ipc_message_queue =
      reinterpret_cast<Win32IpcMessageQueue*>(ipc_message_queue);
  win32_ipc_message_queue->process_registry_.SleepWhileHoldingRegistryLock();
}

void IpcProcessRegistry::SleepWhileHoldingRegistryLock() {
  IpcMutexLock lock(&mutex_);
  Sleep(INFINITE);
}

void TestingIpcMessageQueueWin32_DieWhileHoldingRegistryLock(
        IpcMessageQueue *ipc_message_queue) {
  // Must be called on the IPC IO thread.
  Win32IpcMessageQueue *win32_ipc_message_queue =
      reinterpret_cast<Win32IpcMessageQueue*>(ipc_message_queue);
  win32_ipc_message_queue->process_registry_.DieWhileHoldingRegistryLock();
}


void IpcProcessRegistry::DieWhileHoldingRegistryLock() {
  IpcMutexLock lock(&mutex_);
  TerminateProcess(GetCurrentProcess(), 3);
}


void TestingIpcMessageQueueWin32_DieWhileHoldingWriteLock(
        IpcMessageQueue *ipc_message_queue, IpcProcessId id) {
  // Must be called on the IPC IO thread.
  Win32IpcMessageQueue *win32_ipc_message_queue =
      reinterpret_cast<Win32IpcMessageQueue*>(ipc_message_queue);
  OutboundQueue *queue = win32_ipc_message_queue->GetOutboundQueue(id);
  assert(queue);
  queue->DieWhileHoldingWriteLock();
}


void OutboundQueue::DieWhileHoldingWriteLock() {
  IpcMutexLock lock(&write_mutex_);
  IpcBuffer::WriteTransaction writer;
  if (!writer.Start(&io_buffer_, &data_available_event_) ||
      writer.space_available() < sizeof(MessagePacketHeader)) {
    TerminateProcess(GetCurrentProcess(), 3);
  }

  // We leave a message packet with last_packet set to false
  // in the queue in this case as well
  MessagePacketHeader header(
      owner_->current_process_id_, kIpcQueue_TestMessage, 0);
  header.last_packet = false;
  writer.Write(&header, sizeof(header));
  writer.CommitWithoutSignalling();

  TerminateProcess(GetCurrentProcess(), 3);
}

void TestingIpcMessageQueue_GetCounters(IpcMessageQueueCounters *counters,
                                        bool reset) {
  MutexLock lock(&g_counters_mutex);
  if (counters)
    *counters = g_counters;
  if (reset)
    memset(&g_counters, 0, sizeof(g_counters));
}

#endif

#endif  // defined(WIN32) && !defined(WINCE)
