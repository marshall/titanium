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

#ifndef GEARS_DATABASE2_STATEMENT_H__
#define GEARS_DATABASE2_STATEMENT_H__

#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "third_party/scoped_ptr/scoped_ptr.h"
#include "gears/database2/connection.h"

// forward declarations
class GearsDatabase2Transaction;

// represents statement, for both synchronous and asynchronous operations
class Database2Statement {
 public:
  bool HasCallback() const {
    return callback_.get() != NULL;
  }

  bool HasErrorCallback() const {
    return error_callback_.get() != NULL;
  }

  void InvokeCallback(GearsDatabase2Transaction *tx);
  void InvokeErrorCallback(GearsDatabase2Transaction *tx, JsObject *error);

  // create a statement instance
  // must passs NULL for arguments or callbacks if they are not specified
  static bool Create(const std::string16 &sql_statement,
                     JsArray *sql_arguments,
                     JsRootedCallback *callback,
                     JsRootedCallback *error_callback,
                     Database2Statement **instance);

  const std::string16 &sql() const { return sql_statement_; }
  Database2Variant *arguments() const { return arguments_.get(); }
  int num_arguments() const { return num_arguments_; }
 private:
  Database2Statement() {}
  // if true, the statement has invalid arguments
  bool bogus_;
  std::string16 sql_statement_;
  scoped_array<Database2Variant> arguments_;
  int num_arguments_;

  scoped_ptr<JsRootedCallback> callback_;
  scoped_ptr<JsRootedCallback> error_callback_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2Statement);
};

#endif // GEARS_DATABASE2_STATEMENT_H__
