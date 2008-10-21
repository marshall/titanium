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

#include "gears/base/common/database_name_table.h"

#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"

DatabaseNameTable::DatabaseNameTable(SQLDatabase *db)
    : db_(db) {
}

bool DatabaseNameTable::MaybeCreateTableVersion7() {
  const char *sql = "CREATE TABLE IF NOT EXISTS DatabaseNames ("
                    " DatabaseID INTEGER PRIMARY KEY,"
                    " Origin TEXT NOT NULL,"
                    " Name TEXT NOT NULL,"
                    " Version INTEGER NOT NULL,"
                    " Basename TEXT NOT NULL,"
                    " IsCorrupt INTEGER NOT NULL,"
                    " IsDeleted INTEGER NOT NULL,"
                    " UNIQUE (Origin, Name, Version),"
                    " UNIQUE (Origin, Basename)"
                    ")";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("DatabaseNameTable::MaybeCreateTableVersion7 create "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  return true;
}

bool DatabaseNameTable::UpgradeToVersion7() {
  return MaybeCreateTableVersion7();
}

// GetDatabaseBasename() helper function.  Get the currently active
// basename for origin/database_name, setting *found to true if one is
// found, and returns true.  *found is set to false and true returned
// if no active basename is (successfully) found.  Returns false in
// case of error running SQL code.
static bool BasenameHelper(SQLDatabase *db,
                           const char16 *origin,
                           const char16 *database_name,
                           std::string16 *basename,
                           bool *found) {
  const char16 *sql = STRING16(
      L"SELECT Basename FROM DatabaseNames "
      L" WHERE Origin = ? AND Name = ? AND IsDeleted <> 1");
  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db, sql)) {
    LOG(("BasenameHelper unable to prepare: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("BasenameHelper unable to bind origin: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, database_name)) {
    LOG(("BasenameHelper unable to bind name: %d\n", db->GetErrorCode()));
    return false;
  }

  int rc = statement.step();
  if (SQLITE_DONE == rc) {
    *found = false;
    return true;
  } else if (SQLITE_ROW != rc) {
    LOG(("BasenameHelper unable to step: %d\n", db->GetErrorCode()));
    *found = false;
    return false;
  }

  *basename = statement.column_text16_safe(0);

  // Multiple versions - which should we choose?  Impossible to choose
  // correctly, so you lose.
  // TODO(shess): This is a sticky wicket, you cannot recover.  It may
  // be better to make a pragmatic decision, instead.  One option
  // would be to mark all outstanding versions corrupt so we start
  // over.  Another option would be to mark all but the highest
  // version corrupt, so at least one database works.  Either way, if
  // we don't poison the database we might cause problems if other
  // users have a handle open.
  if (SQLITE_DONE != statement.step()) {
    LOG(("BasenameHelper detected invalid data: %d\n", db->GetErrorCode()));
    *found = false;
    return false;
  }

  *found = true;
  return true;
}

// GetDatabaseBasename() helper function.  Get the next database
// version for the origin/database_name, and return it in *version.
// Returns false in case of SQL error, true in case of success.
static bool NextVersionHelper(SQLDatabase *db,
                              const char16 *origin,
                              const char16 *database_name,
                              int *version) {
  const char16 *sql = STRING16(
      L"SELECT IFNULL(MAX(Version) + 1, 0) FROM DatabaseNames "
      L" WHERE Origin = ? AND Name = ?");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db, sql)) {
    LOG(("NextVersionHelper unable to prepare: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("NextVersionHelper unable to bind origin: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, database_name)) {
    LOG(("NextVersionHelper unable to bind name: %d\n", db->GetErrorCode()));
    return false;
  }

  int rc = statement.step();
  if (SQLITE_DONE == rc) {
    // This case should not be possible.
    assert(SQLITE_DONE != rc);
    *version = 0;
    return true;
  } else if (SQLITE_ROW != rc) {
    LOG(("NextVersionHelper unable to step: %d\n", db->GetErrorCode()));
    return false;
  }

  *version = statement.column_int(0);
  return true;
}

