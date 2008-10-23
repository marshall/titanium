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

#ifndef GEARS_BASE_COMMON_DATABASE_NAME_TABLE_H__
#define GEARS_BASE_COMMON_DATABASE_NAME_TABLE_H__

#include "gears/base/common/security_model.h"
#include "gears/base/common/sqlite_wrapper.h"

// Maps database names to basenames in the filesystem.
class DatabaseNameTable {
 public:
  DatabaseNameTable(SQLDatabase *db);

  bool MaybeCreateTableVersion7();

  // Upgrades to version 7 schema (this table did not previously
  // exist).
  bool UpgradeToVersion7();

  // Creates the most recent version of the table if it doesn't
  // already exist.
  bool MaybeCreateTableLatestVersion() {
    return MaybeCreateTableVersion7();
  }

  // For a given database_name, fills basename with the name of the
  // file to use in origin's directory, and returns true if
  // successful.
  bool GetDatabaseBasename(const char16 *origin, const char16 *database_name,
                           std::string16 *basename);

  // Marks the given database basename corrupt so that future calls to
  // GetDatabaseBasename will no longer return it.  The basename is
  // required because another thread of control could have already
  // invalidated the database for the origin.
  bool MarkDatabaseCorrupt(const char16 *origin, const char16 *database_name,
                           const char16 *basename);

  // Deletes all databases associated with the given origin.  Always
  // attempts to delete as much as possible, so no value is returned.
  void DeleteDatabasesForOrigin(const SecurityOrigin &origin);

 private:
  // A pointer to the SQLDatabase our table lives in.
  SQLDatabase *db_;

  DISALLOW_EVIL_CONSTRUCTORS(DatabaseNameTable);
};

#endif  // GEARS_BASE_COMMON_DATABASE_NAME_TABLE_H__
