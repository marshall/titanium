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

#include "gears/base/common/permissions_db.h"
#include "gears/base/common/message_service.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/thread_locals.h"
#include "gears/database2/database2_metadata.h"
#include "gears/localserver/common/localserver_db.h"

static const char16 *kDatabaseName = STRING16(L"permissions.db");
// TODO(aa): Rename this table "LocalDataAccess"
static const char16 *kLocalDataAccessTableName = STRING16(L"Access");
static const char16 *kLocationDataAccessTableName = STRING16(L"LocationAccess");
static const char16 *kSupressDialogsKeyName = STRING16(L"SupressDialogs");
// TODO(aa): Rename this table "Settings"
static const char16 *kVersionTableName = STRING16(L"VersionInfo");
static const char16 *kVersionKeyName = STRING16(L"Version");
static const int kCurrentVersion = 9;
static const int kOldestUpgradeableVersion = 1;

const char16 *PermissionsDB::kShortcutsChangedTopic = 
                  STRING16(L"base:permissions:shortcuts-changed");

const ThreadLocals::Slot PermissionsDB::kThreadLocalKey = ThreadLocals::Alloc();


PermissionsDB *PermissionsDB::GetDB() {
  if (ThreadLocals::HasValue(kThreadLocalKey)) {
    return reinterpret_cast<PermissionsDB*>(
        ThreadLocals::GetValue(kThreadLocalKey));
  }

  PermissionsDB *db = new PermissionsDB();

  // If we can't initialize, we store NULL in the map so that we don't keep
  // trying to Init() over and over.
  if (!db->Init()) {
    delete db;
    db = NULL;
  }

  ThreadLocals::SetValue(kThreadLocalKey, db, &DestroyDB);
  return db;
}


void PermissionsDB::DestroyDB(void *context) {
  PermissionsDB *db = reinterpret_cast<PermissionsDB*>(context);
  if (db) {
    delete db;
  }
}


PermissionsDB::PermissionsDB()
    : settings_table_(&db_, kVersionTableName),
      local_data_access_table_(&db_, kLocalDataAccessTableName),
      location_access_table_(&db_, kLocationDataAccessTableName),
      shortcut_table_(&db_),
      database_name_table_(&db_),
      database2_metadata_(&db_) {
}


bool PermissionsDB::Init() {
  // Initialize the database and tables
  if (!db_.Open(kDatabaseName)) {
    return false;
  }

  // Examine the contents of the database and determine if we have to
  // instantiate or updgrade the schema.
  int version = 0;
  settings_table_.GetInt(kVersionKeyName, &version);

  // if its the version we're expecting, great
  if (version == kCurrentVersion) {
    return true;
  }

  // Doing this in a transaction effectively locks the database file and
  // ensures that this is synchronized across all threads and processes
  SQLTransaction transaction(&db_, "PermissionsDB::Init");
  if (!transaction.Begin()) {
    return false;
  }

  // Fetch the version again in case someone else beat us to the
  // upgrade.
  settings_table_.GetInt(kVersionKeyName, &version);
  if (version == kCurrentVersion) {
    return true;
  } else if (version == 0) {
    // No database in place, create it.
    //
    // TODO(shess) Verify that this is true.  Is it _no_ database, or
    // is there a database which didn't have a version?  The latter
    // case would be masked by the CREATE IF NOT EXISTS statements
    // we're using.
    if (!CreateDatabase()) {
      return false;
    }
  } else if (version < kCurrentVersion) {
    if (!UpgradeDatabase(version)) {
      return false;
    }
  } else {
    // database is too new for this code.
    // TODO(aa): Surface this error in a better way.
    assert(false);
    return false;
  }

  // Double-check that we ended up with the right version.
  settings_table_.GetInt(kVersionKeyName, &version);
  if (version != kCurrentVersion) {
    assert(false);
    return false;
  }

  return transaction.Commit();
}


bool PermissionsDB::ShouldSupressDialogs() {
  int value = 0;
  settings_table_.GetInt(kSupressDialogsKeyName, &value);
  return value == 1;
}


PermissionsDB::PermissionValue PermissionsDB::GetPermission(
    const SecurityOrigin &origin,
    PermissionType type) {
  int retval_int = PERMISSION_NOT_SET;
  NameValueTable* table = GetTableForPermissionType(type);
  table->GetInt(origin.url().c_str(), &retval_int);
  return static_cast<PermissionsDB::PermissionValue>(retval_int);
}