// GetDatabaseBasename() helper function.  Insert a new entry made up
// of the passed parameters.  Returns false in case of SQL error, true
// in case of success.
static bool InsertHelper(SQLDatabase *db,
                         const char16 *origin, const char16 *database_name,
                         int version, const char16 *basename) {
  const char16 *sql = STRING16(
      L"INSERT INTO DatabaseNames "
      L"(Origin, Name, Version, Basename, IsCorrupt, IsDeleted) "
      L"VALUES (?, ?, ?, ?, 0, 0)");
  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db, sql)) {
    LOG(("InsertHelper unable to prepare : %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("InsertHelper unable to bind origin: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, database_name)) {
    LOG(("InsertHelper unable to bind name: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_int(2, version)) {
    LOG(("InsertHelper unable to bind version: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(3, basename)) {
    LOG(("InsertHelper unable to bind filename: %d\n", db->GetErrorCode()));
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("InsertHelper unable to step: %d\n", db->GetErrorCode()));
    return false;
  }

  return true;
}

bool DatabaseNameTable::GetDatabaseBasename(const char16 *origin,
                                            const char16 *database_name,
                                            std::string16 *basename) {
  bool found = false;
  if (!BasenameHelper(db_, origin, database_name, basename, &found)) {
    return false;
  }

  if (found) {
    return true;
  }

  // If we did not find a basename for this origin/database_name, add
  // one.  The transaction is needed to prevent other clients from
  // changing things while we work.  The call to BasenameHelper() in
  // the transaction handles the possibility that someone else already
  // bumped to a new version.
  SQLTransaction transaction(db_, "DatabaseNameTable::GetDatabaseBasename");
  if (!transaction.Begin()) {
    return false;
  }

  if (!BasenameHelper(db_, origin, database_name, basename, &found)) {
    return false;
  }

  // Still no live versions, so create one with the next higher version
  // number.
  if (!found) {
    int version = 0;
    if (!NextVersionHelper(db_, origin, database_name, &version)) {
      return false;
    }

    std::string16 f(database_name);
    if (version == 0) {
      // The version-0 filename needs to match the name it would have
      // had before this mapping table was introduced.
      f += STRING16(L"#database");
    } else {
      // The other modules would imply <db_name>[<version>]#database.
      // Then db_name "x" version 1, and db_name "x[1]" version 0
      // would both be "x[1]#database".
      f += STRING16(L"#database[");
      f += IntegerToString16(version);
      f += STRING16(L"]");
    }

    if (!InsertHelper(db_, origin, database_name, version, f.c_str())) {
      return false;
    }

    *basename = f;
  }
  return transaction.Commit();
}

bool DatabaseNameTable::MarkDatabaseCorrupt(const char16 *origin,
                                            const char16 *database_name,
                                            const char16 *filename) {
  const char16 *sql = STRING16(
      L"UPDATE DatabaseNames SET IsCorrupt = 1, IsDeleted = 1 "
      L" WHERE Origin = ? AND Name = ? AND Basename = ?");
  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("DatabaseNameTable::MarkDatabaseCorrupt unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("DatabaseNameTable::MarkDatabaseCorrupt unable to bind origin: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, database_name)) {
    LOG(("DatabaseNameTable::MarkDatabaseCorrupt unable to bind name: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(2, filename)) {
    LOG(("DatabaseNameTable::MarkDatabaseCorrupt "
         "unable to bind filename: %d\n", db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("DatabaseNameTable::MarkDatabaseCorrupt unable to step: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  return true;
}

void DatabaseNameTable::DeleteDatabasesForOrigin(const SecurityOrigin &origin) {
  // First mark all the databases as deleted.  Then try to remove all
  // files backing the deleted databases.
  //
  // If the first step fails, continue to the second step instead of
  // returning early.  During the second step, don't return on errors
  // either; simply continue to the next database.

  // Step 1: Mark databases as deleted.
  const char16 *update_sql = STRING16(
      L"UPDATE DatabaseNames SET IsDeleted = 1 WHERE Origin = ?");
  SQLStatement update_stmt;
  if (SQLITE_OK == update_stmt.prepare16(db_, update_sql) &&
      SQLITE_OK == update_stmt.bind_text16(0, origin.url().c_str()) &&
      SQLITE_DONE == update_stmt.step()) {
    // Hooray, it succeeded. Nothing more to do here.
  }

  // Step 2: Try to remove databases marked as deleted.
  const char16 *select_sql = STRING16(
      L"SELECT Basename FROM DatabaseNames"
      L" WHERE IsDeleted = 1 AND Origin = ?");
  SQLStatement select_stmt;
  if (SQLITE_OK == select_stmt.prepare16(db_, select_sql) &&
      SQLITE_OK == select_stmt.bind_text16(0, origin.url().c_str())) {

    std::string16 origin_data_dir;
    if (GetDataDirectory(origin, &origin_data_dir)) {
      origin_data_dir += kPathSeparator;

      while (SQLITE_ROW == select_stmt.step()) {
        std::string16 basename = select_stmt.column_text16_safe(0);
        std::string16 full_path = origin_data_dir + basename;
        File::Delete(full_path.c_str());  // ignore return value
      }
    }
  }
  
}
