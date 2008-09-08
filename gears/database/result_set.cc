// Copyright 2007, Google Inc.
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

#include "gears/database/result_set.h"

#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/stopwatch.h"
#include "gears/database/database.h"
#include "gears/database/database_utils.h"

DECLARE_GEARS_WRAPPER(GearsResultSet);

const std::string GearsResultSet::kModuleName("GearsResultSet");

// static
template<>
void Dispatcher<GearsResultSet>::Init() {
  RegisterMethod("field", &GearsResultSet::Field);
  RegisterMethod("fieldByName", &GearsResultSet::FieldByName);
  RegisterMethod("fieldName", &GearsResultSet::FieldName);
  RegisterMethod("fieldCount", &GearsResultSet::FieldCount);
  RegisterMethod("close", &GearsResultSet::Close);
  RegisterMethod("next", &GearsResultSet::Next);
  RegisterMethod("isValidRow", &GearsResultSet::IsValidRow);
}

GearsResultSet::GearsResultSet()
    : ModuleImplBaseClass(kModuleName),
      statement_(NULL),
      is_valid_row_(false) {
}

GearsResultSet::~GearsResultSet() {
  if (statement_) {
#if BROWSER_IE
    LOG16((L"~GearsResultSet - was NOT closed by caller\n"));
#else
    LOG(("~GearsResultSet - was NOT closed by caller\n"));
#endif
  }

  Finalize();

  if (database_ != NULL) {
    database_->RemoveResultSet(this);
    database_ = NULL;
  }
}

bool GearsResultSet::InitializeResultSet(sqlite3_stmt *statement,
                                         GearsDatabase *db,
                                         std::string16 *error_message) {
  assert(statement);
  assert(db);
  assert(error_message);
  statement_ = statement;
  // convention: call next() when the statement is set
  bool succeeded = NextImpl(error_message); 
  if (!succeeded || sqlite3_column_count(statement_) == 0) {
    // Either an error occurred or this was a command that does
    // not return a row, so we can just close automatically
    Finalize();
  } else {
    database_ = db;
    db->AddResultSet(this);
  }
  return succeeded;
}

void GearsResultSet::PageUnloading() {
  if (database_ != NULL) {
    // Don't remove ourselves from the result set, since database_ is going away
    // soon anyway.
    database_ = NULL;
  }
}

bool GearsResultSet::Finalize() {
  if (statement_) {
    sqlite3 *db = sqlite3_db_handle(statement_);
    int sql_status = sqlite3_finalize(statement_);
    sql_status = SqlitePoisonIfCorrupt(db, sql_status);
    statement_ = NULL;

#if BROWSER_IE
    LOG16((L"DB ResultSet Close: %d", sql_status));
#else
    LOG(("DB ResultSet Close: %d", sql_status));
#endif

    if (sql_status != SQLITE_OK) {
      return false;
    }
  }
  return true;
}

void GearsResultSet::FieldImpl(JsCallContext *context, int index) {
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif // DEBUG

  if (statement_ == NULL) {
    context->SetException(STRING16(L"SQL statement is NULL."));
    return;
  }
  if ((index < 0) || (index >= sqlite3_column_count(statement_))) {
    context->SetException(STRING16(L"Invalid index."));
    return;
  }

  int column_type = sqlite3_column_type(statement_, index);
  switch (column_type) {
    case SQLITE_INTEGER: {
      sqlite_int64 i64 = sqlite3_column_int64(statement_, index);
      context->SetReturnValue(JSPARAM_INT64, &i64);
      break;
    }
    case SQLITE_FLOAT: {
      double retval = sqlite3_column_double(statement_, index);
      context->SetReturnValue(JSPARAM_DOUBLE, &retval);
      break;
    }
    case SQLITE_TEXT: {
      const void *text = sqlite3_column_text16(statement_, index);
      std::string16 retval(static_cast<const char16 *>(text));
      context->SetReturnValue(JSPARAM_STRING16, &retval);
      break;
    }
    case SQLITE_NULL:
      context->SetReturnValue(JSPARAM_NULL, 0);
      break;
    case SQLITE_BLOB:
      // TODO(miket): figure out right way to pass around blobs in variants.
      context->SetException(STRING16(L"Data type not supported."));
      return;
    default:
      context->SetException(STRING16(L"Data type not supported."));
      return;
  }
}

