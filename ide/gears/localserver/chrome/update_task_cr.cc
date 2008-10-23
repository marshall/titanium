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


#include "gears/localserver/chrome/update_task_cr.h"

#include "gears/base/chrome/browsing_context_cr.h"
#include "gears/base/common/stopwatch.h"

// We use a global map to ensure that only one update task per captured
// application executes at a time. Since Chrome only runs us in a single plugin
// process, this mutual exclusion only needs to work across threads rather
// than across processes.
typedef std::map< int64, NPUpdateTask* > UpdateTaskMap;
static Mutex running_tasks_mutex;
static UpdateTaskMap running_tasks;

static bool IsUpdateTaskProcess() {
  if (CP::gears_in_renderer())
    return CP::IsBrowserProcess();
  else
    return CP::IsPluginProcess();
}

void NPUpdateTask::Run() {
  assert(IsUpdateTaskProcess());

  if (SetRunningTask(this)) {
    UpdateTask::Run();
    ClearRunningTask(this);
  } else {
    LOG(("UpdateTask - not running, another task is already running\n"));
    NotifyTaskComplete(false);
  }

  CP::DecrementProcessKeepAliveCount();
}

// static 
bool NPUpdateTask::SetRunningTask(NPUpdateTask *task) {
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
void NPUpdateTask::ClearRunningTask(NPUpdateTask *task) {
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
  return new NPUpdateTask(context);
}

// Virtual method override for chrome
bool NPUpdateTask::StartUpdate(ManagedResourceStore *store) {
  if (IsUpdateTaskProcess()) {
    if (!UpdateTask::StartUpdate(store)) {
      return false;
    }
    CP::IncrementProcessKeepAliveCount();
    return true;
  } else {
    // Send a message to the UpdateTask process to perform the auto update
    CRBrowsingContext *cr_context =
        static_cast<CRBrowsingContext*>(browsing_context_.get());
    CPBrowsingContext context = 0;
    if (cr_context)
      context = cr_context->context;
    else
      LOG16((STRING16(
          L"Warning: NPUpdateTask sending message with no context\n")));

    AutoUpdateMessage auto_update_message(store->GetServerID(), context);
    auto_update_message.Send();
    return false;  // this instance of the task does not actually run
  }
}

void AutoUpdateMessage::OnMessageReceived() {
  assert(IsUpdateTaskProcess());
  scoped_refptr<CRBrowsingContext> context(new CRBrowsingContext(context_));
  scoped_ptr<UpdateTask> task(UpdateTask::CreateUpdateTask(context.get()));
  // Since auto updates can also be initiated by the plugin process,
  // we run thru the rate limiting logic here before kicking it off.
  if (!task.get()->MaybeAutoUpdate(store_id_)) {
    return;
  }
  task.release()->DeleteWhenDone();
}
