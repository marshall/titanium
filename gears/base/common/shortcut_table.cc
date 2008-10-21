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

#include "gears/base/common/shortcut_table.h"

ShortcutTable::ShortcutTable(SQLDatabase *db)
    : db_(db) {
}

bool ShortcutTable::MaybeCreateTableVersion4() {
  SQLTransaction transaction(db_, "ShortcutTable::MaybeCreateTableVersion4");
  if (!transaction.Begin()) {
    return false;
  }

  // TODO(shess) I don't think "IF NOT EXISTS" is warranted below.  If
  // the version indicates that the table should exist, it should
  // exist, if the version indicates that it doesn't exist, then we
  // have a schema error somewhere, and what exists may not be
  // trustworthy.  But I'm too frightened to fix it, because it's
  // possible that other code depends on this behavior.  Should do a
  // thorough review of this.

  // The set of shortcuts, one per Origin/Name.
  const char *sql = "CREATE TABLE IF NOT EXISTS Shortcut ("
    " ShortcutID INTEGER PRIMARY KEY, "
    " Origin TEXT NOT NULL, Name TEXT NOT NULL, "
    " AppUrl TEXT NOT NULL, Msg TEXT NOT NULL, "
    " UNIQUE (Origin, Name)"
    ")";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::MaybeCreateTableVersion4 create Shortcut "
      "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  // A set of icon urls for each shortcut.
  sql = "CREATE TABLE IF NOT EXISTS ShortcutIcon "
    "(ShortcutID INTEGER NOT NULL, IconUrl TEXT NOT NULL,"
    " PRIMARY KEY (ShortcutID, IconUrl))";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::MaybeCreateTableVersion4 create ShortcutIcon "
      "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  return transaction.Commit();
}

bool ShortcutTable::MaybeCreateTableVersion5() {
  SQLTransaction transaction(db_, "ShortcutTable::MaybeCreateTableVersion5");
  if (!transaction.Begin()) {
    return false;
  }

  // TODO(shess) I don't think "IF NOT EXISTS" is warranted below.  If
  // the version indicates that the table should exist, it should
  // exist, if the version indicates that it doesn't exist, then we
  // have a schema error somewhere, and what exists may not be
  // trustworthy.  But I'm too frightened to fix it, because it's
  // possible that other code depends on this behavior.  Should do a
  // thorough review of this.

  // The set of shortcuts, one per Origin/Name.
  const char *sql = "CREATE TABLE IF NOT EXISTS Shortcut ("
                    " ShortcutID INTEGER PRIMARY KEY, "
                    " Origin TEXT NOT NULL, Name TEXT NOT NULL, "
                    " AppUrl TEXT NOT NULL, Msg TEXT NOT NULL, "
                    " Allow INTEGER NOT NULL, "
                    " UNIQUE (Origin, Name)"
                    ")";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::MaybeCreateTableVersion5 create Shortcut "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  // A set of icon urls for each shortcut.
  sql = "CREATE TABLE IF NOT EXISTS ShortcutIcon "
        "(ShortcutID INTEGER NOT NULL, IconUrl TEXT NOT NULL,"
        " PRIMARY KEY (ShortcutID, IconUrl))";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::MaybeCreateTableVersion5 create ShortcutIcon "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  return transaction.Commit();
}

bool ShortcutTable::MaybeCreateTableVersion6() {
  SQLTransaction transaction(db_, "ShortcutTable::MaybeCreateTableVersion6");
  if (!transaction.Begin()) {
    return false;
  }

  // TODO(shess) I don't think "IF NOT EXISTS" is warranted below.  If
  // the version indicates that the table should exist, it should
  // exist, if the version indicates that it doesn't exist, then we
  // have a schema error somewhere, and what exists may not be
  // trustworthy.  But I'm too frightened to fix it, because it's
  // possible that other code depends on this behavior.  Should do a
  // thorough review of this.

  // The set of shortcuts, one per Origin/Name.
  const char *sql = "CREATE TABLE IF NOT EXISTS Shortcut ("
                    " ShortcutID INTEGER PRIMARY KEY, "
                    " Origin TEXT NOT NULL, Name TEXT NOT NULL, "
                    " AppUrl TEXT NOT NULL, Msg TEXT NOT NULL, "
                    " Icon16x16Url TEXT NOT NULL,"
                    " Icon32x32Url TEXT NOT NULL,"
                    " Icon48x48Url TEXT NOT NULL,"
                    " Icon128x128Url TEXT NOT NULL,"
                    " Allow INTEGER NOT NULL, "
                    " UNIQUE (Origin, Name)"
                    ")";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::MaybeCreateTableVersion6 create Shortcut "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  return transaction.Commit();
}

bool ShortcutTable::MaybeCreateTableLatestVersion() {
  return MaybeCreateTableVersion6();
}

bool ShortcutTable::UpgradeToVersion3() {
  const char *sql = "CREATE TABLE IF NOT EXISTS Shortcut ("
                    " Origin TEXT, Name TEXT, "
                    " AppUrl TEXT, IcoUrl TEXT, Msg TEXT, "
                    " PRIMARY KEY (Origin, Name)"
                    ")";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::CreateTableVersion3 create Shortcut "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }
  return true;
}

