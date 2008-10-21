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

#include <vector>
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread_locals.h"


const char *SQLDatabase::kUnspecifiedTransactionLabel = "Unspecified";

// Returns a static string describing any SQLite status code.
// Based on SQLite's internal function sqlite3ErrStr().
const char16* SqliteRetvalAsString(int status) {
  const char16 *z;
  switch (status & 0xFF) {
    case SQLITE_ROW:
    case SQLITE_DONE:
    case SQLITE_OK:         z = STRING16(L"not an error");                           break;
    case SQLITE_ERROR:      z = STRING16(L"SQL logic error or missing database");    break;
    case SQLITE_PERM:       z = STRING16(L"access permission denied");               break;
    case SQLITE_ABORT:      z = STRING16(L"callback requested query abort");         break;
    case SQLITE_BUSY:       z = STRING16(L"database is locked");                     break;
    case SQLITE_LOCKED:     z = STRING16(L"database table is locked");               break;
    case SQLITE_NOMEM:      z = STRING16(L"out of memory");                          break;
    case SQLITE_READONLY:   z = STRING16(L"attempt to write a readonly database");   break;
    case SQLITE_INTERRUPT:  z = STRING16(L"interrupted");                            break;
    case SQLITE_IOERR:      z = STRING16(L"disk I/O error");                         break;
    case SQLITE_CORRUPT:    z = STRING16(L"database disk image is malformed");       break;
    case SQLITE_FULL:       z = STRING16(L"database or disk is full");               break;
    case SQLITE_CANTOPEN:   z = STRING16(L"unable to open database file");           break;
    case SQLITE_PROTOCOL:   z = STRING16(L"database locking protocol failure");      break;
    case SQLITE_EMPTY:      z = STRING16(L"table contains no data");                 break;
    case SQLITE_SCHEMA:     z = STRING16(L"database schema has changed");            break;
    case SQLITE_CONSTRAINT: z = STRING16(L"constraint failed");                      break;
    case SQLITE_MISMATCH:   z = STRING16(L"datatype mismatch");                      break;
    case SQLITE_MISUSE:     z = STRING16(L"library routine called out of sequence"); break;
    case SQLITE_NOLFS:      z = STRING16(L"kernel lacks large file support");        break;
    case SQLITE_AUTH:       z = STRING16(L"authorization denied");                   break;
    case SQLITE_FORMAT:     z = STRING16(L"auxiliary database format error");        break;
    case SQLITE_RANGE:      z = STRING16(L"bind or column index out of range");      break;
    case SQLITE_NOTADB:     z = STRING16(L"file is encrypted or is not a database"); break;
    default:                z = STRING16(L"unknown error");                          break;
  }
  return z;
}


void BuildSqliteErrorString(const char16 *summary, int sql_status, sqlite3 *db,
                            std::string16 *out) {
  out->clear();
  out->append(summary);
  out->append(STRING16(L" ERROR: "));
  out->append(SqliteRetvalAsString(sql_status));
  out->append(STRING16(L" DETAILS: "));
  out->append(STRING16(sqlite3_errmsg16(db)));
}


static void LogIfConspicuouslyLongTime(const char *format_str,
                                       int64 start_time,
                                       const char *label) {
  const int64 kConspicuoslyLongTimeMsec = 5 * 1000;
  int64 duration = GetCurrentTimeMillis() - start_time;
  if (duration > kConspicuoslyLongTimeMsec) {
    LOG((format_str, static_cast<int>(duration), label));
  }
}

//------------------------------------------------------------------------------
// Constructor and Destructor
//------------------------------------------------------------------------------

SQLDatabase::SQLDatabase() : 
    db_(NULL), transaction_count_(0), needs_rollback_(false), 
    transaction_start_time_(0), transaction_listener_(NULL) {
}


