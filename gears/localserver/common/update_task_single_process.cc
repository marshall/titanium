// Copyright 2006, Google Inc.
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

#if defined(BROWSER_WEBKIT) || defined(BROWSER_FF) || defined(OS_ANDROID)
#include <assert.h>
#include <map>
#include "gears/base/common/mutex.h"
#include "gears/localserver/common/update_task_single_process.h"

// We use a global map to ensure that only one update task per captured
// application executes at a time. Since We're doing this for browsers
// that only runs in a single process, this mutual exclusion only needs to 
// work across threads rather than across processes.
typedef std::map< int64, UpdateTaskSingleProcess* > UpdateTaskMap;
static Mutex running_tasks_mutex;
static UpdateTaskMap running_tasks;

void UpdateTaskSingleProcess::Run() {
  if (SetRunningTask(this)) {
    UpdateTask::Run();
    ClearRunningTask(this);
  } else {
    LOG(("UpdateTaskSingleProcess - not running, another task is already" \
         " running\n"));
    NotifyTaskComplete(false);
  }
}

// static 
bool UpdateTaskSingleProcess::SetRunningTask(UpdateTaskSingleProcess *task) {
  MutexLock lock(&running_tasks_mutex);
  int64 key = task->store_.GetServerID();
  UpdateTaskMap::iterator found = running_tasks.find(key);
  if (found != running_tasks.end()) {
    // An existing entry indicates another task is running, return false
    // to prevent Run from proceeding.
    return false;
  } else {
    running_tasks[key] = task;
    return true;
  }
}

// static
void UpdateTaskSingleProcess::ClearRunningTask(UpdateTaskSingleProcess *task) {
  MutexLock lock(&running_tasks_mutex);
  int64 key = task->store_.GetServerID();
  UpdateTaskMap::iterator found = running_tasks.find(key);
  if (found != running_tasks.end()) {
    assert(found->second == task);
    running_tasks.erase(found);
  }
}

// Platform-specific implementation. See declaration in update_task.h.
// Returns true if an UpdateTask for the given store is running
bool UpdateTask::IsUpdateTaskForStoreRunning(int64 store_server_id) {
  MutexLock lock(&running_tasks_mutex);
  UpdateTaskMap::iterator found = running_tasks.find(store_server_id);
  return (found != running_tasks.end());
}

// Platform-specific implementation. See declaration in update_task.h.
UpdateTask *UpdateTask::CreateUpdateTask(BrowsingContext *context) {
  return new UpdateTaskSingleProcess(context);
}

#endif  // defined(BROWSER_WEBKIT) || defined(BROWSER_FF)