// Shift the version-3 Shortcut table aside, create version-4 tables,
// populate them from the old table, and drop the old table.
bool ShortcutTable::UpgradeFromVersion3ToVersion4() {
  SQLTransaction transaction(db_,
      "ShortcutTable::UpgradeFromVersion3ToVersion4");
  if (!transaction.Begin()) {
    return false;
  }

  const char *sql = "ALTER TABLE Shortcut RENAME TO ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion3ToVersion4 rename "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  if (!MaybeCreateTableVersion4()) {
    LOG(("ShortcutTable::UpgradeFromVersion3ToVersion4 create failed"));
    return false;
  }

  // Use the existing rowid as ShortcutID for consistency when
  // populating the new tables.
  sql = "INSERT INTO Shortcut "
        "SELECT rowid AS ShortcutID, Origin, Name, "
        "       AppUrl, Msg FROM ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion3ToVersion4 populate Shortcut "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  sql = "INSERT INTO ShortcutIcon "
        "SELECT rowid AS ShortcutID, IcoUrl AS IconUrl FROM ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion3ToVersion4 populate ShortcutIcon "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  sql = "DROP TABLE ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion3ToVersion4 drop old "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  return transaction.Commit();
}

// Shift the version-4 Shortcut table aside, create version-5 tables,
// populate them from the old table, and drop the old table.
bool ShortcutTable::UpgradeFromVersion4ToVersion5() {
  SQLTransaction transaction(db_,
    "ShortcutTable::UpgradeFromVersion4ToVersion5");
  if (!transaction.Begin()) {
    return false;
  }

  const char *sql = "ALTER TABLE Shortcut RENAME TO ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion4ToVersion5 rename "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  if (!MaybeCreateTableVersion5()) {
    LOG(("ShortcutTable::UpgradeFromVersion4ToVersion5create failed"));
    return false;
  }

  // Use the existing rowid as ShortcutID for consistency when
  // populating the new tables.
  sql = "INSERT INTO Shortcut "
    "SELECT rowid AS ShortcutID, Origin, Name, "
    "       AppUrl, Msg, 1 as Allow FROM ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion4ToVersion5 populate Shortcut "
      "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  sql = "DROP TABLE ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion4ToVersion5 drop old "
      "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  return transaction.Commit();
}

// Shift the version-5 Shortcut tables aside, create version-6 table,
// populate them from the old tables, and drop the old tables.
bool ShortcutTable::UpgradeFromVersion5ToVersion6() {
  SQLTransaction transaction(db_,
    "ShortcutTable::UpgradeFromVersion5ToVersion6");
  if (!transaction.Begin()) {
    return false;
  }

  const char *sql = "ALTER TABLE Shortcut RENAME TO ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion5ToVersion6 rename "
         "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  if (!MaybeCreateTableVersion6()) {
    LOG(("ShortcutTable::UpgradeFromVersion5ToVersion6 create failed"));
    return false;
  }

  // Use the existing rowid as ShortcutID for consistency when
  // populating the new tables.
  sql = "INSERT INTO Shortcut "
    "SELECT rowid AS ShortcutID, Origin, Name, "
    "       '' AS Icon16x16Url, '' AS Icon32x32Url, '' AS Icon48x48Url, "
    "       '' AS Icon128x128Url, AppUrl, Msg, Allow FROM ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion5ToVersion6 populate Shortcut "
      "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  sql = "DROP TABLE ShortcutOld";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion5ToVersion6 drop old "
      "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  sql = "DROP TABLE ShortcutIcon";
  if (SQLITE_OK != db_->Execute(sql)) {
    LOG(("ShortcutTable::UpgradeFromVersion5ToVersion6 drop ShortcutIcon "
      "unable to execute: %d", db_->GetErrorCode()));
    return false;
  }

  return transaction.Commit();
}

