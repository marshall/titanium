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

#ifndef GEARS_DATABASE2_COMMAND_H__
#define GEARS_DATABASE2_COMMAND_H__

#include "gears/base/common/common.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/database2/result_set2.h"
#include "gears/database2/statement.h"
#include "gears/database2/transaction.h"

// base command, represents a typical execution pattern
class Database2Command : public MessageData {
 public:
  Database2Command(GearsDatabase2Transaction *tx) : tx_(tx) {};
  // executes a command and sets has_results to true, if there are any results
  // to process
  virtual void Execute(bool *has_results) = 0;
  // processes the results. This should be only called if has_results was set to
  // true
  virtual void HandleResults() = 0;

 protected:
  GearsDatabase2Transaction *tx() const { return tx_.get(); }
  Database2Connection *connection() const { return tx_->connection(); }
  bool success() const { return success_; }
  void SetResult(bool success) { success_ = success; }

 private:
  bool success_;
  scoped_refptr<GearsDatabase2Transaction> tx_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2Command);
};


// begins a transaction
class Database2BeginCommand : public Database2Command {
 public:
  Database2BeginCommand(GearsDatabase2Transaction *tx) : Database2Command(tx) {}
  virtual void Execute(bool *has_results);
  virtual void HandleResults();

 private:
  DISALLOW_EVIL_CONSTRUCTORS(Database2BeginCommand);
};

// asynchronously executes a SQL statement
class Database2AsyncExecuteCommand : public Database2Command {
 public:
  Database2AsyncExecuteCommand(GearsDatabase2Transaction *tx,
                               Database2Statement *statement)
      : Database2Command(tx), statement_(statement) {}
  virtual void Execute(bool *has_results);
  virtual void HandleResults();

 private:
  scoped_ptr<Database2BufferingRowHandler> results_;
  scoped_ptr<Database2Statement> statement_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2AsyncExecuteCommand);
};

// synchronously executes a SQL statement
class Database2SyncExecuteCommand : public Database2Command {
 public:
  // constructor accepts JsCallContext, which is ok, because
  // this command is only instantiated if the interpreter is synchronous
  Database2SyncExecuteCommand(GearsDatabase2Transaction *tx,
                              JsCallContext *context,
                              Database2Statement *statement)
      : Database2Command(tx), context_(context), statement_(statement) {}
  virtual void Execute(bool *has_results);
  virtual void HandleResults();

 private:
  JsCallContext *context_;
  scoped_refptr<Database2ResultSet> result_set_;
  scoped_ptr<Database2Statement> statement_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2SyncExecuteCommand);
};

// commits a transaction
class Database2CommitCommand : public Database2Command {
 public:
   Database2CommitCommand(GearsDatabase2Transaction *tx)
       : Database2Command(tx) {}
  virtual void Execute(bool *has_results);
  virtual void HandleResults();

 private:
  DISALLOW_EVIL_CONSTRUCTORS(Database2CommitCommand);
};

// rolls back a transaction
class Database2RollbackCommand : public Database2Command {
 public:
  Database2RollbackCommand(GearsDatabase2Transaction *tx)
      : Database2Command(tx) {}
  virtual void Execute(bool *has_results);
  virtual void HandleResults();

 private:
  DISALLOW_EVIL_CONSTRUCTORS(Database2RollbackCommand);
};

#endif // GEARS_DATABASE2_COMMAND_H__
