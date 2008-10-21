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

#ifndef GEARS_DATABASE2_TRANSACTION_H__
#define GEARS_DATABASE2_TRANSACTION_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/database2/connection.h"
#include "gears/database2/interpreter.h"
#include "gears/database2/thread_safe_queue.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// forward declarations
class GearsDatabase2;
class Database2Statement;

// TODO(dimitri.glazkov): Currently, GearsDatabase2Transaction has to listen for
// the JSEVENT_UNLOAD event and release its callbacks, because it appears that
// the js engine may go away before the GearsDatabase2Transaction is
// garbage-collected, which causes a crash trying to unroot the callbacks in
// destructor. This problem was only observed when the module instance is
// created in an iframe. Figure out whether there is a better way to handle
// this.
class GearsDatabase2Transaction
    : public ModuleImplBaseClass,
      public JsEventHandlerInterface {
 public:
  GearsDatabase2Transaction()
      : ModuleImplBaseClass("GearsDatabase2Transaction") {}
  ~GearsDatabase2Transaction() {}
  // creates GearsDatabase2Transaction instance
  static bool Create(ModuleEnvironment *module_environment,
                     JsCallContext *context,
                     Database2Connection *connection,
                     Database2Interpreter *interpreter,
                     JsRootedCallback *callback,
                     JsRootedCallback *error_callback,
                     JsRootedCallback *success_callback,
                     scoped_refptr<GearsDatabase2Transaction> *instance);

  void Start();

  void InvokeCallback();

  void MarkOpen() { is_open_ = true; }

  // IN: in string sqlDatabase2Statement, 
  //     in object[] arguments, 
  //     in function callback, 
  //     in function errorCallback
  // OUT: void
  void ExecuteSql(JsCallContext *context);

  bool open() const { return is_open_; }

  void ExecuteNextStatement(JsCallContext *context);

  bool HasErrorCallback() const {
    assert(error_callback_.get());
    return !JsTokenIsNullOrUndefined(error_callback_->token());
  }

  void MarkClosed() { is_open_ = false; }

  void InvokeErrorCallback();

  bool HasSuccessCallback() const {
    // checking for async is necessary because this method is being used to
    // verify that the callback exists or _could_ be called at all (that is,
    // in case the interpereter is synchronous)
    return interpreter_->async() &&
      !JsTokenIsNullOrUndefined(success_callback_->token());
  }

  void InvokeSuccessCallback();

  // handles the unload event, cleans up the callbacks to prevent
  // crashes in Firefox
  virtual void HandleEvent(JsEventType event_type);

  Database2Connection *connection() const { return connection_.get(); }

 private:
  std::string16 old_version_;
  std::string16 new_version_;

  scoped_refptr<Database2Connection> connection_;
  scoped_refptr<Database2Interpreter> interpreter_;

  Database2ThreadSafeQueue<Database2Statement> statement_queue_;

  bool is_open_;

  // monitors JSEVENT_UNLOAD
  scoped_ptr<JsEventMonitor> unload_monitor_;

  scoped_ptr<JsRootedCallback> callback_;
  scoped_ptr<JsRootedCallback> error_callback_;
  scoped_ptr<JsRootedCallback> success_callback_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsDatabase2Transaction);
};

#endif // GEARS_DATABASE2_TRANSACTION_H__