void PermissionsDB::SetPermission(const SecurityOrigin &origin,
                                  PermissionType type,
                                  PermissionsDB::PermissionValue value) {
  if (origin.url().empty()) {
    assert(false);
    return;
  }

  NameValueTable* table = GetTableForPermissionType(type);
  if (value == PERMISSION_NOT_SET) {
    table->Clear(origin.url().c_str());
  } else if (value == PERMISSION_ALLOWED || value == PERMISSION_DENIED) {
    table->SetInt(origin.url().c_str(), value);
  } else {
    LOG(("PermissionsDB::SetPermission invalid value: %d", value));
    assert(false);
  }

  if ((type == PERMISSION_LOCAL_DATA) &&
      (value == PERMISSION_DENIED || value == PERMISSION_NOT_SET)) {
    // Remove Database content.
    database_name_table_.DeleteDatabasesForOrigin(origin);

    // Remove LocalServer content.
    WebCacheDB *webcacheDB = WebCacheDB::GetDB();
    if (webcacheDB) {
      webcacheDB->DeleteServersForOrigin(origin);
    }
  }
}


bool PermissionsDB::GetOriginsByValue(PermissionsDB::PermissionValue value,
                                      PermissionType type,
                                      std::vector<SecurityOrigin> *result) {
  if (PERMISSION_ALLOWED != value && PERMISSION_DENIED != value) {
    LOG(("Unexpected value: %d", value));
    return false;
  }
  
  NameValueTable* table = GetTableForPermissionType(type);
  std::vector<std::string16> origins;
  if (!table->FindNamesByIntValue(value, &origins)) {
    return false;
  }

  for (int i = 0; i < static_cast<int>(origins.size()); ++i) {
    SecurityOrigin origin;
    if (!origin.InitFromUrl(origins.at(i).c_str())) {
      LOG(("PermissionsDB::ListGearsAccess: InitFromUrl() failed."));
      // If we can't initialize a single URL, don't freak out. Try to do the
      // other ones.
      continue;
    }
    result->push_back(origin);
  }

  return true;
}

bool PermissionsDB::GetPermissionsSorted(PermissionsList *permission_list_out) {
  assert(permission_list_out);
  // This query first does an outer join on attribute 'Name' between the
  // LOCAL_DATA and LOCATION_DATA access tables. This yields all origins
  // that have either a LOCAL_DATA permission or both LOCAL_DATA and
  // LOCATION_DATA permissions set. We compose this query with a second query
  // that selects all origins that have only the LOCATION_DATA set. Note that
  // RIGHT or FULL OUTER JOINs are not implemented by SQLite.
  // Finally, we sort the result based on the following rule:
  // - all origins that are allowed any permission type are listed first,
  //   ordered lexicographically.
  // - all origins that are denied both permission types, or that are denied
  //   one permission type while the other one is not set, are listed last,
  //   ordered lexicographically.
  // To make the distinction between the two groups above, we look at the
  // permission values: 1 denotes 'allowed' and 2 denotes 'denied'. For the 
  // sorting purposes, 2 also denotes 'not set'. It then follows that if the
  // summed permission value for an origin is less than 4 (i.e. (1,2) or (1,1)),
  // then the origin must belong to the first group. If the summed value is
  // equal to 4 (i.e. (2,2)) then the origin must belong to the second group.
  static const char16 *select_query =
      STRING16(L"SELECT Name, Storage, Location "
          L"         FROM (SELECT Access.Name AS Name, "
          L"                   LocationAccess.Value AS Location, "
          L"                   Access.Value AS Storage FROM " 
          L"                   Access LEFT OUTER JOIN LocationAccess ON "
          L"                   Access.Name = LocationAccess.Name "
          L"               UNION "
          L"               SELECT DISTINCT LocationAccess.Name, "
          L"                   LocationAccess.Value AS Location, "
          L"                   null AS Storage "
          L"                   FROM LocationAccess "
          L"                   WHERE LocationAccess.Name NOT IN "
          L"                       (SELECT Access.Name FROM Access)) "
          L"         ORDER BY "
          L"         ((coalesce(Location, 2) + coalesce(Storage, 2)) < 4) "
          L"         DESC, Name");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(&db_, select_query)) {
    return false;
  }

  int rv;
  while (SQLITE_DONE != (rv = statement.step())) {
    if (SQLITE_ROW != rv) {
      LOG(("PermissionsDB::GetPermissionsByOrigin: Iterate failed: %d",
           db_.GetErrorCode()));
      return false;
    }
    const char16* origin = statement.column_text16(0);
    assert(origin);
    OriginPermissions origin_permissions;
    PermissionValue value =
        static_cast<PermissionValue>(statement.column_int(1));
    // If the Storage column is null, column_int returns 0.
    if (value != PERMISSION_NOT_SET) {
      origin_permissions[PERMISSION_LOCAL_DATA] = value;
    }

    value = static_cast<PermissionValue>(statement.column_int(2));
    // If the Location column is null, column_int returns 0.
    if (value != PERMISSION_NOT_SET) {
      origin_permissions[PERMISSION_LOCATION_DATA] = value;
    }
    assert(origin_permissions.size() > 0);
    permission_list_out->push_back(std::make_pair(origin, origin_permissions));
  }

  return true;
}


