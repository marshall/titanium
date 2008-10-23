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

#ifndef GEARS_LOCALSERVER_COMMON_UPDATE_TASK_SINGLE_PROCESS_H__
#define GEARS_LOCALSERVER_COMMON_UPDATE_TASK_SINGLE_PROCESS_H__

#if defined(BROWSER_WEBKIT) || defined(BROWSER_FF) || defined(OS_ANDROID)
#include "gears/localserver/common/update_task.h"

//------------------------------------------------------------------------------
// UpdateTaskSingleProcess
//------------------------------------------------------------------------------
class UpdateTaskSingleProcess : public UpdateTask {
 public:
  UpdateTaskSingleProcess(BrowsingContext *browsing_context)
      : UpdateTask(browsing_context) { }

 protected:
  // Overriden to ensure only one task per application runs at a time
  virtual void Run();

 private:
  static bool SetRunningTask(UpdateTaskSingleProcess *task);
  static void ClearRunningTask(UpdateTaskSingleProcess *task);
};
#endif  // defined(BROWSER_WEBKIT) || defined(BROWSER_FF) || defined(OS_ANDROID)

#endif  // GEARS_LOCALSERVER_COMMON_UPDATE_TASK_SINGLE_PROCESS_H__
