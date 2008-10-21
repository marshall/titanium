// Copyright 2006, Google Inc.
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

#ifndef GEARS_BASE_COMMON_SQLITE_WRAPPER_H__
#define GEARS_BASE_COMMON_SQLITE_WRAPPER_H__

#include <assert.h>
#include <vector>
#include "gears/base/common/common.h"
#include "gears/base/common/string16.h"
#ifdef OS_ANDROID
// Use the system SQLite on Android.
#include <sqlite3.h>
#else
#include "third_party/sqlite_google/preprocessed/sqlite3.h"
#endif

// forward declarations of classes defined here
class SQLTransaction;
class scoped_sqlite3_stmt_ptr;
class SQLStatement;


// Fills 'out' with detailed text about a SQLite error.
void BuildSqliteErrorString(const char16 *summary, int sql_status, sqlite3 *db,
                            std::string16 *out);


//------------------------------------------------------------------------------
// Defines an interface for people wanting to know when transactions happen
// in SQLDatabase. See SetTransactionListener().
//------------------------------------------------------------------------------
class SQLTransactionListener {
 public:
  virtual void OnBegin() = 0;
  virtual void OnCommit() = 0;
  virtual void OnRollback() = 0;
};


//------------------------------------------------------------------------------
// A convenience wrapper for SQLite databases.
// Not thread-safe.
//------------------------------------------------------------------------------
class SQLDatabase {
 public:
  SQLDatabase();
  ~SQLDatabase();

  // Open the database
  bool Open(const char16 *name);

  // Returns true if the database connection is open
  bool IsOpen();

  // Close the database.
  void Close();

  int Execute(const char *sql) {
    assert(db_);
    return sqlite3_exec(db_, sql, NULL, NULL, NULL);
  }

  int GetErrorCode() { 
    assert(db_);
    return sqlite3_errcode(db_);
  }

  const char *GetErrorMessage() {
    assert(db_);
    return sqlite3_errmsg(db_);
  }

  bool IsInTransaction() {
    return transaction_count_ > 0;
  }
  
  // Begin a transaction. SQLite does not support nested transactions so we
  // simulate them by keeping a count of how many open ones there are. The
  // log_label parameter is optional and may be NULL.
  //
  // NOTE: There are very strict requirements regarding the use of these
  // methods. For each BeginTransaction() that returns true,
  // CommitTransaction() or RollbackTransaction() (not both) *must* be called.
  // For each BeginTransaction() that return false, CommitTransaction() or
  // RollbackTransaction() *must not* be called.
  //
  // Clients are encouraged to use the convenience scoped wrapped
  // SQLTransaction instead of these methods where possible.
  //
  // The general pattern is:
  //
  // if (!db.BeginTransaction("LoggingLabel")) {
  //   return false;
  // }
  //
  // ... do your db stuff here ...
  //
  // if (youDecideToRollback) {
  //   db.Rollback("LoggingLabel");
  //   return false;
  // }
  //
  // if (!db.CommitTransaction("LoggingLabel")) {
  //   ... The db has been rolled back. Do no manually call db.Rollback() ...
  //   return false;
  // }
  //
  // return true;
  bool BeginTransaction(const char *log_label);

  // Rollback a transaction started with BeginTransaction. If multiple 
  // transactions are open, the rollback will occur when the last transaction 
  // is committed or rolled back. The log_label parameter is optional and may
  // be NULL.
  void RollbackTransaction(const char *log_label);

  // Commit a transaction started with BeginTransaction. If multiple
  // transactions are open, the commit will occur when the last transaction is
  // committed, so long as RollbackTransaction() has not been called. The
  // log_label parameter is optional and may be NULL.
  bool CommitTransaction(const char *log_label);

  // Callbacks
  void SetTransactionListener(SQLTransactionListener *listener);

  // Drop all objects (tables, indicies, etc) from the database
  bool DropAllObjects();

  static const char *kUnspecifiedTransactionLabel;

  // We set the busy timeout to 5 seconds. What we are basically saying here is
  // that if a single SQL operation ever takes longer than 5 seconds something
  // very serious has gone wrong and it should be considered an error. It may
  // be that there are legitimite reasons for this to be higher, but let's start
  // out strict and loosen if necessary.
  static const int kBusyTimeout = 5 * 1000;

 private:
  // SQLite handles, which this class wraps, can only be used on a single
  // thread.
  DECL_SINGLE_THREAD

  // Returns the sqlite3 database connection associated with this site.
  // TODO(shess) Friends are our own worst enemy.  Kill this.
  friend class SQLStatement;
  sqlite3 *GetDBHandle();

  friend bool TestSQLConcurrency();
  bool OpenConnection(const char16 *name);
  bool ConfigureConnection();

  // Private helper called by CommmitTransaction and RollbackTransaction
  bool EndTransaction(const char *log_label);