bool PermissionsDB::EnableGearsForWorker(const SecurityOrigin &worker_origin,
                                         const SecurityOrigin &host_origin) {
  SQLTransaction transaction(&db_, "PermissionsDB::EnableGearsForWorker");
  if (!transaction.Begin()) {
    return false;
  }

  if (IsOriginAllowed(host_origin, PERMISSION_LOCAL_DATA)) {
    if (!TryAllow(worker_origin, PERMISSION_LOCAL_DATA)) {
      return false;
    }
  }

  if (IsOriginAllowed(host_origin, PERMISSION_LOCATION_DATA)) {
    if (!TryAllow(worker_origin, PERMISSION_LOCATION_DATA)) {
      return false;
    }
  }

  return transaction.Commit();
}

bool PermissionsDB::TryAllow(const SecurityOrigin &origin,
                             PermissionType type) {

  NameValueTable* table = GetTableForPermissionType(type);
  switch (GetPermission(origin, type)) {
    case PERMISSION_ALLOWED:
      return true;
    case PERMISSION_DENIED:
      return false;
    case PERMISSION_NOT_SET:
      if (!table->SetInt(origin.url().c_str(), PERMISSION_ALLOWED)) {
        return false;
      }
      return true;
    default:
      LOG(("Unexpected permission value"));
      return false;
  }
}

bool PermissionsDB::SetShortcut(const SecurityOrigin &origin,
                                const char16 *name, const char16 *app_url,
                                const char16 *icon16x16_url,
                                const char16 *icon32x32_url,
                                const char16 *icon48x48_url,
                                const char16 *icon128x128_url,
                                const char16 *msg,
                                const bool allow) {
  bool ok = shortcut_table_.SetShortcut(origin.url().c_str(), name,
                                        app_url, icon16x16_url, icon32x32_url,
                                        icon48x48_url, icon128x128_url, msg,
                                        allow);
  if (ok) {
    MessageService::GetInstance()->NotifyObservers(kShortcutsChangedTopic,
                                                   NULL);
  }
  return ok;
}

bool PermissionsDB::GetOriginsWithShortcuts(
    std::vector<SecurityOrigin> *result) {

  std::vector<std::string16> origin_urls;
  if (!shortcut_table_.GetOriginsWithShortcuts(&origin_urls)) {
    return false;
  }

  for (size_t ii = 0; ii < origin_urls.size(); ++ii) {
    SecurityOrigin origin;
    if (!origin.InitFromUrl(origin_urls[ii].c_str())) {
      LOG(("PermissionsDB::GetOriginsWithShortcuts: InitFromUrl() failed."));
      // If we can't initialize a single URL, don't freak out. Try to do the
      // other ones.
      continue;
    }
    result->push_back(origin);
  }
  return true;
}

bool PermissionsDB::GetOriginShortcuts(const SecurityOrigin &origin,
                                       std::vector<std::string16> *names) {
  return shortcut_table_.GetOriginShortcuts(origin.url().c_str(), names);
}

bool PermissionsDB::GetShortcut(const SecurityOrigin &origin,
                                const char16 *name, std::string16 *app_url,
                                std::string16 *icon16x16_url,
                                std::string16 *icon32x32_url,
                                std::string16 *icon48x48_url,
                                std::string16 *icon128x128_url,
                                std::string16 *msg,
                                bool *allow) {
  return shortcut_table_.GetShortcut(origin.url().c_str(), name, app_url,
                                     icon16x16_url, icon32x32_url,
                                     icon48x48_url, icon128x128_url, msg,
                                     allow);
}

bool PermissionsDB::DeleteShortcut(const SecurityOrigin &origin,
                                   const char16 *name) {
  bool ok = shortcut_table_.DeleteShortcut(origin.url().c_str(), name);
  if (ok) {
    MessageService::GetInstance()->NotifyObservers(kShortcutsChangedTopic,
                                                   NULL);
  }
  return ok;
}

