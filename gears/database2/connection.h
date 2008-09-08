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

#ifndef GEARS_DATABASE2_CONNECTION_H__
#define GEARS_DATABASE2_CONNECTION_H__

#include <vector>

#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/string16.h"
#include "third_party/scoped_ptr/scoped_ptr.h"
#include "gears/database2/database2_metadata.h"
#include "third_party/sqlite_google/preprocessed/sqlite3.h"

// used to store value-type pairs
struct Database2Variant {
  Database2Variant() : type(JSPARAM_NULL) {}
  ~Database2Variant() {
    if (type == JSPARAM_STRING16) { delete string_value; }
  }
 
  JsParamType type;
  union {
    int int_value;
    double double_value;
    std::string16 *string_value;
  };
};

// Represents a consumer of results, produced by Connection::Execute method.
// Conceptually, it is a forward-only cursor over a result set. The cursor
// allows creating new rows and assigning the values in one row at a time.
// The proper execution sequence is:
// 1) Init -- to initialize the result set. This method can be only called once
//    for a given instance. Re-initialization is not supported.
// 2) HandleNewRow -- to create a new row (no rows exist in a newly initialized
//    result set.
// 3) HandleColumn* -- to set a value in a row. The index value must be between 
//    0 and column_count, specified in the Init
class Database2RowHandlerInterface {
 public:
  // initialize the handler with an array of column names
  // the method takes ownership of the array
  virtual void Init(int column_count, std::string16 *column_names) = 0;
  virtual void HandleNewRow() = 0;
  virtual bool HandleColumnInt(int index, int value) = 0;
  virtual bool HandleColumnDouble(int index, double value) = 0;
  // The implementation must copy the string value argument.
  virtual bool HandleColumnString(int index, const std::string16 &value) = 0;
  virtual bool HandleColumnNull(int index) = 0;
  virtual void HandleStats(int64 last_insert_rowid, int rows_affected) = 0;
};

// Encapsulates database operations, opens and closes database connection
class Database2Connection : public RefCounted {
 public:
  Database2Connection(const SecurityOrigin &origin,
                      const std::string16 &filename,
                      int version_cookie,
                      Database2Metadata *database2_metadata) :
    handle_(NULL), origin_(origin), filename_(filename),
    version_cookie_(version_cookie_), database2_metadata_(database2_metadata),
    bogus_version_(false) {}
  ~Database2Connection() {
    if (handle_) {
      sqlite3_close(handle_);
    }
  }

  bool Execute(const std::string16 &statement,
               int num_arguments,
               Database2Variant *arguments,
               Database2RowHandlerInterface *row_handler);
  bool Begin();
  void Rollback();
  bool Commit();

  int error_code() const { return error_code_; }
  const std::string16 &error_message() const { return error_message_; }

 private:
  bool OpenIfNecessary();
  // populates error code and error message fields using sqlite_wrapper helpers
  // and if the sqlite3 returned SQLITE_CORRUPT code, poisons the database
  void SetAndHandleError(int error_code,
                         const char16 *summary,
                         int sqlite_result_code);

  sqlite3 *handle_;
  SecurityOrigin origin_;
  std::string16 filename_;
  int version_cookie_;
  Database2Metadata *database2_metadata_;
  bool bogus_version_;
  std::string16 error_message_;
  int error_code_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2Connection);
};

// Used for marshaling of results from the database thread to the main thread.
// Implements a row handler that stores all of supplied data, and a CopyTo
// method to apply the data to another row handler.
class Database2BufferingRowHandler : public Database2RowHandlerInterface {
 public:
  Database2BufferingRowHandler() : column_count_(0) {}
  ~Database2BufferingRowHandler() {
    // remove all rows
    for(unsigned int i = 0; i < rows_.size(); ++i) {
      delete[] rows_[i];
    }
  }

  // Database2RowHandlerInterface
  virtual void Init(int column_count, std::string16 *column_names);
  virtual void HandleNewRow();
  virtual bool HandleColumnInt(int index, int value);
  virtual bool HandleColumnDouble(int index, double value);
  virtual bool HandleColumnString(int index, const std::string16 &value);
  virtual bool HandleColumnNull(int index);
  virtual void HandleStats(int64 last_insert_rowid, int rows_affected);

  bool CopyTo(Database2RowHandlerInterface *target);
 private:

  std::vector<Database2Variant*> rows_;
  scoped_array<std::string16> column_names_;
  int rows_affected_;
  int64 last_insert_rowid_;
  int column_count_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2BufferingRowHandler);
};

#endif // GEARS_DATABASE2_CONNECTION_H__
