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

#include "gears/database/database.h"

#include "gears/base/common/common.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/database/database_utils.h"
#include "gears/database/result_set.h"

#ifdef DEBUG
Stopwatch GearsDatabase::g_stopwatch_;
#endif // DEBUG

DECLARE_GEARS_WRAPPER(GearsDatabase);

const std::string GearsDatabase::kModuleName("GearsDatabase");

// static
template<>
void Dispatcher<GearsDatabase>::Init() {
  RegisterMethod("open", &GearsDatabase::Open);
  RegisterMethod("execute", &GearsDatabase::Execute);
  RegisterMethod("close", &GearsDatabase::Close);
  RegisterProperty("rowsAffected", &GearsDatabase::GetRowsAffected, NULL);
  RegisterProperty("lastInsertRowId", &GearsDatabase::GetLastInsertRowId, NULL);
#ifdef DEBUG
  RegisterProperty("executeMsec", &GearsDatabase::GetExecuteMsec, NULL);
#endif
}

GearsDatabase::GearsDatabase()
    : ModuleImplBaseClass(kModuleName),
      db_(NULL) {
}

GearsDatabase::~GearsDatabase() {
  assert(result_sets_.empty());

  if (db_) {
    sqlite3_close(db_);
    db_ = NULL;
  }
}

void GearsDatabase::Open(JsCallContext *context) {
  if (db_) {
    context->SetException(STRING16(L"A database is already open."));
    return;
  }

  // Create an event monitor to close remaining ResultSets when the page
  // unloads.
  if (unload_monitor_ == NULL) {
    unload_monitor_.reset(new JsEventMonitor(GetJsRunner(),
                                             JSEVENT_UNLOAD, this));
  }

  // Get parameters.
  std::string16 database_name;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &database_name },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 error_message;
  if (!IsUserInputValidAsPathComponent(database_name, &error_message)) {
    context->SetException(error_message.c_str());
    return;
  }

  // For now, callers cannot open DBs in other security origins.
  // To support that, parse an 'origin' argument here and call
  // IsOriginAccessAllowed (yet to be written).

  // Open the database.
  if (!OpenSqliteDatabase(database_name.c_str(), EnvPageSecurityOrigin(),
                          &db_)) {
    context->SetException(STRING16(L"Couldn't open SQLite database."));
  }
}

void GearsDatabase::Execute(JsCallContext *context) {
#ifdef WINCE
  // Sleep() is used as a poor-man's yield() here to improve concurrency
  // and prevent possible thread starvation on Windows Mobile,
  // especially for cases when one thread is using the database very 
  // actively, and other threads only use it occasionally.
  // The root problem is in SQLite lock implementation which
  // uses a busy wait.
  // TODO(shess): more efficient locking implementation on SQLite level.
  Sleep(0);  
#endif  // WINCE

#ifdef DEBUG
  ScopedStopwatch scoped_stopwatch(&GearsDatabase::g_stopwatch_);
#endif // DEBUG

  if (!db_) {
    context->SetException(STRING16(L"Database handle was NULL."));
    return;
  }

  // Get parameters.
  std::string16 expr;
  JsArray arg_array;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &expr },
    { JSPARAM_OPTIONAL, JSPARAM_ARRAY, &arg_array },
  };
  int argc = context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

#ifdef BROWSER_IE
  LOG16((L"DB Execute: %s\n", expr));
#else
#ifdef DEBUG
// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
  std::string expr_utf8;
  String16ToUTF8(expr.c_str(), expr.length(), &expr_utf8);
  LOG(("DB Execute: %s\n", expr_utf8.c_str()));