bool PermissionsDB::DeleteShortcuts(const SecurityOrigin &origin) {
  bool ok = shortcut_table_.DeleteShortcuts(origin.url().c_str());
  if (ok) {
    MessageService::GetInstance()->NotifyObservers(kShortcutsChangedTopic,
                                                   NULL);
  }
  return ok;
}

// TODO(shess): Should these just be inline in the .h?
bool PermissionsDB::GetDatabaseBasename(const SecurityOrigin &origin,
                                        const char16 *database_name,
                                        std::string16 *basename) {
  return database_name_table_.GetDatabaseBasename(origin.url().c_str(),
                                                  database_name,
                                                  basename);
}

bool PermissionsDB::MarkDatabaseCorrupt(const SecurityOrigin &origin,
                                        const char16 *database_name,
                                        const char16 *basename) {
  return database_name_table_.MarkDatabaseCorrupt(origin.url().c_str(),
                                                  database_name,
                                                  basename);
}

bool PermissionsDB::CreateDatabase() {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "PermissionsDB::CreateDatabase");
  if (!transaction.Begin()) {
    return false;
  }

  if (!db_.DropAllObjects()) {
    return false;
  }

  if (!settings_table_.MaybeCreateTable() ||
      !local_data_access_table_.MaybeCreateTable() ||
      !location_access_table_.MaybeCreateTable() ||
      !shortcut_table_.MaybeCreateTableLatestVersion() ||
      !database_name_table_.MaybeCreateTableLatestVersion() ||
      !database2_metadata_.MaybeCreateTableLatestVersion()) {
    return false;
  }

  // set the current version
  if (!settings_table_.SetInt(kVersionKeyName, kCurrentVersion)) {
    return false;
  }

  return transaction.Commit();
}

bool PermissionsDB::UpgradeDatabase(int from_version) {
  ASSERT_SINGLE_THREAD();

  assert(from_version < kCurrentVersion);
  assert(db_.IsInTransaction());

  // NOTE: Each of these cases falls through to the next on purpose.
  switch (from_version) {
    case 1:
      if (!UpgradeToVersion2()) return false;
      ++from_version;
    case 2:
      if (!UpgradeToVersion3()) return false;
      ++from_version;
    case 3:
      if (!UpgradeToVersion4()) return false;
      ++from_version;
    case 4:
      if (!UpgradeToVersion5()) return false;
      ++from_version;
    case 5:
      if (!UpgradeToVersion6()) return false;
      ++from_version;
    case 6:
      if (!UpgradeToVersion7()) return false;
      ++from_version;
    case 7:
      if (!UpgradeToVersion8()) return false;
      ++from_version;
    case 8:
      if (!UpgradeToVersion9()) return false;
      ++from_version;
  }

  return settings_table_.SetInt(kVersionKeyName, from_version);
}

bool PermissionsDB::UpgradeToVersion2() {
  // There was a bug in v1 of this db where we inserted some corrupt UTF-8
  // characters into the db. This was pre-release, so it's not worth trying
  // to clean it up. Instead just remove old permissions.
  //
  // TODO(shess) I'm inclined to say "DROP TABLE IF EXISTS
  // ScourAccess".  Or, since this was from a pre-release schema,
  // "upgrade" version 1 by calling CreateDatabase(), which will drop
  // all existing tables.
  return (SQLITE_OK != db_.Execute("DELETE FROM ScourAccess"));
}

bool PermissionsDB::UpgradeToVersion3() {
  return shortcut_table_.UpgradeToVersion3();
}

bool PermissionsDB::UpgradeToVersion4() {
  return shortcut_table_.UpgradeFromVersion3ToVersion4();
}

bool PermissionsDB::UpgradeToVersion5() {
  return shortcut_table_.UpgradeFromVersion4ToVersion5();
}

bool PermissionsDB::UpgradeToVersion6() {
  return shortcut_table_.UpgradeFromVersion5ToVersion6();
}

bool PermissionsDB::UpgradeToVersion7() {
  return database_name_table_.UpgradeToVersion7();
}

bool PermissionsDB::UpgradeToVersion8() {
  return database2_metadata_.CreateTableVersion8();
}

bool PermissionsDB::UpgradeToVersion9() {
  return location_access_table_.MaybeCreateTable();
}

NameValueTable* PermissionsDB::GetTableForPermissionType(PermissionType type) {
  switch (type) {
    case PERMISSION_LOCAL_DATA:
      return &local_data_access_table_;
    case PERMISSION_LOCATION_DATA:
      return &location_access_table_;
    default:
      LOG(("Unexpected permission type"));
      assert(false);
      return NULL;
  }
}