bool ShortcutTable::SetShortcut(const char16 *origin, const char16 *name,
                                const char16 *app_url,
                                const char16 *icon16x16_url,
                                const char16 *icon32x32_url,
                                const char16 *icon48x48_url,
                                const char16 *icon128x128_url,
                                const char16 *msg,
                                const bool allow) {
  SQLTransaction transaction(db_, "ShortcutTable::SetShortcut");
  if (!transaction.Begin()) {
    return false;
  }

  // Clear out the existing shortcut.
  if (!DeleteShortcut(origin, name)) {
    LOG(("ShortcutTable::SetShortcut failed to delete old shortcut"));
    return false;
  }

  const char16 *sql = STRING16(L"INSERT INTO Shortcut "
                               L"(Origin, Name, AppUrl, Msg, Allow, "
                               L"Icon16x16Url, Icon32x32Url, "
                               L"Icon48x48Url, Icon128x128Url) "
                               L"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("ShortcutTable::SetShortcut unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("ShortcutTable::SetShortcut unable to bind origin: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, name)) {
    LOG(("ShortcutTable::SetShortcut unable to bind name: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(2, app_url)) {
    LOG(("ShortcutTable::SetShortcut unable to bind app_url: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(3, msg)) {
    LOG(("ShortcutTable::SetShortcut unable to bind msg: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_int(4, allow ? 1 : 0)) {
    LOG(("ShortcutTable::SetShortcut unable to bind Allow: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(5, icon16x16_url)) {
    LOG(("ShortcutTable::SetShortcut unable to bind icon16x16_url: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(6, icon32x32_url)) {
    LOG(("ShortcutTable::SetShortcut unable to bind icon32x32_url: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(7, icon48x48_url)) {
    LOG(("ShortcutTable::SetShortcut unable to bind icon48x48_url: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(8, icon128x128_url)) {
    LOG(("ShortcutTable::SetShortcut unable to bind icon128x128_url: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("ShortcutTable::SetShortcut unable to step: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  return transaction.Commit();
}

bool ShortcutTable::
GetOriginsWithShortcuts(std::vector<std::string16> *result) {
  const char16 *sql = STRING16(L"SELECT DISTINCT(Origin) FROM Shortcut");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("ShortcutTable::GetOriginsWithShortcuts unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  std::vector<std::string16> temp;
  int rv;
  while (SQLITE_ROW == (rv = statement.step())) {
    std::string16 origin(statement.column_text16_safe(0));
    temp.push_back(origin);
  }

  if (SQLITE_DONE != rv) {
    LOG(("ShortcutTable::GetOriginsWithShortcuts unable to step: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  result->swap(temp);
  return true;
}

bool ShortcutTable::GetOriginShortcuts(const char16 *origin,
                                       std::vector<std::string16> *names) {
  const char16 *sql = STRING16(L"SELECT Name FROM Shortcut WHERE Origin = ?");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("ShortcutTable::GetOriginShortcuts unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("ShortcutTable::GetOriginShortcuts unable to bind origin: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  std::vector<std::string16> temp;
  int rv;
  while (SQLITE_ROW == (rv = statement.step())) {
    std::string16 name(statement.column_text16_safe(0));
    temp.push_back(name);
  }

  if (SQLITE_DONE != rv) {
    LOG(("ShortcutTable::GetOriginShortcuts unable to step: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  names->swap(temp);
  return true;
}

bool ShortcutTable::GetShortcut(const char16 *origin, const char16 *name,
                                std::string16 *app_url,
                                std::string16 *icon16x16_url,
                                std::string16 *icon32x32_url,
                                std::string16 *icon48x48_url,
                                std::string16 *icon128x128_url,
                                std::string16 *msg,
                                bool *allow) {
  // This query is a little convoluted in the interests of avoiding an
  // explicit transaction.  There will be one row for every associated
  // ShortcutIcon, with the LEFT JOIN forcing there to always be at
  // least one row if there is a matching Origin/Name.  The AppUrl and
  // Msg will be the same for every row.
  const char16 *sql = STRING16(L"SELECT AppUrl, Msg, Allow, "
                               L"Icon16x16Url, Icon32x32Url, "
                               L"Icon48x48Url, Icon128x128Url "
                               L"FROM Shortcut "
                               L"WHERE Origin = ? AND Name = ? ");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("ShortcutTable::GetShortcut unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("ShortcutTable::GetShortcut unable to bind origin: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, name)) {
    LOG(("ShortcutTable::GetShortcut unable to bind name: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  int rc = statement.step();
  if (SQLITE_DONE == rc) {
    return false;
  } else if (SQLITE_ROW != rc) {
    LOG(("ShortcutTable::GetShortcut results error: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  // Get these from the first row, they're always the same.
  *app_url = statement.column_text16_safe(0);
  *msg = statement.column_text16_safe(1);
  *allow = (statement.column_int(2) != 0);
  *icon16x16_url = statement.column_text16_safe(3);
  *icon32x32_url = statement.column_text16_safe(4);
  *icon48x48_url = statement.column_text16_safe(5);
  *icon128x128_url = statement.column_text16_safe(6);

  return true;
}

bool ShortcutTable::DeleteShortcut(const char16 *origin, const char16 *name) {
  const char16 *sql = STRING16(L"DELETE FROM Shortcut "
                               L"WHERE Origin = ? AND Name = ?");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("ShortcutTable::DeleteShortcut unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("ShortcutTable::DeleteShortcut unable to bind origin: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(1, name)) {
    LOG(("ShortcutTable::DeleteShortcut unable to bind name: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("ShortcutTable::DeleteShortcut unable to step: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  return true;
}

bool ShortcutTable::DeleteShortcuts(const char16 *origin) {
  const char16 *sql = STRING16(L"DELETE FROM Shortcut WHERE Origin = ?");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("ShortcutTable::DeleteShortcuts unable to prepare: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, origin)) {
    LOG(("ShortcutTable::DeleteShortcuts unable to bind origin: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("ShortcutTable::DeleteShortcuts unable to step: %d\n",
         db_->GetErrorCode()));
    return false;
  }

  return true;
}