#endif
#endif

  // Prepare a statement for execution.
  scoped_sqlite3_stmt_ptr stmt;
  int sql_status = sqlite3_prepare16_v2(db_, expr.c_str(), -1, &stmt, NULL);
  if ((sql_status != SQLITE_OK) || (stmt.get() == NULL)) {
    sql_status = SqlitePoisonIfCorrupt(db_, sql_status);

    std::string16 msg;
    BuildSqliteErrorString(STRING16(L"SQLite prepare() failed."),
                           sql_status, db_, &msg);
    msg += STRING16(L" EXPRESSION: ");
    msg += expr;
    context->SetException(msg.c_str());
    return;
  }

  // Bind parameters
  if (!BindArgsToStatement(context, (argc >= 2) ? &arg_array : NULL,
                           stmt.get())) {
    // BindArgsToStatement already set an exception
    return;
  }

  // Wrap a GearsResultSet around the statement and execute it
  scoped_refptr<GearsResultSet> result_set;
  if (!CreateModule<GearsResultSet>(module_environment_.get(),
                                    context, &result_set)) {
    return;
  }

  // Note the ResultSet takes ownership of the statement
  std::string16 error_message;
  if (!result_set->InitializeResultSet(stmt.release(), this, &error_message)) {
    context->SetException(error_message.c_str());
    return;
  }

  context->SetReturnValue(JSPARAM_MODULE, result_set.get());
}

bool GearsDatabase::BindArgsToStatement(JsCallContext *context,
                                        const JsArray *arg_array,
                                        sqlite3_stmt *stmt) {
  int num_args_expected = sqlite3_bind_parameter_count(stmt);
  int num_args = 0;

  if (arg_array && !arg_array->GetLength(&num_args)) {
    context->SetException(STRING16(L"Error finding array length."));
    return false;
  }

  if (num_args_expected != num_args) {
    context->SetException(STRING16(L"Wrong number of SQL parameters."));
    return false;
  }

  for (int i = 0; i < num_args; i++) {
    int sql_index = i + 1; // sql parameters are 1-based
    int sql_status = SQLITE_ERROR;
    
    JsParamType element_type = arg_array->GetElementType(i);
    
    switch (element_type) {
      case JSPARAM_STRING16: {
        std::string16 arg_str;
        if (!arg_array->GetElementAsString(i, &arg_str)) {
          context->SetException(GET_INTERNAL_ERROR_MESSAGE().c_str());
          return false;
        }
#ifdef BROWSER_IE
        LOG16((L"        Parameter %i: %s (string)\n", i, arg_str));
#else
#ifdef DEBUG
        std::string str_utf8;
        String16ToUTF8(arg_str.c_str(), arg_str.length(), &str_utf8);
        LOG(("        Parameter %i: %s (string)\n", i, str_utf8.c_str()));
#endif
#endif
        sql_status = sqlite3_bind_text16(
            stmt, sql_index, arg_str.c_str(), -1,
            SQLITE_TRANSIENT); // so SQLite copies string immediately
        break;
      }
      case JSPARAM_NULL: {
#ifdef BROWSER_IE
        LOG16((L"        Parameter %i: null\n", i));
#else
        LOG(("        Parameter %i: null\n", i));
#endif
        sql_status = sqlite3_bind_null(stmt, sql_index);
        break;
      }
      case JSPARAM_UNDEFINED: {
        // Insert the string "undefined" to match the firefox implementation.
        // TODO(zork): This should throw an error in beta.database2.
#ifdef BROWSER_IE
        LOG16((L"        Parameter %i: undefined\n", i));
#else
        LOG(("        Parameter %i: undefined\n", i));
#endif
        sql_status = sqlite3_bind_text16(
            stmt, sql_index, STRING16(L"undefined"), -1,
            SQLITE_TRANSIENT); // so SQLite copies string immediately
        break;
      }
      case JSPARAM_INT: {
        int arg_int;
        if (!arg_array->GetElementAsInt(i, &arg_int)) {
          context->SetException(GET_INTERNAL_ERROR_MESSAGE().c_str());
          return false;
        }
#ifdef BROWSER_IE
        LOG16((L"        Parameter %i: %i\n", i, arg_int));
#else
        LOG(("        Parameter %i: %i\n", i, arg_int));
#endif
        sql_status = sqlite3_bind_int(stmt, sql_index, arg_int);
        break;
      }
      case JSPARAM_DOUBLE: {
        double arg_double;
        if (!arg_array->GetElementAsDouble(i, &arg_double)) {
          context->SetException(GET_INTERNAL_ERROR_MESSAGE().c_str());
          return false;
        }
#ifdef BROWSER_IE
        LOG16((L"        Parameter %i: %lf\n", i, arg_double));
#else
        LOG(("        Parameter %i: %lf\n", i, arg_double));
#endif
        sql_status = sqlite3_bind_double(stmt, sql_index, arg_double);
        break;
      }
      case JSPARAM_BOOL: {
        bool arg_bool;
        if (!arg_array->GetElementAsBool(i, &arg_bool)) {
          context->SetException(GET_INTERNAL_ERROR_MESSAGE().c_str());
          return false;
        }
        std::string16 arg_str;
        arg_str = arg_bool ? STRING16(L"true") : STRING16(L"false");
#ifdef BROWSER_IE
        LOG16((L"        Parameter %i: %s\n", i, arg_str));
#else
#ifdef DEBUG
        std::string str_utf8;
        String16ToUTF8(arg_str.c_str(), arg_str.length(), &str_utf8);
        LOG(("        Parameter %i: %s\n", i, str_utf8.c_str()));
#endif
#endif
        sql_status = sqlite3_bind_text16(
            stmt, sql_index, arg_str.c_str(), -1,
            SQLITE_TRANSIENT); // so SQLite copies string immediately
        break;
      }
      default: {
        std::string16 error =
            STRING16(L"SQL parameter ") + IntegerToString16(i) +
            STRING16(L" has unknown type.");
        context->SetException(error.c_str());
        return false;
        break;
      }
    }

    if (sql_status != SQLITE_OK) {
      sql_status = SqlitePoisonIfCorrupt(db_, sql_status);
      context->SetException(STRING16(
                                L"Could not bind arguments to expression."));
      return false;
    }
  }

  return true;
}