  // Creates (if necessary) the directory for the specified database file, 
  // and stores the full path of this file in 'path'. Returns true if 
  // successful.
  static bool GetFullDatabaseFilePath(const char16 *filename, 
                                      std::string16 *path);

  // The sqlite3 database handle we are using
  sqlite3 *db_;
  
  // Number of nested transactions that are open
  int transaction_count_;

  // Whether the current transaction needs rollback
  bool needs_rollback_;

  // When the current transaction was started, not valid when there is
  // no transaction open
  int64 transaction_start_time_;  // used for logging only

  // callbacks
  SQLTransactionListener *transaction_listener_;
};


//------------------------------------------------------------------------------
// A convenience for managing SQL transactions that rolls back when it goes out
// of  scope if the caller has not called Commit or Rollback.
// Note: the constructor does NOT Begin a transaction.
//------------------------------------------------------------------------------
class SQLTransaction {
 public:
  SQLTransaction(SQLDatabase *db, const char *log_label)
     : began_(false), db_(db),
       log_label_(log_label ? log_label
                            : SQLDatabase::kUnspecifiedTransactionLabel) {
  }

  ~SQLTransaction() {
    if (began_) {
      Rollback();
    }
  }

  bool Begin() {
    assert(!began_);
    if (began_) {
      return false;
    }

    if (!db_->BeginTransaction(log_label_.c_str())) {
      return false;
    }
    began_ = true;
    return true;
  }

  bool BeginIfNeeded() {
    if (began_) {
      return true;
    }

    return Begin();
  }

  bool Commit() {
    return End(true);
  }

  void Rollback() {
    End(false);
  }

  bool HasBegun() {
    return began_;
  }

 private:
  bool End(bool commit) {
    assert(began_);
    if (!began_) {
      return false;
    }

    began_ = false;
    if (commit) {
      return db_->CommitTransaction(log_label_.c_str());
    } else {
      db_->RollbackTransaction(log_label_.c_str());
      return true;
    }
  }

  bool began_;
  SQLDatabase *db_;
  std::string log_label_;
  DISALLOW_EVIL_CONSTRUCTORS(SQLTransaction);
};


//------------------------------------------------------------------------------
// A scoped sqlite statement that finalizes when it goes out of scope.
//------------------------------------------------------------------------------
class scoped_sqlite3_stmt_ptr {
 public:
  ~scoped_sqlite3_stmt_ptr() {
    finalize();
  }

  scoped_sqlite3_stmt_ptr() : stmt_(NULL) {
  }

  explicit scoped_sqlite3_stmt_ptr(sqlite3_stmt *stmt)
    : stmt_(stmt) {
  }

  sqlite3_stmt *get() const {
    return stmt_;
  }

  void set(sqlite3_stmt *stmt) {
    finalize();
    stmt_ = stmt;
  }

  sqlite3_stmt *release() {
    sqlite3_stmt *tmp = stmt_;
    stmt_ = NULL;
    return tmp;
  }

  // The & operator supports usage with sqlite3_prepare(..., &stmt,...)
  sqlite3_stmt** operator &() {
    assert(stmt_ == NULL);
    return &stmt_;
  }

  // It is not safe to call sqlite3_finalize twice on the same stmt.
  // Sqlite3's sqlite3_finalize() function should not be called directly
  // without calling the release method.  If sqlite3_finalize() must be
  // called directly, the following usage is advised:
  //  scoped_sqlite3_stmt_ptr stmt;
  //  ... do something with stmt ...
  //  sqlite3_finalize(stmt.release());
  int finalize() {
    int err = sqlite3_finalize(stmt_);
    stmt_ = NULL;
    return err;
  }

 protected:
  sqlite3_stmt *stmt_;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(scoped_sqlite3_stmt_ptr);
};


//------------------------------------------------------------------------------
// A scoped sqlite statement with convenient C++ wrappers for sqlite3 APIs.
//
// Note: we don't include the non-wide-char SQLite functions here, because we
// don't want ASCII strings creeping into our shared code.
//------------------------------------------------------------------------------
class SQLStatement : public scoped_sqlite3_stmt_ptr {
public:
  SQLStatement() {
  }

  int prepare16(SQLDatabase *db, const char16 *sql) {
    return prepare16(db, sql, -1);
  }

  // sql_len is number of characters or may be negative
  // a for null-terminated sql string
  int prepare16(SQLDatabase *db, const char16 *sql, int sql_len) {
    assert(!stmt_);
    sql_len *= sizeof(char16);
    int rv = sqlite3_prepare16_v2(db->GetDBHandle(), sql, sql_len, &stmt_, 
                                  NULL);
    if (rv != SQLITE_OK) {
      LOG(("SQLStatement.prepare16 failed: %s\n", 
           sqlite3_errmsg(db->GetDBHandle())));
    }
    return rv;
  }