void GearsResultSet::Field(JsCallContext *context) {
  // Get parameters.
  int index;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &index },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  FieldImpl(context, index);  // sets the return value/error.
}

void GearsResultSet::FieldByName(JsCallContext *context) {
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif // DEBUG

  if (statement_ == NULL) {
    context->SetException(STRING16(L"SQL statement is NULL."));
    return;
  }

  // Get parameters.
  std::string16 field_name;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &field_name },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  // TODO(miket): This is horrible O(n) code but we didn't have a hashtable
  // implementation handy. Fix this!
  int n = sqlite3_column_count(statement_);
  int i;
  for (i = 0; i < n; ++i) {
    const void *column_name = sqlite3_column_name16(statement_, i);
    std::string16 s(static_cast<const char16 *>(column_name));
    if (field_name == s) {
      break;  // found it
    }
  }
  if (i >= n) {
    context->SetException(STRING16(L"Field name not found."));
    return;
  }

  FieldImpl(context, i);  // sets the return value/error.
}

void GearsResultSet::FieldName(JsCallContext *context) {
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif // DEBUG

  if (statement_ == NULL) {
    context->SetException(STRING16(L"SQL statement is NULL."));
    return;
  }

  // Get parameters.
  int index;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &index },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  if ((index < 0) || (index >= sqlite3_column_count(statement_))) {
    context->SetException(STRING16(L"Invalid index."));
    return;
  }

  const void *column_name = sqlite3_column_name16(statement_, index);
  std::string16 retval(static_cast<const char16 *>(column_name));
  context->SetReturnValue(JSPARAM_STRING16, &retval);
}

void GearsResultSet::FieldCount(JsCallContext *context) {
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif // DEBUG

  // rs.fieldCount() should never throw. Return 0 if there is no statement.
  int retval = statement_ ? sqlite3_column_count(statement_) : 0;
  context->SetReturnValue(JSPARAM_INT, &retval);
}

void GearsResultSet::Close(JsCallContext *context) {
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif // DEBUG

  if (!Finalize()) {
    context->SetException(STRING16(L"SQLite finalize() failed."));
  }
}

void GearsResultSet::Next(JsCallContext *context) {
  if (!statement_) {
    context->SetException(STRING16(L"Called Next() with NULL statement."));
    return;
  }
  std::string16 error_message;
  if (!NextImpl(&error_message)) {
    context->SetException(error_message.c_str());
  }
}

bool GearsResultSet::NextImpl(std::string16 *error_message) {
#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif // DEBUG
  assert(statement_);
  assert(error_message);
  int sql_status = sqlite3_step(statement_);
  sql_status = SqlitePoisonIfCorrupt(sqlite3_db_handle(statement_),
                                     sql_status);
  LOG(("GearsResultSet::next() sqlite3_step returned %d", sql_status));
  switch (sql_status) {
    case SQLITE_ROW:
      is_valid_row_ = true;
      break;
    case SQLITE_BUSY:
      // If there was a timeout (SQLITE_BUSY) the SQL row cursor did not
      // advance, so we don't reset is_valid_row_. If it was valid prior to
      // this call, it's still valid now.
      break;
    default:
      is_valid_row_ = false;
      break;
  }
  bool succeeded = (sql_status == SQLITE_ROW) ||
                   (sql_status == SQLITE_DONE) ||
                   (sql_status == SQLITE_OK);
  if (!succeeded) {
    BuildSqliteErrorString(STRING16(L"Database operation failed."),
                           sql_status, sqlite3_db_handle(statement_),
                           error_message);
  }
  return succeeded;
}

void GearsResultSet::IsValidRow(JsCallContext *context) {
  // rs.isValidRow() should never throw. Return false if there is no statement.
  bool valid = false;
  if (statement_ != NULL) {
    valid = is_valid_row_;
  }

  context->SetReturnValue(JSPARAM_BOOL, &valid);
}