void GearsDatabase::Close(JsCallContext *context) {
  if (!CloseInternal()) {
    context->SetException(STRING16(L"SQLite close() failed."));
  }
}

void GearsDatabase::GetLastInsertRowId(JsCallContext *context) {
  if (!db_) {
    context->SetException(STRING16(L"Database handle was NULL."));
    return;
  }

  sqlite_int64 rowid = sqlite3_last_insert_rowid(db_);
  context->SetReturnValue(JSPARAM_INT64, &rowid);
}

void GearsDatabase::GetRowsAffected(JsCallContext *context) {
  if (!db_) {
    context->SetException(STRING16(L"Database handle was NULL."));
    return;
  }

  int retval = sqlite3_changes(db_);
  context->SetReturnValue(JSPARAM_INT, &retval);
}

void GearsDatabase::AddResultSet(GearsResultSet *rs) {
  result_sets_.insert(rs);
}

void GearsDatabase::RemoveResultSet(GearsResultSet *rs) {
  assert(result_sets_.find(rs) != result_sets_.end());

  result_sets_.erase(rs);
}


bool GearsDatabase::CloseInternal() {
  if (db_) {
    for (std::set<GearsResultSet *>::iterator result_set = result_sets_.begin();
         result_set != result_sets_.end();
         ++result_set) {
      (*result_set)->Finalize();
    }

    int sql_status = sqlite3_close(db_);
    db_ = NULL;
    if (sql_status != SQLITE_OK) {
      return false;
    }
  }
  return true;
}

void GearsDatabase::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  CloseInternal();

  scoped_refptr<GearsDatabase> keep_alive(this);
  // When the page unloads, NPAPI plugins are unloaded.  When that happens,
  // objects are cleaned up and deleted regardless of reference count, so we
  // can't ensure that the ResultSets are deleted first.  So we give them a
  // chance to do cleanup while the GearsDatabase object still exists.
  for (std::set<GearsResultSet *>::iterator result_set = result_sets_.begin();
       result_set != result_sets_.end();
       ++result_set) {
    (*result_set)->PageUnloading();
  }
  result_sets_.clear();
}

#ifdef DEBUG
void GearsDatabase::GetExecuteMsec(JsCallContext *context) {
  int retval = GearsDatabase::g_stopwatch_.GetElapsed();
  context->SetReturnValue(JSPARAM_INT, &retval);
}
#endif
