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

#ifndef GEARS_DATABASE2_DATABASE2_H__
#define GEARS_DATABASE2_DATABASE2_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "gears/database2/connection.h"
#include "gears/database2/interpreter.h"
#include "gears/database2/thread_safe_queue.h"

// forward declarations
class GearsDatabase2Transaction;

typedef Database2ThreadSafeQueue<GearsDatabase2Transaction> 
    Database2TransactionQueue;

// Implements the HTML5 database interface, which allows the creation of 
// asynchronous transactions. We also have our own proprietary
// synchronousTransaction() method.
// This class creates and holds a reference to a Database2Connection object
// which it shares with all transactions it creates.
class GearsDatabase2 : public ModuleImplBaseClass {
 public:
  GearsDatabase2() : ModuleImplBaseClass("GearsDatabase2") {}
  ~GearsDatabase2() {}

  // creates an instance of GearsDatabase2
  static bool Create(ModuleEnvironment *module_environment,
                     JsCallContext *context,
                     const std::string16 &name,
                     const std::string16 &version,
                     Database2Connection *connection,
                     scoped_refptr<GearsDatabase2> *instance);

  // creates an object, implementing HTML5 SQLError interface
  static bool CreateError(const ModuleImplBaseClass *sibling, 
                          const int code, 
                          const std::string16 &message,
                          JsObject *instance);

  // returns (or creates) a transaction queue for this database
  Database2TransactionQueue *GetQueue();

  // IN: void
  // OUT: string version
  void GetVersion(JsCallContext *context) {
    context->SetReturnValue(JSPARAM_STRING16, &version_);
  }

  // IN: string old_version, string new_version, optional function callback,
  //     optional function success_callback, optional function failure
  // OUT: void
  void ChangeVersion(JsCallContext *context);

  // IN: function start_callback
  // OUT: void
  void SynchronousTransaction(JsCallContext *context);

  // IN: function callback, 
  //     optional function error_callback,
  //     optional function success_callback
  // OUT: void
  void Transaction(JsCallContext *context);

 private:
  void QueueTransaction(GearsDatabase2Transaction *transaction);

  std::string16 name_;
  std::string16 origin_;
  std::string16 version_;

  // Shared reference to the connection used by all transactions from this
  // database instance. This is initialized during the first transaction.
  scoped_refptr<Database2Connection> connection_;
  // The lifetime of the thread inside threaded interpreter is controlled
  // through reference-counting, so any object that uses it should hold a
  // scoped_refptr reference to it. The non-threaded interpreter does not
  // require counting and thus does not need to be accounted for.
  scoped_refptr<Database2ThreadedInterpreter> threaded_interpreter_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsDatabase2);
};

#endif // GEARS_DATABASE2_DATABASE2_H__
