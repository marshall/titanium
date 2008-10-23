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

#include "gears/database2/commands.h"
#include "gears/database2/database2_common.h"
#include "gears/database2/result_set2.h"

void Database2BeginCommand::Execute(bool *has_results) {
  SetResult(connection()->Begin());
}

void Database2BeginCommand::HandleResults() {
  if (success()) {
    tx()->MarkOpen();
  } else {
    tx()->InvokeErrorCallback();
  }
}

void Database2AsyncExecuteCommand::Execute(bool *has_results) {
  results_.reset(new Database2BufferingRowHandler());
  SetResult(connection()->Execute(statement_->sql(),
      statement_->num_arguments(), statement_->arguments(), results_.get()));
  if (success() && !statement_->HasCallback()) {
    // execute next statement
    tx()->ExecuteNextStatement(NULL);
    *has_results = false;
  }
}

void Database2AsyncExecuteCommand::HandleResults() {
  // create JS objects from collected rows
  // create Database2ResultSet
  // results_.CopyTo(result_set);
  // stmt_->InvokeCallback(result_set);
  // if statement succeeded and callback failed, queue rollback op
  // else if statement failed and there is no callback, or callback did
  // not return false, queue rollback op
}

void Database2SyncExecuteCommand::Execute(bool *has_results) {
  // Since this is a sync operation, this method is invoked on the main thread,
  // thus we can create a Database2ResultSet instance here.
  scoped_refptr<ModuleEnvironment> module_environment;
  tx()->GetModuleEnvironment(&module_environment);
  if (!Database2ResultSet::Create(module_environment.get(), context_,
                                  &result_set_)) {
    return;
  }

  // Execute statement, using Database2ResultSet as Database2RowHandleInterface.
  bool result = connection()->Execute(statement_->sql(),
      statement_->num_arguments(), statement_->arguments(), result_set_.get());
  SetResult(result);
  if (!result) {
    connection()->Rollback();
    tx()->MarkClosed();
  }
}

void Database2SyncExecuteCommand::HandleResults() {
  if (!success()) {
    // throw SQL error as an exception
    context_->SetException(connection()->error_message());
    return;
  }

  context_->SetReturnValue(JSPARAM_MODULE, result_set_.get());
}

void Database2CommitCommand::Execute(bool *has_results) {
  bool result = connection()->Commit();
  SetResult(result);
  if (!result) {
    connection()->Rollback();
    return;
  }

  *has_results = tx()->HasSuccessCallback();
}

void Database2CommitCommand::HandleResults() {
  if (success()) {
    tx()->InvokeSuccessCallback();
    return;
  }

  tx()->InvokeErrorCallback();
}

void Database2RollbackCommand::Execute(bool *has_results) {
  // stub
}

void Database2RollbackCommand::HandleResults() {
  // stub
}

