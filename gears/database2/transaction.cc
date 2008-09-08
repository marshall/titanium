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

#include "gears/database2/transaction.h"

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/database2/commands.h"
#include "gears/database2/database2.h"
#include "gears/database2/database2_common.h"
#include "gears/database2/statement.h"

DECLARE_GEARS_WRAPPER(GearsDatabase2Transaction);

template<>
void Dispatcher<GearsDatabase2Transaction>::Init() {
  RegisterMethod("executeSql", &GearsDatabase2Transaction::ExecuteSql);
}


// static
bool GearsDatabase2Transaction::Create(
    ModuleEnvironment *module_environment,
    JsCallContext *context,
    Database2Connection *connection,
    Database2Interpreter *interpreter,
    JsRootedCallback *callback,
    JsRootedCallback *error_callback,
    JsRootedCallback *success_callback,
    scoped_refptr<GearsDatabase2Transaction> *instance) {
  assert(instance);
  if (!CreateModule<GearsDatabase2Transaction>(module_environment, context,
                                               instance)) {
    return false;
  }

  GearsDatabase2Transaction *tx = instance->get();
  tx->connection_.reset(connection);
  tx->interpreter_.reset(interpreter);
  // set callbacks
  tx->callback_.reset(callback);
  tx->error_callback_.reset(error_callback);
  tx->success_callback_.reset(success_callback);
  // register unload handler
  tx->unload_monitor_.reset(new JsEventMonitor(module_environment->js_runner_,
                                               JSEVENT_UNLOAD, tx));
  return true;
}

void GearsDatabase2Transaction::Start() {
  // queue operation to begin transaction
  interpreter_->Run(new Database2BeginCommand(this));
}

void GearsDatabase2Transaction::InvokeCallback() {
  assert(callback_.get());
  // prepare to return transaction
  JsParamToSend send_argv[] = {
    { JSPARAM_MODULE, static_cast<ModuleImplBaseClass *>(this) }
  };

  GetJsRunner()->InvokeCallback(callback_.get(), ARRAYSIZE(send_argv),
                                send_argv, NULL);
}

void GearsDatabase2Transaction::ExecuteSql(JsCallContext *context) {
  std::string16 sql_statement;
  JsArray temp_sql_arguments;
  JsRootedCallback *temp_callback = NULL;
  JsRootedCallback *temp_error_callback = NULL;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &sql_statement },
    { JSPARAM_OPTIONAL, JSPARAM_ARRAY, &temp_sql_arguments },
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &temp_callback },
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &temp_error_callback }
  };

  int argc = context->GetArguments(ARRAYSIZE(argv), argv);
  JsArray *sql_arguments = &temp_sql_arguments;
  scoped_ptr<JsRootedCallback> callback(temp_callback);
  scoped_ptr<JsRootedCallback> error_callback(temp_error_callback);
  if (context->is_exception_set()) return;

  if (!is_open_) {
    context->SetException(kTransactionClosed);
    return;
  }

  // if any of the arguments are not supplied or null, send them to statement
  // factory as NULL
  if (argc < 2 || JsTokenIsNullOrUndefined(sql_arguments->token())) {
    sql_arguments = NULL;
  }
  if (argc < 3 || JsTokenIsNullOrUndefined(callback->token())) {
    callback.reset(NULL);
  }
  if (argc < 4 || JsTokenIsNullOrUndefined(error_callback->token())) {
    error_callback.reset(NULL);
  }

  Database2Statement *statement;
  if (!Database2Statement::Create(sql_statement, sql_arguments,
      callback.get(), error_callback.get(), &statement)) {
    context->SetException(GET_INTERNAL_ERROR_MESSAGE());
    return;
  }

  bool first;
  // TODO(dimitri.glazkov): ideally, if the queue is empty prior to this
  // call, we should avoid pushing/popping the statement
  statement_queue_.Push(statement, &first);
  if (first) {
    ExecuteNextStatement(context);
  }
  // otherwise, the statement will be executed after the previous statement
  // in queue
}

void GearsDatabase2Transaction::ExecuteNextStatement(JsCallContext *context) {
  // pop statement from the end of the queue
  Database2Statement *statement = statement_queue_.Pop();
  // if no more statements,
  if (!statement) {
    interpreter_->Run(new Database2CommitCommand(this));
    return;
  }

  if (interpreter_->async()) {
  //     interpreter_->Run(new Database2AsyncExecuteCommand(this));
  } else {
   assert(context);
   interpreter_->Run(new Database2SyncExecuteCommand(this, context,
       statement));
  }
}

void GearsDatabase2Transaction::InvokeErrorCallback() {
  // for synchronous transaction, throw an error in case of transaction failure,
  // otherwise, invoke callback or fail silently (per HTML5 spec)
  if (!interpreter_->async()) {
    GetJsRunner()->ThrowGlobalError(connection()->error_message());
    return;
  }
  
  if (!HasErrorCallback()) {
    return;
  }

  JsObject* error = new JsObject();
  if (!GearsDatabase2::CreateError(this, connection()->error_code(),
      connection()->error_message(), error)) {
    // unable to create an error object
    GetJsRunner()->ThrowGlobalError(GET_INTERNAL_ERROR_MESSAGE());
    return;
  }

  JsParamToSend send_argv[] = {
    { JSPARAM_OBJECT, error },
  };

  GetJsRunner()->InvokeCallback(error_callback_.get(), ARRAYSIZE(send_argv),
                                send_argv, NULL);
}

void GearsDatabase2Transaction::InvokeSuccessCallback() {
  // success callback may only exist for asynchronous transaction
  // InvokeSucessCallback() is called from CommitCommand, which doesn't know
  // whether it exists or not
  // TODO(dimitri.glazkov): think of a better pattern that doesn't cause
  // conflating the execution path
  if (interpreter_->async() && HasSuccessCallback()) {
    // TODO(dimitri.glazkov): investigate whether this is the right way to
    // invoke callback with no parameters
    GetJsRunner()->InvokeCallback(success_callback_.get(), 0, NULL, NULL);
  }
}

void GearsDatabase2Transaction::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  // clear callbacks, because in FF, the JS runtime may go away without
  // finishing garbage-collection
  callback_.reset();
  error_callback_.reset();
  success_callback_.reset();

  unload_monitor_.reset();
}
