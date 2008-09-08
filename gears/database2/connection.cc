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

#include "gears/database2/connection.h"

#include "gears/base/common/exception_handler.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
// TODO(aa): Refactor so we don't have to rely on code from database1.
#include "gears/database/database_utils.h"
#include "gears/database2/database2_common.h"
#include "gears/database2/statement.h"

// Open filename as a SQLite database, and setup appropriately for Gears use. 
// Returns SQLITE_OK in case of success, otherwise returns the error code sqlite
// returned.  The caller must arrange to eventually call sqlite3_close() on the
// handle returned in *db even if an error is returned.
//
// Note: we *do not* set a busy timeout because in Database2 we ensure that all
// database access is serial at the API layer.
//
// TODO(aa): Refactor to share with Database1 after integration dispatcher-based
// Database1.
static int OpenAndSetupDatabase(const std::string16 &filename, sqlite3 **db) {
  assert(*db == NULL);

  int sql_status = sqlite3_open16(filename.c_str(), db);
  if (sql_status != SQLITE_OK) {
    return sql_status;
  }

  // Set reasonable defaults.
  sql_status = sqlite3_exec(*db,
                            "PRAGMA encoding = 'UTF-8';"
                            "PRAGMA auto_vacuum = 1;"
                            "PRAGMA cache_size = 2048;"
                            "PRAGMA page_size = 4096;"
                            "PRAGMA synchronous = NORMAL;",
                            NULL, NULL, NULL
                            );
  if (sql_status != SQLITE_OK) {
    return sql_status;
  }

  sql_status = sqlite3_set_authorizer(*db, ForbidActions, NULL);
  if (sql_status != SQLITE_OK) {
    return sql_status;
  }

  return SQLITE_OK;
}

