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

#ifndef GEARS_BASE_COMMON_NAME_VALUE_TABLE_H__
#define GEARS_BASE_COMMON_NAME_VALUE_TABLE_H__

#include "gears/base/common/sqlite_wrapper.h"

// Provides an API for storing name/value pairs in table of SQLDatabase.
class NameValueTable {
 public:
  // Creates an instance for the specified database and table name.
  NameValueTable(SQLDatabase *db_, const char16 *table_name);

  // Creates the table if it doesn't already exist.
  bool MaybeCreateTable();

  // Sets an int value.
  bool SetInt(const char16 *name, int value);

  // Gets an int value that was previously created with SetInt().
  bool GetInt(const char16 *name, int *value);

  // Sets a string value.
  bool SetString(const char16 *name, const char16 *value);

  // Gets a string value that was previously created with SetString().
  bool GetString(const char16 *name, std::string16 *value);

  // Check whether the specified name exists in the table.
  bool HasName(const char16 *name, bool *retval);
  
  // Gets a vector of names that match the given int value.
  bool FindNamesByIntValue(int value, std::vector<std::string16>* names);

  // Removes a value that was previously created with one of the Set methods.
  bool Clear(const char16 *name);

 private:
  // A pointer to the SQLDatabase our table will be created in.
  SQLDatabase *db_;

  // The name of the table we will be storing name/value pairs in.
  std::string16 table_name_;

  // Prepares a SQL statement for execution.
  // TODO(aa): The C++ style guide says never to pass un-const references, but
  // SQLStatement & operator is overloaded to return the internal sqlite3_stmt
  // so that I cannot get a pointer to SQLStatement. Figure out what to do.
  bool PrepareStatement(SQLStatement &stmt, const char16 *sql_prefix,
                        const char16 *sql_suffix, const char16 *name);

  // Prepares a SQL statement for one of the Set* methods for execution.
  bool PrepareSetStatement(SQLStatement &stmt, const char16 *name) {
    return PrepareStatement(
      stmt, STRING16(L"REPLACE INTO "), STRING16(L" VALUES (?, ?)"), name);
  }

  // Prepares a SQL statement for one of the Get* methods for execution.
  bool PrepareGetStatement(SQLStatement &stmt, const char16 *name) {
    return PrepareStatement(
      stmt, STRING16(L"SELECT Value FROM "), STRING16(L" WHERE Name = ?"),
      name);
  }

  DISALLOW_EVIL_CONSTRUCTORS(NameValueTable);
};

#endif  // GEARS_BASE_COMMON_NAME_VALUE_TABLE_H__