SQLDatabase::~SQLDatabase() {
  Close();
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
bool SQLDatabase::Open(const char16 *name) {
  ASSERT_SINGLE_THREAD();

  // When parameter binding multiple parameters, we frequently use a scheme
  // of OR'ing return values together for testing for an error once after
  // all rv |= bind_foo() assignments have been made. This relies on
  // SQLITE_OK being 0.
  assert(SQLITE_OK == 0);

  // For testing purposes, we've seperated opening and configuring the
  // sqlite3 connection.
  if (!OpenConnection(name)) {
    return false;
  }

  if (!ConfigureConnection()) {
    sqlite3_close(db_);
    db_ = NULL;
    return false;
  }

  return true; 
}

bool SQLDatabase::OpenConnection(const char16 *name) {
  // Nobody should be calling Open() twice.
  assert(!db_);
  if (db_) {
    LOG(("SQLDatabase: already open\n"));
    return false;
  }

  transaction_count_ = 0;
  needs_rollback_ = false;

  std::string16 path;
  if (!GetFullDatabaseFilePath(name, &path)) {
    return false;
  }

  if (SQLITE_OK != sqlite3_open16(path.c_str(), &db_)) {
    // sqlite3_close() should be called after sqlite3_open() failures.
    // The DB handle may be valid or NULL, sqlite3_close() handles
    // either.
    sqlite3_close(db_);
    db_ = NULL;
    return false;
  }

  return true;
}

bool SQLDatabase::ConfigureConnection() {
  assert(db_);
  // Set the busy timeout value before executing any SQL statements.
  // With the timeout value set, SQLite will wait and retry if another
  // thread has the database locked rather than immediately fail with an
  // SQLITE_BUSY error.
  if (SQLITE_OK != sqlite3_busy_timeout(db_, kBusyTimeout)) {
    LOG(("SQLDatabase: Could not set busy timeout: %d\n",
         sqlite3_errcode(db_)));
    return false;
  }

  // Turn off flushing writes thru to disk, significantly faster (2x)
  if (SQLITE_OK != 
      sqlite3_exec(db_, "PRAGMA synchronous = OFF" , NULL, NULL, NULL)) {
    LOG(("SQLDatabase: Could not set PRAGMA synchronous: %d\n", 
         sqlite3_errcode(db_)));
    return false;
  }

  // Use UTF8, significantly smaller
  if (SQLITE_OK != 
      sqlite3_exec(db_, "PRAGMA encoding = \"UTF-8\"", NULL, NULL, NULL)) {
    LOG(("SQLDatabase: Could not set PRAGMA encoding: %d\n", 
         sqlite3_errcode(db_)));
    return false;
  }

  return true;
}


//------------------------------------------------------------------------------
// Close
//------------------------------------------------------------------------------
void SQLDatabase::Close() {
  if (db_) {
    sqlite3_close(db_);
    db_ = NULL;
  }  
}


//------------------------------------------------------------------------------
// IsOpen
//------------------------------------------------------------------------------
bool SQLDatabase::IsOpen() {
  ASSERT_SINGLE_THREAD();
  return db_ != NULL;
}


//------------------------------------------------------------------------------
// BeginTransaction
//------------------------------------------------------------------------------
bool SQLDatabase::BeginTransaction(const char *log_label) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  if (!db_) {
    return false;
  }

  if (!log_label)
    log_label = kUnspecifiedTransactionLabel;

  // EndTransaction should have been watching out for us going negative.
  assert(transaction_count_ >= 0);

  if (transaction_count_ > 0) {
    if (needs_rollback_) {
      LOG(("SQLDatabase: Cannot begin transaction for %s"
               " - already rolled back\n",
           log_label));
      return false;
    } else {
      ++transaction_count_;
      return true;
    }
  }

  LOG(("SQLDatabase: BeginTransaction for %s\n", log_label));
  transaction_start_time_ = GetCurrentTimeMillis();

  // We always use BEGIN IMMEDIATE for now but this could be parameterized in
  // the future if necessary.
  if (SQLITE_OK != sqlite3_exec(db_, "BEGIN IMMEDIATE", NULL, NULL, NULL)) {
    LOG(("SQLDatabase: Cannot exceute BEGIN IMMEDIATE: %d for %s\n", 
         sqlite3_errcode(db_),
         log_label));
    return false;
  }

  LogIfConspicuouslyLongTime(
      "SQLDatabase: Warning, BEGIN IMMEDIATE took %d ms for %s\n",
      transaction_start_time_, log_label);

  needs_rollback_ = false;
  ++transaction_count_;

  if (transaction_listener_) {
    transaction_listener_->OnBegin();
  }

  return true;
}


//------------------------------------------------------------------------------
// RollbackTransaction
//------------------------------------------------------------------------------
void SQLDatabase::RollbackTransaction(const char *log_label) {
  ASSERT_SINGLE_THREAD();
  needs_rollback_ = true;
  EndTransaction(log_label);
}


//------------------------------------------------------------------------------
// CommitTransaction
//------------------------------------------------------------------------------
bool SQLDatabase::CommitTransaction(const char *log_label) {
  ASSERT_SINGLE_THREAD();

  if (!EndTransaction(log_label)) {
    return false;
  }

  return !needs_rollback_;
}