bool Database2Connection::Execute(const std::string16 &statement,
                                  int num_arguments,
                                  Database2Variant *arguments,
                                  Database2RowHandlerInterface *row_handler) {
  assert(row_handler);
  if (!OpenIfNecessary()) return false;
  int sqlite_status;

  if (bogus_version_) {
    SetAndHandleError(kDatabaseVersionMismatch, kInvalidStateError, SQLITE_OK);
    return false;
  }

  // prepare statement
  scoped_sqlite3_stmt_ptr stmt;
  sqlite_status = sqlite3_prepare16_v2(handle_, statement.c_str(), -1, &stmt,
                                       NULL);
  if (sqlite_status != SQLITE_OK) {
    SetAndHandleError(kOtherDatabaseError, kPrepareError, sqlite_status);
    return false;
  }

  if (stmt.get() == NULL) {
    error_message_ = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  // bind arguments
  for(int i = 0; i < num_arguments; ++i) {
    int sql_index = i + 1;  // sql parameters are 1-based
    switch(arguments[i].type) {
      case JSPARAM_INT: {
        sqlite_status = sqlite3_bind_int(stmt.get(), sql_index,
                                         arguments[i].int_value);
        break;
      }
      case JSPARAM_DOUBLE: {
        sqlite_status = sqlite3_bind_double(stmt.get(), sql_index,
                                 arguments[i].double_value);
        break;
      }
      case JSPARAM_STRING16: {
        sqlite_status = sqlite3_bind_text16(
            stmt.get(), sql_index,
            arguments[i].string_value->c_str(), -1,
            SQLITE_STATIC);
        break;
      }
      case JSPARAM_NULL: {
        sqlite_status = sqlite3_bind_null(stmt.get(), sql_index);
        break;
      }
      default: {
        // This should never occur, because Database2Variant only handles the
        // four types above by design.
        assert(false);
        return false;
      }
    }
    if (sqlite_status != SQLITE_OK) {
      SetAndHandleError(kOtherDatabaseError, kBindError, sqlite_status);
      return false;
    }
  }

  // get number of columns
  int column_count = sqlite3_column_count(stmt.get());
  scoped_array<std::string16> column_names(new std::string16[column_count]);

  // read column names
  for(int i = 0; i < column_count; ++i) {
    const void *column_name = sqlite3_column_name16(stmt.get(), i);
    column_names[i] = std::string16(static_cast<const char16 *>(column_name));
  }

  // row_handler takes ownership of the column names
  row_handler->Init(column_count, column_names.release());

  // read values
  while(true) {
    sqlite_status = sqlite3_step(stmt.get());
    if (sqlite_status == SQLITE_DONE) {
      break;
    }
    if (sqlite_status != SQLITE_ROW) {
      // an error has occured
      SetAndHandleError(kOtherDatabaseError, kStepError, sqlite_status);
      return false;
    }
    row_handler->HandleNewRow();
    for (int i = 0; i < column_count; ++i) {
      switch(sqlite3_column_type(stmt.get(), i)) {
        case SQLITE_INTEGER: {
          row_handler->HandleColumnInt(i, sqlite3_column_int(stmt.get(), i));
          break;
        }
        case SQLITE_FLOAT: {
          row_handler->HandleColumnDouble(i, 
                           sqlite3_column_double(stmt.get(), i));
          break;
        }
        case SQLITE_TEXT: {
          std::string16 value(
              static_cast<const char16*>(sqlite3_column_text16(stmt.get(), i)));
          row_handler->HandleColumnString(i, value);
          break;
        }
        case SQLITE_NULL: {
          row_handler->HandleColumnNull(i);
          break;
        }
        default: {
          // the only remaining SQLite type would be SQLITE_BLOB, which is not
          // supported
          SetAndHandleError(kOtherDatabaseError, kResultSetError, SQLITE_OK);
          return false;
        }
      }
    }
  }

  // read last inserted rowid
  sqlite_int64 rowid = sqlite3_last_insert_rowid(handle_);
  if ((rowid < JS_INT_MIN) || (rowid > JS_INT_MAX)) {
    SetAndHandleError(kOtherDatabaseError,
                      kLastRowIdOutOfRangeError, SQLITE_OK);
    return false;
  }

  int rows_affected = sqlite3_changes(handle_);

  row_handler->HandleStats(static_cast<int64>(rowid), rows_affected);
  return true;
}

void Database2Connection::SetAndHandleError(int error_code,
                                            const char16 *summary,
                                            int sqlite_status) {
  // TODO(dimitri.glazkov): add poisoning in case of corruption
  error_code_ = error_code;
  if (sqlite_status == SQLITE_OK) {
    // not a SQLite error
    error_message_ = summary;
    return;
  }
  BuildSqliteErrorString(summary, sqlite_status, handle_, &error_message_);
}

bool Database2Connection::Begin() {
  if (!OpenIfNecessary()) return false;

  // execute BEGIN    
  // if error, set error code and message, return false
  // read actual_version, if doesn't match expected_version_, 
    // set bogus_version_ flag
  // return true upon success
  return true;
}

void Database2BufferingRowHandler::Init(int column_count,
                                        std::string16 *column_names) {
  assert(column_count >= 0);
  // at this time, we only support invoking this method on a new instance
  assert(column_count_ == 0 && rows_.size() == 0);
  column_count_ = column_count;
  column_names_.reset(column_names);
}

void Database2BufferingRowHandler::HandleNewRow() {
  rows_.push_back(new Database2Variant[column_count_]);
}

bool Database2BufferingRowHandler::HandleColumnInt(int index, int value) {
  assert(index >=0 && index < column_count_);
  assert(rows_.size() > 0);
  Database2Variant *row = rows_.back();
  row[index].type = JSPARAM_INT;
  row[index].int_value = value;
  return true;
}

bool Database2BufferingRowHandler::HandleColumnDouble(int index, double value) {
  assert(index >=0 && index < column_count_);
  assert(rows_.size() > 0);
  Database2Variant *row = rows_.back();
  row[index].type = JSPARAM_DOUBLE;
  row[index].double_value = value;
  return true;
}

bool Database2BufferingRowHandler::HandleColumnString(
                                       int index,
                                       const std::string16 &value) {
  assert(index >=0 && index < column_count_);
  assert(rows_.size() > 0);
  Database2Variant *row = rows_.back();
  row[index].type = JSPARAM_STRING16;
  row[index].string_value = new std::string16(value);
  return true;
}

bool Database2BufferingRowHandler::HandleColumnNull(int index) {
  assert(index >=0 && index < column_count_);
  assert(rows_.size() > 0);
  Database2Variant *row = rows_.back();
  row[index].type = JSPARAM_NULL;
  return true;
}

void Database2BufferingRowHandler::HandleStats(int64 last_insert_rowid,
                                               int rows_affected) {
  last_insert_rowid_ = last_insert_rowid;
  rows_affected_ = rows_affected;
}

bool Database2BufferingRowHandler::CopyTo(
                                       Database2RowHandlerInterface *target) {
  // TODO(dimitri.glazkov): implement copying contents of this instance into
  // target
  return false;
}

void Database2Connection::Rollback() {
  assert(handle_);

  // execute ROLLBACK
  // don't remember or handle errors
}

bool Database2Connection::Commit() {
  assert(handle_);

  // execute COMMIT
  // if error, set error code and message, return false
  // return true upon success
  return true;
}

bool Database2Connection::OpenIfNecessary() {
  if (handle_) { return true; }  // already opened

  // Setup the directory.
  std::string16 dirname;
  if (!GetDataDirectory(origin_, &dirname)) {
    error_message_ = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  // Ensure directory exists; sqlite_open does not do this.
  if (!File::RecursivelyCreateDir(dirname.c_str())) {
    error_message_ = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  std::string16 full_path(dirname);
  full_path += kPathSeparator;
  full_path += filename_;

  sqlite3 *temp_db = NULL;
  int sql_status = OpenAndSetupDatabase(full_path, &temp_db);
  if (sql_status != SQLITE_OK) {
    sql_status = SqlitePoisonIfCorrupt(temp_db, sql_status);
    if (sql_status == SQLITE_CORRUPT) {
      database2_metadata_->MarkDatabaseCorrupt(origin_, filename_);
      ExceptionManager::ReportAndContinue();
      error_message_ = GET_INTERNAL_ERROR_MESSAGE();
    } else {
      error_message_ = GET_INTERNAL_ERROR_MESSAGE();
    }

    sqlite3_close(temp_db);
    return false;
  }

  handle_ = temp_db;
  return true;
}
