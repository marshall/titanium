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

#include "gears/base/common/paths.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/string_utils.h"
#include "gears/database2/database2_metadata.h"

Database2Metadata::Database2Metadata(SQLDatabase *db)
    : db_(db) {
}

bool Database2Metadata::MaybeCreateTableLatestVersion() {
  SQLTransaction transaction(db_,
      "Database2Metadata::MaybeCreateTableLatestVersion");
  if (!transaction.Begin()) {
    return false;
  }
  if (!CreateTableVersion8()) {
    return false;
  }
  return transaction.Commit();
}

bool Database2Metadata::CreateTableVersion8() {
  const char *sql[] = {
    "CREATE TABLE Database2Metadata ("
      " DatabaseID INTEGER PRIMARY KEY, "
      " Origin TEXT NOT NULL, "
      " Name TEXT NOT NULL, "
      " Version TEXT NOT NULL, "  // no version represented by empty string
      " VersionCookie INTEGER NOT NULL, "  // no version represented by zero
      " Filename TEXT NOT NULL DEFAULT '', "
      " IsCorrupt INTEGER NOT NULL, "
      " IsDeleted INTEGER NOT NULL, "
      " UNIQUE (Origin, Filename)"
      ")",
    "CREATE INDEX IxDatabase2Name ON Database2Metadata "
      "(Origin, Name)",
    "CREATE INDEX IxDatabase2Filename ON Database2Metadata "
      "(Origin, Filename)"
  };

  for (size_t i = 0; i < ARRAYSIZE(sql); ++i) {
    int sqlite_result = db_->Execute(sql[i]);
    if (SQLITE_OK != sqlite_result) {
      std::string error("Database2Metadata::CreateTableVersion8. ");
      error += "Failure on statement ";
      error += IntegerToString(i);
      error += ": ";
      error += db_->GetErrorMessage();
      LOG((error.c_str()));
      return false;
    }
  }

  return true;
}

// Helper to get current information about a database. Returns false on internal
// error. If the database was found, filename will also be non-empty.
static bool GetDatabaseInfo(SQLDatabase *db,
                            const SecurityOrigin &origin,
                            const std::string16 &database_name,
                            const std::string16 &expected_version,
                            std::string16 *filename,
                            std::string16 *version,
                            int *version_cookie,
                            bool *version_mismatch,
                            std::string16 *error) {
  const char16 *sql = STRING16(
      L"SELECT Filename, Version, VersionCookie "
      L" FROM Database2Metadata "
      L" WHERE Origin = ? AND Name = ? AND "
      L" IsDeleted <> 1 AND IsCorrupt <> 1");
  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db, sql)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin.url().c_str())) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, database_name.c_str())) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  int rc = statement.step();
  if (SQLITE_DONE == rc) {
    return true;
  } else if (SQLITE_ROW != rc) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  std::string16 found_filename(statement.column_text16(0));
  if (found_filename.empty()) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  // OK for these to be empty.
  std::string16 found_version(statement.column_text16(1));
  int found_version_cookie = statement.column_int(2);

  // This should not happen. There should only ever be one active database per
  // origin/name pair.
  if (SQLITE_DONE != statement.step()) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (expected_version.empty() || found_version == expected_version) {
    *filename = found_filename;
    *version = found_version;
    *version_cookie = found_version_cookie;
  } else {
    *version_mismatch = true;
  }

  return true;
}