  int step() {
    assert(stmt_);
    return sqlite3_step(stmt_);
  }

  int reset() {
    assert(stmt_);
    return sqlite3_reset(stmt_);
  }

  sqlite_int64 last_insert_rowid() {
    assert(stmt_);
    return sqlite3_last_insert_rowid(db_handle());
  }

  sqlite3 *db_handle() {
    assert(stmt_);
    return sqlite3_db_handle(stmt_);
  }

  //
  // Parameter binding helpers (NOTE: index is 0-based)
  //

  int bind_parameter_count() {
    assert(stmt_);
    return sqlite3_bind_parameter_count(stmt_);
  }

  typedef void (*Function)(void*);

  int bind_blob(int index, std::vector<uint8> *blob) {
    if (blob && (blob->size() > 0)) {
      const void *value = &(*blob)[0];
      size_t len = blob->size();
      return bind_blob(index, value, len);
    } else {
      return bind_null(index);
    }
  }

  int bind_blob(int index, const void *value, int value_len) {
     return bind_blob(index, value, value_len, SQLITE_TRANSIENT);
  }

  int bind_blob(int index, const void *value, int value_len, Function dtor) {
    assert(stmt_);
    return sqlite3_bind_blob(stmt_, index + 1, value, value_len, dtor);
  }

  int bind_double(int index, double value) {
    assert(stmt_);
    return sqlite3_bind_double(stmt_, index + 1, value);
  }

  int bind_int(int index, int value) {
    assert(stmt_);
    return sqlite3_bind_int(stmt_, index + 1, value);
  }

  int bind_int64(int index, sqlite_int64 value) {
    assert(stmt_);
    return sqlite3_bind_int64(stmt_, index + 1, value);
  }

  int bind_null(int index) {
    assert(stmt_);
    return sqlite3_bind_null(stmt_, index + 1);
  }

  int bind_text16(int index, const char16 *value) {
    return bind_text16(index, value, -1, SQLITE_TRANSIENT);
  }

  // value_len is number of characters or may be negative
  // a for null-terminated value string
  int bind_text16(int index, const char16 *value, int value_len) {
    return bind_text16(index, value, value_len, SQLITE_TRANSIENT);
  }

  // value_len is number of characters or may be negative
  // a for null-terminated value string
  int bind_text16(int index, const char16 *value, int value_len,
                  Function dtor) {
    assert(stmt_);
    value_len *= sizeof(char16);
    return sqlite3_bind_text16(stmt_, index + 1, value, value_len, dtor);
  }

  int bind_value(int index, const sqlite3_value *value) {
    assert(stmt_);
    return sqlite3_bind_value(stmt_, index + 1, value);
  }

  //
  // Column helpers (NOTE: index is 0-based)
  //

  int column_count() {
    assert(stmt_);
    return sqlite3_column_count(stmt_);
  }

  int column_type(int index) {
    assert(stmt_);
    return sqlite3_column_type(stmt_, index);
  }

  const char16 *column_name16(int index) {
    assert(stmt_);
    return static_cast<const char16*>( sqlite3_column_name16(stmt_, index) );
  }

  const void *column_blob(int index) {
    assert(stmt_);
    return sqlite3_column_blob(stmt_, index);
  }

  bool column_blob_as_vector(int index, std::vector<uint8> *blob) {
    assert(stmt_);
    const void *p = column_blob(index);
    size_t len = column_bytes(index);
    blob->resize(len);
    if (blob->size() != len) {
      return false;
    }
    if (blob->size() > 0) {
      memcpy(&(*blob)[0], p, len);
    }
    return true;
  }

  int column_bytes(int index) {
    assert(stmt_);
    return sqlite3_column_bytes(stmt_, index);
  }

  int column_bytes16(int index) {
    assert(stmt_);
    return sqlite3_column_bytes16(stmt_, index);
  }

  double column_double(int index) {
    assert(stmt_);
    return sqlite3_column_double(stmt_, index);
  }

  int column_int(int index) {
    assert(stmt_);
    return sqlite3_column_int(stmt_, index);
  }

  sqlite_int64 column_int64(int index) {
    assert(stmt_);
    return sqlite3_column_int64(stmt_, index);
  }

  // Returns the text value for this column or NULL if no value has been set
  const char16 *column_text16(int index) {
    assert(stmt_);
    return static_cast<const char16*>( sqlite3_column_text16(stmt_, index) );
  }

  // Returns the text value for this column or an empty string if no value
  // has been set
  const char16 *column_text16_safe(int index) {
    assert(stmt_);
    const char16 *s = column_text16(index);
    return s ? s : STRING16(L"");
  }

 private:
  DISALLOW_EVIL_CONSTRUCTORS(SQLStatement);
};

#endif  // GEARS_BASE_COMMON_SQLITE_WRAPPER_H__
