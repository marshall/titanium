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

// TODO(mpcomplete): make this not win32 specific.

#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"
#endif
#include "gears/localserver/npapi/update_task_np.h"

#ifdef WINCE
typedef CMutexWince CATLMutex;
#else
typedef CMutex CATLMutex;
#endif

inline void GetUpdateTaskMutexName(int64 store_server_id,
                                   CStringW *mutex_name) {
  mutex_name->Format(L"NPUpdateTask:UpdateMutex:%I64d",
                     store_server_id);
}

void NPUpdateTask::Run() {
  // We use a global mutex to ensure that only one update task per captured
  // application executes at a time. Since there can be multiple IE
  // processes, this mutual exclusion needs to work across process
  // boundaries.
  CATLMutex global_mutex;
  CStringW mutex_name;
  GetUpdateTaskMutexName(store_.GetServerID(), &mutex_name);
  if (global_mutex.Create(NULL, FALSE, mutex_name)) {
    DWORD result = WaitForSingleObject(global_mutex.m_h, 0);
    if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
      UpdateTask::Run();
      ReleaseMutex(global_mutex.m_h);
      return;  // Note: early return
    } else {
      LOG16((L"NPUpdateTask - not running, another task is already running\n"));
    }
  } else {
    LOG16((L"NPUpdateTask - failed to start, unable to created mutex\n"));
  }
  NotifyTaskComplete(false);
}

// Returns true if an UpdateTask for the given store is running
// Platform-specific implementation. See declaration in update_task.h.
bool UpdateTask::IsUpdateTaskForStoreRunning(int64 store_server_id) {
  // We consider the existence of a mutex having right name as proof
  // positive of a running update task
  CATLMutex global_mutex;
  CStringW mutex_name;
  GetUpdateTaskMutexName(store_server_id, &mutex_name);
  return global_mutex.Open(SYNCHRONIZE , FALSE, mutex_name) ? true : false;
}

// Platform-specific implementation. See declaration in update_task.h.
UpdateTask *UpdateTask::CreateUpdateTask(BrowsingContext *browsing_context) {
  return new NPUpdateTask(browsing_context);
}