// Inserts information about a new database into the table.
static bool InsertDatabaseInfo(SQLDatabase *db,
                               const SecurityOrigin &origin,
                               const std::string16 &database_name,
                               const std::string16 &version,
                               int version_cookie,
                               std::string16 *filename,
                               std::string16 *error) {
  const char16 *sql = STRING16(
      L"INSERT INTO Database2Metadata "
      L"(Origin, Name, Version, VersionCookie, IsCorrupt, IsDeleted) "
      L"VALUES (?, ?, ?, ?, 0, 0)");
  SQLStatement insert_statement;
  if (SQLITE_OK != insert_statement.prepare16(db, sql)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != insert_statement.bind_text16(0, origin.url().c_str())) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != insert_statement.bind_text16(1, database_name.c_str())) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != insert_statement.bind_text16(2, version.c_str())) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != insert_statement.bind_int(3, version_cookie)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_DONE != insert_statement.step()) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  int64 primary_key = insert_statement.last_insert_rowid();
  
  // Now that we know the primary key, we can construct the filename.
  std::string16 suffix;
  suffix += STRING16(L"[");
  suffix += Integer64ToString16(primary_key);
  suffix += STRING16(L"]");
  suffix += STRING16(L"#database2");

  // limit the size of the full filename.
  std::string16 filename_temp(database_name);
  if (filename_temp.length() + suffix.length() > kUserPathComponentMaxChars) {
    filename_temp.resize(kUserPathComponentMaxChars - suffix.length());
  }

  EnsureStringValidPathComponent(filename_temp, true);
  filename_temp += suffix;

  // Now update the row in the database with the filename
  sql = STRING16(L"UPDATE Database2Metadata SET Filename = ? "
                 L"WHERE DatabaseId = ?");

  SQLStatement update_statement;
  if (SQLITE_OK != update_statement.prepare16(db, sql)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != update_statement.bind_text16(0, filename_temp.c_str())) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_OK != update_statement.bind_int64(1, primary_key)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  if (SQLITE_DONE != update_statement.step()) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  *filename = filename_temp;
  return true;
}

bool Database2Metadata::GetDatabaseInfo(const SecurityOrigin &origin,
                                        const std::string16 &database_name,
                                        const std::string16 &version,
                                        std::string16 *filename,
                                        std::string16 *found_version,
                                        int *version_cookie,
                                        std::string16 *error) {
  assert(filename->length() == 0);
  assert(error->length() == 0);

  // As an optimization, we check for this filename once without a transaction.
  bool version_mismatch = false;
  if (!::GetDatabaseInfo(db_, origin, database_name, version, filename,
                         found_version, version_cookie, &version_mismatch,
                         error)) {
    return false;
  }
  if (!filename->empty() || version_mismatch) {
    return true;
  }

  // We didn't find one, so start a transaction and do it right.
  SQLTransaction transaction(db_, "Database2Metadata::GetDatabaseFilename");
  if (!transaction.Begin()) {
    return false;
  }

  // Try again to find the database, in case someone changed it in the meantime.
  if (!::GetDatabaseInfo(db_, origin, database_name, version, filename,
                         found_version, version_cookie, &version_mismatch,
                         error)) {
    return false;
  }
  if (!filename->empty() || version_mismatch) {
    return true;
  }

  // Still no live versions, so create one.
  const int kInitialVersionCookie = 1; 
  if (!InsertDatabaseInfo(db_, origin, database_name, version,
                          kInitialVersionCookie, filename, error)) {
    return false;
  }
  if (!transaction.Commit()) {
    return false;
  }

  *found_version = version;
  *version_cookie = kInitialVersionCookie;
  return true;
}

bool Database2Metadata::MarkDatabaseCorrupt(const SecurityOrigin &origin,
                                            const std::string16 &filename) {
  const char16 *sql = STRING16(
      L"UPDATE Database2Metadata SET IsCorrupt = 1 "
      L" WHERE Origin = ? AND Filename = ?");
  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("Database2Metadata::MarkDatabaseCorrupt unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin.url().c_str())) {
    LOG(("Database2Metadata::MarkDatabaseCorrupt unable to bind origin: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, filename.c_str())) {
    LOG(("Database2Metadata::MarkDatabaseCorrupt "
         "unable to bind filename: %d\n", db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("Database2Metadata::MarkDatabaseCorrupt unable to step: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  return true;
}
