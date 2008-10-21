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

#include "gears/database2/statement.h"
#include "gears/database2/transaction.h"

void Database2Statement::InvokeCallback(GearsDatabase2Transaction *tx) {
  // for now, just return the Database2Statement
  JsParamToSend send_argv[] = {
    { JSPARAM_STRING16, &sql_statement_ }
  };

  if (HasCallback()) {
    tx->GetJsRunner()->InvokeCallback(callback_.get(), ARRAYSIZE(send_argv),
                                      send_argv, NULL);
  }
}

void Database2Statement::InvokeErrorCallback(GearsDatabase2Transaction *tx,
                                             JsObject *error) {
}

bool Database2Statement::Create(const std::string16 &sql_statement,
                                JsArray *sql_arguments,
                                JsRootedCallback *callback,
                                JsRootedCallback *error_callback,
                                Database2Statement **instance) {
  scoped_ptr<Database2Statement> statement(new Database2Statement());

  // NULL should be passed if no arguments are specified
  assert(!sql_arguments || !JsTokenIsNullOrUndefined(sql_arguments->token()));
  // NULL should be passed if a callback is not specified
  assert(!callback || !JsTokenIsNullOrUndefined(callback->token()));
  assert(!error_callback || !JsTokenIsNullOrUndefined(error_callback->token()));

  statement->sql_statement_.assign(sql_statement);
  statement->callback_.reset(callback);
  statement->error_callback_.reset(error_callback);

  scoped_array<Database2Variant> arguments;

  if (sql_arguments == NULL) {
    statement->num_arguments_ = 0;
    *instance = statement.release();
    return true;
  }

  int num_arguments;
  if (!sql_arguments->GetLength(&num_arguments)) {
    // unable to query JsArray, someting's gone horribly wrong
    // returning with failure will trigger an internal error
    assert(false);
    return false;
  }

  statement->num_arguments_ = num_arguments;
  arguments.reset(new Database2Variant[num_arguments]);
  for(int i = 0; i < num_arguments; ++i) {
    switch(sql_arguments->GetElementType(i)) {
      case JSPARAM_INT: {
        int value;
        if (!sql_arguments->GetElementAsInt(i, &value)) {
          return false;
        }
        arguments[i].type = JSPARAM_INT;
        arguments[i].int_value = value;
        break;
      }
      case JSPARAM_DOUBLE: {
        double value;
        if (!sql_arguments->GetElementAsDouble(i, &value)) {
          return false;
        }
        arguments[i].type = JSPARAM_DOUBLE;
        arguments[i].double_value = value;
        break;
      }
      case JSPARAM_STRING16: {
        std::string16 value;
        if (!sql_arguments->GetElementAsString(i, &value)) {
          return false;
        }
        arguments[i].type = JSPARAM_STRING16;
        arguments[i].string_value = new std::string16(value);
        break;
      }
      case JSPARAM_NULL: {
        // Variant's type is set to JSPARAM_NULL by default, no need to do
        // anything here
        break;
      }
      default: {
        // invalid type
        return false;
      }
    }
  }

  statement->arguments_.reset(arguments.release());

  *instance = statement.release();
  return true;
}