//------------------------------------------------------------------------------
// EndTransaction
//------------------------------------------------------------------------------
bool SQLDatabase::EndTransaction(const char *log_label) {
  ASSERT_SINGLE_THREAD();

  if (!log_label)
    log_label = kUnspecifiedTransactionLabel;

  if (0 == transaction_count_) {
    LOG(("SQLDatabase: unbalanced transaction - %s\n", log_label));
    assert(false);
    return false;
  }

  // Always decrement. If Commit() fails we will Rollback(), which cannot fail.
  --transaction_count_;

  assert(db_);
  if (!db_) {
    return false;
  }

  if (transaction_count_ > 0) {
    // This is not the top tx, nothing to do
    return true;
  }

  LOG(("SQLDatabase: EndTransaction for %s\n", log_label));

  // OK, we are closing the last transaction, commit provided rollback has
  // not been called.
  if (!needs_rollback_) {
    if (SQLITE_OK == sqlite3_exec(db_, "COMMIT", NULL, NULL, NULL)) {
      LogIfConspicuouslyLongTime(
          "SQLDatabase: Committed transaction was open for %d ms for %s\n",
          transaction_start_time_, log_label);

      if (transaction_listener_) {
        transaction_listener_->OnCommit();
      }
      return true;
    }

    LOG(("SQLDatabase: Could not execute COMMIT: %d\n",
         sqlite3_errcode(db_)));

    // Since commit did not succeed, we should rollback. For most types of
    // errors, sqlite has already rolled back at this point. But for
    // SQLITE_BUSY, it won't have, and we want to treat that as an error.
  }

  // Rollback is necessary.
  // TODO(aa): What, if any, are the cases that rollback can fail. Should we do
  // anything about them?
  sqlite3_exec(db_, "ROLLBACK", NULL, NULL, NULL);
  LogIfConspicuouslyLongTime(
      "SQLDatabase: Rolled back transaction was open for %d ms for %s\n",
      transaction_start_time_, log_label);

  if (transaction_listener_) {
    transaction_listener_->OnRollback();
  }

  LOG(("SQLDatabase: Rolled back transaction for %s\n", log_label));
  return false;
}


//------------------------------------------------------------------------------
// DropAllObjects
//------------------------------------------------------------------------------
bool SQLDatabase::DropAllObjects() {
  // It appears that when you drop a table, it's associated indicies and
  // triggers also get dropped. However, shess cautions that there have been
  // bugs where virtual tables could not be dropped after the tables they
  // depedended upon had been dropped. This means that if we ever start using
  // virtual tables with this class, we may need to revisit this
  // implementation and find a way to distinguish virtual tables from regular
  // ones and drop those first.
  SQLTransaction tx(this, "SQLDatabase::DropAllObjects");
  if (!tx.Begin()) {
    return false;
  }

  // Find all the tables and gather them into a list
  SQLStatement stmt;
  const char16 *select_sql = 
    STRING16(L"SELECT name FROM sqlite_master WHERE type = 'table'");
  if (SQLITE_OK != stmt.prepare16(this, select_sql)) {
    LOG(("SQLDatabase::DropAllObjects - error preparing select: %d\n",
         sqlite3_errcode(db_)));
    return false;
  }

  std::vector<std::string16> table_names;
  int rv = stmt.step();
  while (SQLITE_ROW == rv) {
    table_names.push_back(std::string16(stmt.column_text16_safe(0)));
    rv = stmt.step();
  }

  if (SQLITE_DONE != rv) {
    LOG(("SQLDatabase::DropAllObjects - error iterating objects: %d\n",
         sqlite3_errcode(db_)));
    return false;
  }

  // Now iterate the list and drop all the tables we found
  for (std::vector<std::string16>::iterator iter = table_names.begin();
       iter != table_names.end(); ++iter) {
    SQLStatement drop_stmt;
    std::string16 drop_sql(STRING16(L"DROP TABLE "));
    drop_sql += (*iter);
    if (SQLITE_OK != drop_stmt.prepare16(this, drop_sql.c_str())) {
      // Some tables internal to sqlite may not be dropped, for example
      // sqlite_sequence. We ignore this error.
      if (StartsWith(*iter, std::string16(STRING16(L"sqlite_sequence"))))
        continue;
      return false;     
    }

    if (SQLITE_DONE != drop_stmt.step()) {
      LOG(("SQLDatabase::DropAllObjects - error dropping table: %d\n",
           sqlite3_errcode(db_)));
      return false;
    }
  }

  if (!tx.Commit()) {
    return false;
  }

  return true;
}


//------------------------------------------------------------------------------
// GetDBHandle
//------------------------------------------------------------------------------
sqlite3 *SQLDatabase::GetDBHandle() {
  ASSERT_SINGLE_THREAD();

  return db_;
}


//------------------------------------------------------------------------------
// SetTransactionListener
//------------------------------------------------------------------------------
void SQLDatabase::SetTransactionListener(SQLTransactionListener *listener) {
  ASSERT_SINGLE_THREAD();

  transaction_listener_ = listener;
}


//------------------------------------------------------------------------------
// GetFullDatabaseFilePath
//------------------------------------------------------------------------------
// static
bool SQLDatabase::GetFullDatabaseFilePath(const char16 *filename, 
                                          std::string16 *path) {
  if (!GetBaseDataDirectory(path)) {
    return false;
  }
  if (!File::RecursivelyCreateDir(path->c_str())) {
    return false;
  }
  (*path) += kPathSeparator;
  (*path) += filename;
  return true;
}
