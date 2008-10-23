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

#include "gears/localserver/common/localserver_db.h"

#include <map>
#include <string>
#include <vector>

#include "gears/base/common/exception_handler.h"  // For ExceptionManager
#include "gears/base/common/file.h"
#include "gears/base/common/http_utils.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/common/url_utils.h"
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"  // For BrowserCache
#endif
#include "gears/inspector/inspector_resources.h"
#include "gears/localserver/common/blob_store.h"
#ifdef USE_FILE_STORE
#include "gears/localserver/common/file_store.h"
#endif
#include "gears/localserver/common/http_cookies.h"
#include "gears/localserver/common/managed_resource_store.h"
#include "gears/localserver/common/update_task.h"

const char16 *WebCacheDB::kFilename = STRING16(L"localserver.db");
const int64 WebCacheDB::kUnknownID = 0;   // SQLITE rowids start at 1

// Name of NameValueTable created to store version and browser information
const char16 *kSystemInfoTableName = STRING16(L"SystemInfo");

// Names of various other tables
const char *kServersTable = "Servers";
const char *kVersionsTable = "Versions";
const char *kEntriesTable = "Entries";
const char *kPayloadsTable = "Payloads";
const char *kResponseBodiesTable = "ResponseBodies";

// Key used to store cache instances in ThreadLocals
const ThreadLocals::Slot kThreadLocalKey = ThreadLocals::Alloc();

// TODO(michaeln): VS2005 for windows has redefined ARRAYSIZE in a
// fashion that is not equivalent with the macro we use elsewhere.
// Specifically, MS's macro doesn't work well with anonymous types.
// Switch common.h to use the simple macro and change usages throughout
// our project.
#define SIMPLEARRAYSIZE(a) \
  ((sizeof(a) / sizeof(*(a))) / \
   static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

static const struct {
    const char *table_name;
    const char *columns;
} kWebCacheTables[] =
    {
      { kServersTable,
        "(ServerID INTEGER PRIMARY KEY AUTOINCREMENT,"
        " Enabled INT CHECK(Enabled IN (0, 1)),"
        " SecurityOriginUrl TEXT NOT NULL,"
        " Name TEXT NOT NULL,"
        " RequiredCookie TEXT,"
        " ServerType INT CHECK(ServerType IN (0, 1)),"
        " ManifestUrl TEXT,"
        " UpdateStatus INT CHECK(UpdateStatus IN (0,1,2,3)),"
        " LastUpdateCheckTime INTEGER DEFAULT 0,"
        " ManifestDateHeader TEXT,"
        " LastErrorMessage TEXT)" },

      { kVersionsTable,
        "(VersionID INTEGER PRIMARY KEY AUTOINCREMENT,"
        " ServerID INTEGER NOT NULL,"
        " VersionString TEXT NOT NULL,"
        " ReadyState INTEGER CHECK(ReadyState IN (0, 1)),"
        " SessionRedirectUrl TEXT)" },

      { kEntriesTable,
        "(EntryID INTEGER PRIMARY KEY AUTOINCREMENT,"
        " VersionID INTEGER,"
        " Url TEXT NOT NULL,"
        " Src TEXT,"       // The manifest file entry's src attribute
        " PayloadID INTEGER,"
        " Redirect TEXT,"
        " IgnoreQuery INTEGER CHECK(IgnoreQuery IN (0, 1)))" },

      { kPayloadsTable,
        "(PayloadID INTEGER PRIMARY KEY AUTOINCREMENT,"
        " CreationDate INTEGER,"
        " Headers TEXT,"
        " StatusCode INTEGER,"
        " StatusLine TEXT)" },

      { kResponseBodiesTable,
        "(BodyID INTEGER PRIMARY KEY,"  // This is the same ID as the payloadID
        " FilePath TEXT,"  // With USE_FILE_STORE, bodies are stored as
        " Data BLOB)" }    // discrete files, otherwise as blobs in the DB
    };

static const int kWebCacheTableCount = SIMPLEARRAYSIZE(kWebCacheTables);

static const struct {
    const char *index_name;
    const char *table_name;
    const char *columns;
    bool unique;
} kWebCacheIndexes[] =
    {
      { "ServersOriginIndex",
        kServersTable,
        "(SecurityOriginUrl,"
        " Name,"
        " RequiredCookie,"
        " ServerType)",
        true },

      { "VersionsServerIndex",
        kVersionsTable,
        "(ServerID)",
        false },

      { "EntriesUrlIndex",
        kEntriesTable,
        "(Url)",
        false },

      { "EntriesVersionIndex",
        kEntriesTable,
        "(VersionID)",
        false },

      { "EntriesPayloadIndex",
        kEntriesTable,
        "(PayloadID)",
        false }
    };

static const int kWebCacheIndexCount = SIMPLEARRAYSIZE(kWebCacheIndexes);

/* Schema version history

  version 1: Initial version
  version 2: Added the Enabled column to the Servers table
  version 3: IE ONLY: Added the FilePath column to the Payloads table,
             switched to storing file contents in the file system
             rather than in SQLite as a BLOB. Having files on disk
             allows us to satisfies URLMON's interfaces that require
             returning a file path w/o having to create temp files.
             Also there are potential (unrealized) performance gains to be
             found around shortening the length of time the DB file is 
             locked, and performing streaming I/O into and out of the 
             cached files.
  version 4: Added LastErrorMessage column to Servers table
  version 5: Added the ResponseBodies table
  version 6: Added StatusCode and StatusLine columns to the Payloads table
             and the Redirect plus IsUserSpecifiedRedirect columns to Entries
  version 7: Removed VersionReadyState.VERSION_PENDING and all related code
  version 8: Renamed 'SessionValue' column to 'RequiredCookie'
  version 9: Added IgnoreQuery and removed IsUserSpecified columns
  version 10: Added SecurityOriginUrl and removed Domain columns
  version 11: No actual database schema change, but if an origin does not have
              PERMISSION_ALLOWED, the DB should not contain anything for that
              origin. The version was bumped to trigger an upgrade script which
              removes existing data that should not be there.
  version 12: Added indexes
*/

// The names of values stored in the system_info table
static const char16 *kSchemaVersionName = STRING16(L"version");
static const char16 *kSchemaBrowserName = STRING16(L"browser");

// The values stored in the system_info table
const int kCurrentVersion = 12;
#if BROWSER_IE
static const char16 *kCurrentBrowser = STRING16(L"ie");
#elif BROWSER_FF
static const char16 *kCurrentBrowser = STRING16(L"firefox");
#elif BROWSER_SAFARI
// Some versions of the Safari port had this incorrectly set
// to "npapi", so be careful relying on this!
static const char16 *kCurrentBrowser = STRING16(L"safari");
#elif BROWSER_NPAPI
static const char16 *kCurrentBrowser = STRING16(L"npapi");
#else
#error "BROWSER_?? not defined."
#endif


//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
WebCacheDB::WebCacheDB()
    : system_info_table_(&db_, kSystemInfoTableName),
      response_bodies_store_(NULL) {
  // When parameter binding multiple parameters, we frequently use a scheme
  // of OR'ing return values together for testing for an error once after
  // all rv |= bind_foo() assignments have been made. This relies on
  // SQLITE_OK being 0.
  assert(SQLITE_OK == 0);
}


//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
bool WebCacheDB::Init() {
  // Initialize the database
  if (!db_.Open(kFilename)) {
    return false;
  }

  // Initialize the storage for bodies
#ifdef USE_FILE_STORE
  response_bodies_store_ = new WebCacheFileStore;
  db_.SetTransactionListener(this);
#else
  response_bodies_store_ = new WebCacheBlobStore;
#endif
  if (!response_bodies_store_ || !response_bodies_store_->Init(this)) {
    return false;
  }

  // Examine the contents of the database and determine if we have to
  // instantiate or updgrade the schema.  Also ensure that the database
  // file is for the browser we're running under.

  int version = 0;
  std::string16 browser;
  system_info_table_.GetInt(kSchemaVersionName, &version);
  system_info_table_.GetString(kSchemaBrowserName, &browser);

  // We used to check that the browser entry in the database matched
  // kCurrentBrowser, we removed the test becaue of a bug where
  // kCurrentBrowser was set to 'npapi' for the Safari port.  There may
  // still be dbs with erroneous values in the wild, and the test
  // doesn't seem too important, so think twice before reimplementing.
  
  // if its the version we're expecting and browser is valid, great
  if ((version == kCurrentVersion) && !browser.empty()) {
    return true;
  }
  
  // We have to either create or upgrade the database
  if (!CreateOrUpgradeDatabase()) {
    return false;
  }

  return true;
}


//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
WebCacheDB::~WebCacheDB() {
  delete response_bodies_store_;
}


//------------------------------------------------------------------------------
// Creates or upgrades the database to kCurrentVersion
//------------------------------------------------------------------------------
bool WebCacheDB::CreateOrUpgradeDatabase() {
  const int kOldestUpgradeableVersion = 10;

  // Doing this in a transaction effectively locks the database file and
  // ensures that this is synchronized across all threads and processes
  SQLTransaction transaction(&db_, "CreateOrUpgradeDatabase");
  if (!transaction.Begin()) {
    return false;
  }

  // Now that we have locked the database, fetch the version
  int version = 0;
  system_info_table_.GetInt(kSchemaVersionName, &version);

  if (version == kCurrentVersion) {
    // some other thread/process has performed the create or upgrade already
    return true;
  } else if (version < 0) {
    // something is seriously wrong, bail
    return false;
  } else if (version == 0) {
    // db is brand new and empty
    if (!CreateDatabase()) {
      return false;
    }
  } else if (version > kCurrentVersion) {
    // db is too new, the user is running old code against a new db
    return false;
  } else if (version < kOldestUpgradeableVersion) {
    // db is too old to migrate, drop the database (!?)
    // TODO(michaeln): we need to think about whether this is really the right
    // thing to do, or if we would rather just have it be an error and let the
    // user decide.
    LOG(("Recreating webcache database\n"));
    if (!CreateDatabase()) {
      return false;
    }
  } else {
    // we can upgrade the db
    switch (version) {
      case 10:
        if (!UpgradeFrom10To11()) {
          LOG(("WebCache: UpgradeFrom10To11 failed\n"));
          db_.Close();
          return false;
        }
        // fallthru...
      case 11:
        if (!UpgradeFrom11To12()) {
          LOG(("WebCache: UpgradeFrom11To12 failed\n"));
          db_.Close();
          return false;
        }
        // fallthru...

      // additional upgrades here...
    }
  }

#ifdef DEBUG
  // Debug only test to ensure that an upgrade script is not missing
  if (!system_info_table_.GetInt(kSchemaVersionName, &version)) {
    return false;
  }
  assert(version == kCurrentVersion);
  if (version != kCurrentVersion) {
    return false;
  }
#endif

  if (!transaction.Commit()) {
    return false;
  }

  return true;
}


//------------------------------------------------------------------------------
// CreateDatabase
//------------------------------------------------------------------------------
bool WebCacheDB::CreateDatabase() {
  ASSERT_SINGLE_THREAD();
  assert(db_.IsOpen());

  SQLTransaction transaction(&db_, "CreateDatabase");
  if (!transaction.Begin())
    return false;

#ifdef USE_FILE_STORE
  // TODO(michaeln): clear out pre-existing cached files
#endif

  db_.DropAllObjects();
  if (!CreateTables())
    return false;

  SQLStatement stmt;
  const char16 *sql = STRING16(L"INSERT INTO SystemInfo"
                               L" (Name, Value)"
                               L" VALUES(?, ?)");
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK)
    return false;

  // insert schema version

  rv = SQLITE_OK;
  rv |= stmt.bind_text16(0, kSchemaVersionName);
  rv |= stmt.bind_int(1, kCurrentVersion);
  if (rv != SQLITE_OK)
    return false;

  if (stmt.step() != SQLITE_DONE)
    return false;

  // insert browser identifier

  if (stmt.reset() != SQLITE_OK)
    return false;

  rv = SQLITE_OK;
  rv |= stmt.bind_text16(0, kSchemaBrowserName);
  rv |= stmt.bind_text16(1, kCurrentBrowser);
  if (rv != SQLITE_OK)
    return false;

  if (stmt.step() != SQLITE_DONE)
    return false;

  return transaction.Commit();
}


//------------------------------------------------------------------------------
// CreateTables
//------------------------------------------------------------------------------
bool WebCacheDB::CreateTables() {
  ASSERT_SINGLE_THREAD();
  // TODO(michaeln):  assert(db_.IsInTransaction());

  if (!system_info_table_.MaybeCreateTable()) {
    return false;
  }

  for (int i = 0; i < kWebCacheTableCount; ++i) {
    std::string sql("CREATE TABLE ");
    sql += kWebCacheTables[i].table_name;
    sql += kWebCacheTables[i].columns;
    if (db_.Execute(sql.c_str()) != SQLITE_OK) {
      return false;
    }
  }

  return CreateIndexes();
}


bool WebCacheDB::CreateIndexes() {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "CreateIndexes");
  if (!transaction.Begin()) {
    return false;
  }

  for (int i = 0; i < kWebCacheIndexCount; ++i) {
    std::string sql;
    if (kWebCacheIndexes[i].unique)
      sql += "CREATE UNIQUE INDEX ";
    else
      sql += "CREATE INDEX ";
    sql += kWebCacheIndexes[i].index_name;
    sql += " ON ";
    sql += kWebCacheIndexes[i].table_name;
    sql += kWebCacheIndexes[i].columns;
    if (db_.Execute(sql.c_str()) != SQLITE_OK) {
      return false;
    }
  }

  return transaction.Commit();
}

#ifdef USING_CCTESTS
bool WebCacheDB::DropIndexes() {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "DropIndexes");
  if (!transaction.Begin()) {
    return false;
  }

  for (int i = 0; i < kWebCacheIndexCount; ++i) {
    std::string sql;
    sql += "DROP INDEX IF EXISTS ";
    sql += kWebCacheIndexes[i].index_name;
    if (db_.Execute(sql.c_str()) != SQLITE_OK) {
      return false;
    }
  }

  return transaction.Commit();
}
#endif

/*
Sample upgrade function
//------------------------------------------------------------------------------
// UpgradeFromXToY
//------------------------------------------------------------------------------
bool WebCacheDB::UpgradeFromXToY() {
  const char *kUpgradeCommands[] = {
      // schema upgrade statements go here
      "UPDATE SystemInfo SET value=Y WHERE name=\"version\""
  };
  const int kUpgradeCommandsCount = ARRAYSIZE(kUpgradeCommands);

  return ExecuteSqlCommandsInTransaction(kUpgradeCommands,
                                         kUpgradeCommandsCount);
}
*/

//------------------------------------------------------------------------------
// UpgradeFrom10To11
//------------------------------------------------------------------------------
bool WebCacheDB::UpgradeFrom10To11() {
  SQLTransaction transaction(&db_, "UpgradeFrom10To11");
  if (!transaction.Begin()) {
    return false;
  }

  // NOTE: This upgrade depends on aspects of the current WebCacheDB impl
  // being compatible with the version 10/11 schema. Specifically, the
  // current implementation of DeleteServersForOrigin() is assumed to
  // be compatible with the 10/11 schema. If future versions change the
  // implementation of that method in an incompatible fashion, this upgrade
  // script must be revised.

  // Build an array of SecurityOrigin that have Servers in the DB

  std::vector<SecurityOrigin> origins;
  const char16* sql =
            STRING16(L"SELECT DISTINCT SecurityOriginUrl FROM Servers");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.UpgradeFrom10To11 failed to prepare statement\n"));
    return false;
  }

  while (stmt.step() == SQLITE_ROW) {
    origins.push_back(SecurityOrigin());
    if (!origins.back().InitFromUrl(stmt.column_text16_safe(0))) {
      LOG(("WebCacheDB.UpgradeFrom10To11 failed to origin.InitFromUrl\n"));
      return false;
    }
  }
  stmt.finalize();

  // Check perms on each and if not allowed delete data for that origin

  PermissionsDB *permissions = PermissionsDB::GetDB();
  if (!permissions) {
    return false;
  }

  for (std::vector<SecurityOrigin>::const_iterator iter = origins.begin();
       iter != origins.end(); ++iter) {
    if (!permissions->IsOriginAllowed(*iter,
                                      PermissionsDB::PERMISSION_LOCAL_DATA)) {
      if (!DeleteServersForOrigin(*iter)) {
        return false;
      }
    }
  }

  // Bump the schema version

  const char *kUpgradeCommands[] = {
      "UPDATE SystemInfo SET value=11 WHERE name=\"version\""
  };
  const int kUpgradeCommandsCount = ARRAYSIZE(kUpgradeCommands);
  if (!ExecuteSqlCommands(kUpgradeCommands, kUpgradeCommandsCount))
    return false;

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// UpgradeFrom11To12
//------------------------------------------------------------------------------
bool WebCacheDB::UpgradeFrom11To12() {
  const char *kUpgradeCommands[] = {
      "CREATE UNIQUE INDEX ServersOriginIndex ON "
          "Servers (SecurityOriginUrl,"
                   "Name,"
                   "RequiredCookie,"
                   "ServerType)",
      "CREATE INDEX VersionsServerIndex ON "
          "Versions (ServerID)",
      "CREATE INDEX EntriesUrlIndex ON "
          "Entries (Url)",
      "CREATE INDEX EntriesVersionIndex ON "
          "Entries (VersionID)",
      "CREATE INDEX EntriesPayloadIndex ON "
          "Entries (PayloadID)",
      "UPDATE SystemInfo SET value=12 WHERE name='version'"
  };
  const int kUpgradeCommandsCount = ARRAYSIZE(kUpgradeCommands);
  return ExecuteSqlCommands(kUpgradeCommands, kUpgradeCommandsCount);
}

//------------------------------------------------------------------------------
// ExecuteSqlCommandsInTransaction
//------------------------------------------------------------------------------
bool WebCacheDB::ExecuteSqlCommandsInTransaction(const char *commands[],
                                                 int count) {
  SQLTransaction transaction(&db_, "ExecuteSqlCommandsInTransaction");
  if (!transaction.Begin()) {
    return false;
  }
  if (!ExecuteSqlCommands(commands, count)) {
    return false;
  }
  return transaction.Commit();
}

//------------------------------------------------------------------------------
// ExecuteSqlCommands
//------------------------------------------------------------------------------
bool WebCacheDB::ExecuteSqlCommands(const char *commands[], int count) {
  for (int i = 0; i < count; ++i) {
    int rv = db_.Execute(commands[i]);
    if (rv != SQLITE_OK) {
      return false;
    }
  }
  return true;
}


//------------------------------------------------------------------------------
// CanService
//------------------------------------------------------------------------------
bool WebCacheDB::CanService(const char16 *url, BrowsingContext *context) {
  ASSERT_SINGLE_THREAD();

  return ServiceImpl(url, context, NULL, false);
}


//------------------------------------------------------------------------------
// Service
//------------------------------------------------------------------------------
bool WebCacheDB::Service(const char16 *url, BrowsingContext *context,
                         bool head_only, PayloadInfo *payload) {
  ASSERT_SINGLE_THREAD();
  assert(url);
  assert(payload);

  return ServiceImpl(url, context, payload, head_only);
}


//------------------------------------------------------------------------------
// ServiceImpl
//------------------------------------------------------------------------------
bool WebCacheDB::ServiceImpl(const char16 *url,
                             BrowsingContext *context,
                             PayloadInfo *payload,
                             bool payload_head_only) {
  ASSERT_SINGLE_THREAD();
  assert(url);
  // 'payload' can be NULL if the caller doesn't want that information.

  // If the origin is not explicitly allowed, don't serve anything
  SecurityOrigin origin;
  PermissionsDB *permissions = PermissionsDB::GetDB();
  if (!permissions ||
      !origin.InitFromUrl(url) ||
      !permissions->IsOriginAllowed(origin,
                                    PermissionsDB::PERMISSION_LOCAL_DATA)) {
    return false;
  }

  // Hook for intercepting and serving Inspector content.
  if (ServiceInspectorUrl(url, origin, payload)) {
    return true;
  }

  // If a fragment identifier is appended to the url, ignore it. The fragment
  // identifier is not part of the url and specifies a position within the
  // resource, rather than the resource itself. So we remove the fragment
  // identifier for the purpose of searching the database. The fragment
  // identifier is separated from the URL by '#' and may contain reserved
  // characters including '?'.
  size_t url_length = std::char_traits<char16>::length(url);
  const char16 *fragment = std::char_traits<char16>::find(url, url_length, '#');
  std::string16 url_without_fragment;
  if (fragment) {
    url_without_fragment.assign(url, fragment);
    url = url_without_fragment.c_str();
    url_length = url_without_fragment.length();
  }

  // State shared across our two queries
  bool loaded_cookie_map = false;  // we defer reading cookies until needed
  bool loaded_cookie_map_ok = false;
  CookieMap cookie_map;
  std::string16 possible_session_redirect;

  // First we run a query that looks for exact matches against entries not
  // intended to ignore query arguments.
  int64 payload_id = kUnknownID;
  if (DoServiceQuery(url, true, context, url,
                     &loaded_cookie_map, &loaded_cookie_map_ok, &cookie_map,
                     &possible_session_redirect, &payload_id)) {
    if (payload &&
        payload_id != kUnknownID &&
        !FindPayload(payload_id, payload, payload_head_only)) {
      return false;
    }
    return true;
  }

  // If the requested url contains query parameters, we have to do additional
  // work to respect the 'ignoreQuery' attribute which allows an entry to
  // be hit for a url plus arbitrary query parameters. We do an additional
  // search with the query parameters removed from the requested url.
  const char16 *query = std::char_traits<char16>::find(url, url_length, '?');
  if (query) {
    std::string16 url_without_query(url, query);
    payload_id = kUnknownID;
    if (DoServiceQuery(url_without_query.c_str(), false, context, url,
                       &loaded_cookie_map, &loaded_cookie_map_ok, &cookie_map,
                       &possible_session_redirect, &payload_id)) {
      if (payload &&
          payload_id != kUnknownID &&
          !FindPayload(payload_id, payload, payload_head_only)) {
        return false;
      }
      return true;
    }
  }

  // If we found an entry that requires a cookie, and no cookie exists, and
  // specifies a redirect url for use in that case, then return the redirect.
  if (!possible_session_redirect.empty() && possible_session_redirect != url) {
    if (payload) {
      return payload->SynthesizeHttpRedirect(NULL,
                                             possible_session_redirect.c_str());
    }
    return true;
  }

  return false;
}

bool WebCacheDB::DoServiceQuery(
                     const char16 *url,
                     bool exact_match,
                     BrowsingContext *context,
                     const char16 *requested_url,
                     bool *loaded_cookie_map,
                     bool *loaded_cookie_map_ok,
                     CookieMap *cookie_map,
                     std::string16 *possible_session_redirect,
                     int64 *payload_id_out) {
  // Select possible matching entries for this url from all current versions.
  const char16 *sql = STRING16(
      L"SELECT s.ServerID, s.RequiredCookie, s.ServerType, "
      L"       v.SessionRedirectUrl, e.IgnoreQuery, e.PayloadID, e.Redirect "
      L"FROM Entries e, Versions v, Servers s "
      L"WHERE e.Url = ? AND IFNULL(?, e.IgnoreQuery) AND "
      L"      v.VersionID = e.VersionID AND "
      L"      v.ReadyState = ? AND "
      L"      s.ServerID = v.ServerID AND "
      L"      s.Enabled = 1 ");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.Service failed\n"));
    return false;
  }
  int param = 0;
  rv |= stmt.bind_text16(param++, url);
  if (exact_match) {
    // select entries with any IgnoreQuery value
    rv |= stmt.bind_int(param++, 1);  
  } else {
    // select entries with IgnoreQuery = 1
    rv |= stmt.bind_null(param++);  
  }
  rv |= stmt.bind_int(param++, VERSION_CURRENT);
  if (rv != SQLITE_OK) {
    return false;
  }

  // We can select mulitple candidates for a url. We give preference to entries
  // from stores that are guarded by a required cookie, and within a store, we
  // give preference to entries that don't have the ignore query flag set.
  // The most specific entry is returned.
  const int kBaseRank = 1;
  const int kRequiredCookieBoost = 10;
  const int kNoIgnoreQueryBoost = 1;
  int max_possible_rank = kBaseRank + kRequiredCookieBoost;
  if (exact_match) max_possible_rank += kNoIgnoreQueryBoost;

  int hit_rank = 0;
  int64 hit_server_id = kUnknownID;

  while ((hit_rank < max_possible_rank) && (stmt.step() == SQLITE_ROW)) {
    const int64 server_id = stmt.column_int64(0);
    const std::string16 required_cookie(stmt.column_text16_safe(1));
    const ServerType server_type = static_cast<ServerType>(stmt.column_int(2));
    const char16 *session_redirect = stmt.column_text16_safe(3);
    const bool ignore_query = (stmt.column_int(4) != 0);
    const int64 payload_id = stmt.column_int64(5);
    const char16 *entry_redirect = stmt.column_text16_safe(6);
    bool is_cookie_required = !required_cookie.empty();

    // Compute this entry's rank up front and compare to the rank of the
    // best candidate thus far so we can avoid executing the bulk of the
    // loop's body early on.
    int rank = kBaseRank;
    if (is_cookie_required) rank += kRequiredCookieBoost;
    if (exact_match && !ignore_query) rank += kNoIgnoreQueryBoost;
    if (rank <= hit_rank) { 
      continue;
    }

    if (is_cookie_required) {
      if (!(*loaded_cookie_map)) {
        *loaded_cookie_map = true;
        *loaded_cookie_map_ok = cookie_map->LoadMapForUrl(requested_url,
                                                          context);
        if (!(*loaded_cookie_map_ok)) {
          LOG(("WebCacheDB.Service failed to read cookies\n"));
        }
      }

      if (!(*loaded_cookie_map_ok)) {
        continue;
      }

      std::string16 cookie_name;
      std::string16 cookie_value;
      ParseCookieNameAndValue(required_cookie, &cookie_name, &cookie_value);
      bool has_required_cookie = cookie_map->HasLocalServerRequiredCookie(
                                                 required_cookie);
      if (!has_required_cookie && session_redirect[0] &&
          (cookie_value != kNegatedRequiredCookieValue) &&
          !cookie_map->HasCookie(cookie_name)) {
        if (!possible_session_redirect->empty() &&
            ((*possible_session_redirect) != session_redirect)) {
          LOG(("WebCacheDB.Service conflicting possible session redirects\n"));
        }
        *possible_session_redirect = session_redirect;
      }

      if (!has_required_cookie) {
        continue;
      }
    }

    if (entry_redirect[0]) {
      std::string16 entry_redirect_str(entry_redirect);
      if (entry_redirect_str == requested_url) {
        // Do not return redirects back to the requested url.
        // This can occur when an entry intended to ignore
        // query args and redirect to the url w/o args matches
        // the requested url exactly.
        continue;
      }
    }

    *payload_id_out = payload_id;
    hit_rank = rank;
    hit_server_id = (server_type == MANAGED_RESOURCE_STORE) ? server_id 
                                                            : kUnknownID;
  }

  if (hit_server_id != kUnknownID) {
    // We found a match from a managed store, try to update it.
    // Note that a failure does not prevent servicing the request.
    MaybeInitiateUpdateTask(hit_server_id, context);
  }
  return hit_rank > 0;
}

//------------------------------------------------------------------------------
// ServiceInspectorUrl
//------------------------------------------------------------------------------
bool WebCacheDB::ServiceInspectorUrl(const char16 *url_char16,
                                     const SecurityOrigin &origin,
                                     PayloadInfo *payload) {
#ifdef OFFICIAL_BUILD
  // Inspector is not yet enabled in official builds.
  return false;
#else
  // // TODO(aa): There should be a global setting to enable/disable the inspector.
  // 
  // // TODO(aa): Use gears:// scheme instead of /-gears-/ path?  Other options?
  // 
  // // TODO(aa): Consider adding a helper to get a URL's components.
  // std::string16 url(url_char16);
  // std::string16::size_type path_position = origin.scheme().length() + 3;
  // path_position = url.find(STRING16(L"/"), path_position);
  // if (path_position == std::string16::npos) { return false; }
  // std::string16 path = url.substr(path_position);
  // 
  // const unsigned char *data_pointer;
  // size_t size;
  // std::string16 headers;
  // if (!GetInspectorResource(path, &data_pointer, &size, &headers)) {
  //   return false;
  // }
  // 
  // // We can serve this url but are not interested in content just yet.
  // if (!payload) return true;
  // 
  // // Setup fake HTTP response.
  // payload->id = kUnknownID;
  // payload->creation_date = 0;
  // payload->headers = headers;
  // payload->status_line = STRING16(L"HTTP/1.0 200 OK");
  // payload->status_code = HttpConstants::HTTP_OK;
  // payload->is_synthesized_http_redirect = false;
  // 
  // payload->data.reset(new std::vector<uint8>);
  // payload->data->resize(size);
  // memcpy(&((*payload->data)[0]), data_pointer, size);
  // return true;
return false;

#endif  // OFFICIAL_BUILD ... else ...
}

//------------------------------------------------------------------------------
// MaybeInitiateUpdateTask
//------------------------------------------------------------------------------
void WebCacheDB::MaybeInitiateUpdateTask(int64 server_id,
                                         BrowsingContext *context) {
  scoped_ptr<UpdateTask> task(UpdateTask::CreateUpdateTask(context));
  if (!task.get()->MaybeAutoUpdate(server_id)) return;
  task.release()->DeleteWhenDone();
}

//------------------------------------------------------------------------------
// InsertPayload
//------------------------------------------------------------------------------
bool WebCacheDB::InsertPayload(int64 server_id,
                               const char16 *url,
                               PayloadInfo *payload) {
  ASSERT_SINGLE_THREAD();
  assert(payload);
  assert(url);
  assert((payload->status_code == HttpConstants::HTTP_OK) ||
         payload->IsHttpRedirect());

  if (payload->IsHttpRedirect()) {
    if (!payload->is_synthesized_http_redirect) {
      if (!payload->CanonicalizeHttpRedirect(url)) {
        return false;
      }
    }
  } else if (payload->status_code != HttpConstants::HTTP_OK) {
    return false;
  }

  std::string16 adjusted_headers;
  if (!payload->PassesValidationTests(&adjusted_headers)) {
    assert(false);
    return false;
  }

  SQLTransaction transaction(&db_, "InsertPayload");
  if (!transaction.Begin()) {
    return false;
  }

  payload->creation_date = GetCurrentTimeMillis();

  // First, insert a row into the Payloads table to get a payload_id (rowid),
  // this same id is used as the primary key into the ResponseBodies table

  const char16* sql = STRING16(
      L"INSERT INTO Payloads (CreationDate, Headers, StatusLine, StatusCode) "
      L"VALUES (?, ?, ?, ?)");
  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.InsertPayload failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int64(++param, payload->creation_date);
  rv |= stmt.bind_text16(++param, adjusted_headers.c_str());
  rv |= stmt.bind_text16(++param, payload->status_line.c_str());
  rv |= stmt.bind_int(++param, payload->status_code);
  if (rv != SQLITE_OK) {
    return false;
  }
  if (stmt.step() != SQLITE_DONE) {
    return false;
  }

  payload->id = stmt.last_insert_rowid();

  // Save the body in the bodies store. The full path of the file
  // created is returned in payload->cached_filepath
  if (!response_bodies_store_->InsertBody(server_id, url, payload)) {
    return false;
  }

  return transaction.Commit();
}


//------------------------------------------------------------------------------
// FindPayload
//------------------------------------------------------------------------------
bool WebCacheDB::FindPayload(int64 id,
                             PayloadInfo *payload,
                             bool info_only) {
  ASSERT_SINGLE_THREAD();
  assert(payload);

  const char16* sql = STRING16(L"SELECT ?, CreationDate, Headers, "
                               L"       StatusLine, StatusCode "
                               L"FROM Payloads "
                               L"WHERE PayloadID=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.GetPayload failed\n"));
    return false;
  }
  rv |= stmt.bind_int64(0, id);
  rv |= stmt.bind_int64(1, id);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  return ReadPayloadInfo(stmt, payload, info_only);
}


//------------------------------------------------------------------------------
// ReadPayloadInfo
//------------------------------------------------------------------------------
bool WebCacheDB::ReadPayloadInfo(SQLStatement &stmt,
                                 PayloadInfo *payload,
                                 bool info_only) {
  int col = -1;
  payload->id = stmt.column_int64(++col);
  payload->creation_date = stmt.column_int64(++col);
  payload->headers = stmt.column_text16_safe(++col);
  payload->status_line = stmt.column_text16_safe(++col);
  payload->status_code = stmt.column_int(++col);
  payload->is_synthesized_http_redirect = payload->IsHttpRedirect();
  return response_bodies_store_->ReadBody(payload, info_only);
}

//------------------------------------------------------------------------------
// FindServer
//------------------------------------------------------------------------------
bool WebCacheDB::FindServer(int64 server_id, ServerInfo *server) {
  ASSERT_SINGLE_THREAD();
  assert(server);

  const char16* sql = STRING16(L"SELECT * "
                               L"FROM Servers "
                               L"WHERE ServerID=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindOneServer failed\n"));
    return false;
  }
  rv = stmt.bind_int64(0, server_id);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  ReadServerInfo(stmt, server);
  return true;
}


//------------------------------------------------------------------------------
// FindServer
//------------------------------------------------------------------------------
bool WebCacheDB::FindServer(const SecurityOrigin &security_origin,
                            const char16 *name,
                            const char16 *required_cookie,
                            ServerType server_type,
                            ServerInfo *server) {
  ASSERT_SINGLE_THREAD();
  assert(name);
  assert(server);

  const char16* sql = STRING16(L"SELECT * "
                               L"FROM Servers "
                               L"WHERE SecurityOriginUrl=? AND Name=? AND"
                               L"      RequiredCookie=? AND ServerType=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindOneServer failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_text16(++param, security_origin.url().c_str());
  rv |= stmt.bind_text16(++param, name);
  rv |= stmt.bind_text16(++param, required_cookie);
  rv |= stmt.bind_int(++param, server_type);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  ReadServerInfo(stmt, server);
  return true;
}


//------------------------------------------------------------------------------
// FindServersForOrigin
//------------------------------------------------------------------------------
bool WebCacheDB::FindServersForOrigin(const SecurityOrigin &origin,
                                      std::vector<ServerInfo> *servers) {
  ASSERT_SINGLE_THREAD();
  assert(servers);

  const char16* sql = STRING16(L"SELECT * "
                               L"FROM Servers "
                               L"WHERE SecurityOriginUrl=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindServersForOrigin failed\n"));
    return false;
  }
  rv = stmt.bind_text16(0, origin.url().c_str());
  if (rv != SQLITE_OK) {
    return false;
  }

  while (stmt.step() == SQLITE_ROW) {
    servers->push_back(ServerInfo());
    ReadServerInfo(stmt, &servers->back());
  }

  return true;
}


//------------------------------------------------------------------------------
// DeleteServersForOrigin
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteServersForOrigin(const SecurityOrigin &origin) {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "DeleteServersForOrigin");
  if (!transaction.Begin()) {
    return false;
  }

  std::vector<ServerInfo> servers;
  if (!FindServersForOrigin(origin, &servers)) {
    return false;
  }

  for (std::vector<ServerInfo>::const_iterator iter = servers.begin();
       iter != servers.end(); ++iter) {
    if (!DeleteServer(iter->id)) {
      return false;
    }
  }

  return transaction.Commit();
}


//------------------------------------------------------------------------------
// InsertServer
//------------------------------------------------------------------------------
bool WebCacheDB::InsertServer(ServerInfo *server) {
  ASSERT_SINGLE_THREAD();
  assert(server);

  if (!IsStringValidPathComponent(server->name.c_str())) {
    return false;  // invalid user-defined name
  }

  SQLTransaction transaction(&db_, "InsertServer");
  if (!transaction.Begin()) {
    return false;
  }

  SecurityOrigin origin;
  PermissionsDB *permissions = PermissionsDB::GetDB();
  if (!permissions ||
      !origin.InitFromUrl(server->security_origin_url.c_str()) ||
      !permissions->IsOriginAllowed(origin,
                                    PermissionsDB::PERMISSION_LOCAL_DATA)) {
    return false;
  }

  const char16* sql = STRING16(
                          L"INSERT INTO Servers"
                          L" (Enabled, SecurityOriginUrl, Name, RequiredCookie,"
                          L"  ServerType, ManifestUrl, UpdateStatus,"
                          L"  LastErrorMessage, LastUpdateCheckTime, "
                          L"  ManifestDateHeader)"
                          L" VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.InsertServer failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int(++param, server->enabled ? 1 : 0);
  rv |= stmt.bind_text16(++param, server->security_origin_url.c_str());
  rv |= stmt.bind_text16(++param, server->name.c_str());
  rv |= stmt.bind_text16(++param, server->required_cookie.c_str());
  rv |= stmt.bind_int(++param, server->server_type);
  rv |= stmt.bind_text16(++param, server->manifest_url.c_str());
  rv |= stmt.bind_int(++param, server->update_status);
  rv |= stmt.bind_text16(++param, server->last_error_message.c_str());
  rv |= stmt.bind_int64(++param, server->last_update_check_time);
  rv |= stmt.bind_text16(++param, server->manifest_date_header.c_str());
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_DONE) {
    return false;
  }

  server->id = stmt.last_insert_rowid();

#ifdef USE_FILE_STORE
  if (!response_bodies_store_->CreateDirectoryForServer(server->id)) {
    return false;
  }
#endif

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// UpdateServer
//------------------------------------------------------------------------------
bool WebCacheDB::UpdateServer(int64 id,
                              const char16 *manifest_url) {
  ASSERT_SINGLE_THREAD();

  const char16* sql = STRING16(L"UPDATE Servers "
                               L"SET ManifestUrl=?, "
                               L"    ManifestDateHeader=\"\" "
                               L"WHERE ServerID=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.UpdateServer failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_text16(++param, manifest_url);
  rv |= stmt.bind_int64(++param, id);
  if (rv != SQLITE_OK) {
    return false;
  }

  return (stmt.step() == SQLITE_DONE);
}


//------------------------------------------------------------------------------
// UpdateServer
//------------------------------------------------------------------------------
bool WebCacheDB::UpdateServer(int64 id,
                              UpdateStatus update_status,
                              int64 last_update_check_time,
                              const char16* manifest_date_header,
                              const char16* error_message) {
  ASSERT_SINGLE_THREAD();

  std::string16 sql = STRING16(L"UPDATE Servers "
                               L"SET UpdateStatus=?, "
                               L"    LastUpdateCheckTime=?");
  if (manifest_date_header)
    sql.append(STRING16(L", ManifestDateHeader=?"));
  if (error_message)
    sql.append(STRING16(L", LastErrorMessage=?"));
  sql.append(STRING16(L" WHERE ServerID=?"));

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql.c_str());
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.UpdateServer failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int64(++param, update_status);
  rv |= stmt.bind_int64(++param, last_update_check_time);
  if (manifest_date_header)
    rv |= stmt.bind_text16(++param, manifest_date_header);
  if (error_message)
    rv |= stmt.bind_text16(++param, error_message);
  rv |= stmt.bind_int64(++param, id);
  if (rv != SQLITE_OK) {
    return false;
  }

  return (stmt.step() == SQLITE_DONE);
}


//------------------------------------------------------------------------------
// UpdateServer
//------------------------------------------------------------------------------
bool WebCacheDB::UpdateServer(int64 id, bool enabled) {
  ASSERT_SINGLE_THREAD();

  const char16* sql = STRING16(L"UPDATE Servers "
                               L"SET Enabled=? "
                               L"WHERE ServerID=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.UpdateServer failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int(++param, enabled ? 1 : 0);
  rv |= stmt.bind_int64(++param, id);
  if (rv != SQLITE_OK) {
    return false;
  }

  return (stmt.step() == SQLITE_DONE);
}


//------------------------------------------------------------------------------
// DeleteServer
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteServer(int64 id) {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "DeleteServer");
  if (!transaction.Begin()) {
    return false;
  }

#ifdef USE_FILE_STORE
  response_bodies_store_->DeleteDirectoryForServer(id);
#endif

#ifdef WINCE
  std::vector<EntryInfo> entries;
  VersionInfo version;
  if (FindVersion(id, VERSION_CURRENT, &version)) {
    FindEntries(version.id, &entries);
  }
#endif

  // Delete all versions, entries, no longer referenced payloads
  // related to this server

  if (!DeleteVersions(id)) {
    return false;
  }

  // Now delete the server row

  const char16* sql = STRING16(L"DELETE FROM Servers WHERE ServerID=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.DeleteServer failed\n"));
    return false;
  }
  rv |= stmt.bind_int64(0, id);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_DONE) {
    return false;
  }

  bool committed = transaction.Commit();
#ifdef WINCE
  if (committed) {
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
      BrowserCache::RemoveBogusEntry(entries[i].url.c_str());
    }
  }
#endif
  return committed;
}


//------------------------------------------------------------------------------
// FindVersion
//------------------------------------------------------------------------------
bool WebCacheDB::FindVersion(int64 server_id,
                             VersionReadyState ready_state,
                             VersionInfo *version) {
  ASSERT_SINGLE_THREAD();
  assert(version);

  const char16* sql = STRING16(L"SELECT * "
                               L"FROM Versions "
                               L"WHERE ServerID=? AND ReadyState=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindOneVersion failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int64(++param, server_id);
  rv |= stmt.bind_int(++param, ready_state);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  ReadVersionInfo(stmt, version);
  return true;
}

//------------------------------------------------------------------------------
// FindVersion
//------------------------------------------------------------------------------
bool WebCacheDB::FindVersion(int64 server_id,
                             const char16 *version_string,
                             VersionInfo *version) {
  ASSERT_SINGLE_THREAD();
  assert(version);
  assert(version_string);

  const char16* sql = STRING16(L"SELECT * "
                               L"FROM Versions "
                               L"WHERE ServerID=? AND VersionString=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindOneVersion failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int64(++param, server_id);
  rv |= stmt.bind_text16(++param, version_string);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  ReadVersionInfo(stmt, version);
  return true;
}


//------------------------------------------------------------------------------
// FindVersions
//------------------------------------------------------------------------------
bool WebCacheDB::FindVersions(int64 server_id,
                              std::vector<VersionInfo> *versions) {
  ASSERT_SINGLE_THREAD();
  assert(versions);

  const char16* sql = STRING16(L"SELECT * "
                               L"FROM Versions "
                               L"WHERE ServerID=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindVersions failed\n"));
    return false;
  }
  rv |= stmt.bind_int64(0, server_id);
  if (rv != SQLITE_OK) {
    return false;
  }

  while (stmt.step() == SQLITE_ROW) {
    versions->push_back(VersionInfo());
    ReadVersionInfo(stmt, &versions->back());
  }

  return true;
}

//------------------------------------------------------------------------------
// ReadServerInfo
//------------------------------------------------------------------------------
void WebCacheDB::ReadServerInfo(SQLStatement &stmt, ServerInfo *server) {
  ASSERT_SINGLE_THREAD();
  assert(server);
  int index = -1;
  server->id = stmt.column_int64(++index);
  server->enabled = (stmt.column_int(++index) == 1);
  server->security_origin_url = stmt.column_text16_safe(++index);
  server->name = stmt.column_text16_safe(++index);
  server->required_cookie = stmt.column_text16_safe(++index);
  server->server_type = static_cast<ServerType>(stmt.column_int(++index));
  server->manifest_url = stmt.column_text16_safe(++index);
  server->update_status = static_cast<UpdateStatus>(stmt.column_int(++index));
  server->last_update_check_time = stmt.column_int64(++index);
  server->manifest_date_header = stmt.column_text16_safe(++index);
  server->last_error_message = stmt.column_text16_safe(++index);
}

//------------------------------------------------------------------------------
// ReadVersionInfo
//------------------------------------------------------------------------------
void WebCacheDB::ReadVersionInfo(SQLStatement &stmt, VersionInfo *version) {
  ASSERT_SINGLE_THREAD();
  assert(version);
  version->id = stmt.column_int64(0);
  version->server_id = stmt.column_int64(1);
  version->version_string = stmt.column_text16_safe(2);
  version->ready_state = static_cast<VersionReadyState>(stmt.column_int(3));
  version->session_redirect_url = stmt.column_text16_safe(4);
}


//------------------------------------------------------------------------------
// InsertVersion
//------------------------------------------------------------------------------
bool WebCacheDB::InsertVersion(VersionInfo *version) {
  ASSERT_SINGLE_THREAD();
  assert(version);

  const char16* sql = STRING16(L"INSERT INTO Versions"
                               L" (ServerID, VersionString, ReadyState,"
                               L"  SessionRedirectUrl)"
                               L" VALUES(?, ?, ?, ?)");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.InsertVersion failed\n"));
    return false;
  }

  int param = -1;
  rv |= stmt.bind_int64(++param, version->server_id);
  rv |= stmt.bind_text16(++param, version->version_string.c_str());
  rv |= stmt.bind_int(++param, version->ready_state);
  rv |= stmt.bind_text16(++param, version->session_redirect_url.c_str());
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_DONE) {
    return false;
  }

  version->id = stmt.last_insert_rowid();

  return true;
}

//------------------------------------------------------------------------------
// UpdateVersion
//------------------------------------------------------------------------------
bool WebCacheDB::UpdateVersion(int64 id,
                               VersionReadyState ready_state) {
  ASSERT_SINGLE_THREAD();

  const char16* sql = STRING16(L"UPDATE Versions "
                               L"SET ReadyState=? "
                               L"WHERE VersionID=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.UpdateVersion failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int(++param, ready_state);
  rv |= stmt.bind_int64(++param, id);
  if (rv != SQLITE_OK) {
    return false;
  }

  return (stmt.step() == SQLITE_DONE);
}

//------------------------------------------------------------------------------
// DeleteVersion
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteVersion(int64 id) {
  ASSERT_SINGLE_THREAD();
  std::vector<int64> version_ids;
  version_ids.push_back(id);
  return DeleteVersions(&version_ids);
}

//------------------------------------------------------------------------------
// DeleteVersions
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteVersions(int64 server_id) {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "DeleteVersions");
  if (!transaction.Begin()) {
    return false;
  }

  std::vector<VersionInfo> versions;
  if (FindVersions(server_id, &versions)) {
    std::vector<int64> version_ids;
    for (unsigned int i = 0; i < versions.size(); ++i) {
      version_ids.push_back(versions[i].id);
    }
    DeleteVersions(&version_ids);
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// DeleteVersions
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteVersions(std::vector<int64> *version_ids) {
  ASSERT_SINGLE_THREAD();
  assert(version_ids);
  if (version_ids->size() == 0) {
    return true;
  }

  SQLTransaction transaction(&db_, "DeleteVersions");
  if (!transaction.Begin()) {
    return false;
  }

  // Delete all entries and no longer referenced payloads for these versions

  if (!DeleteEntries(version_ids)) {
    return false;
  }

  // Now delete the version rows

  std::string16 sql = STRING16(L"DELETE FROM Versions WHERE VersionID IN (");
  for (unsigned int i = 0; i < version_ids->size(); ++i) {
    if (i == version_ids->size() - 1)
      sql += STRING16(L"?");
    else
      sql += STRING16(L"?, ");
  }
  sql += STRING16(L")");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql.c_str());
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.DeleteVersion failed\n"));
    return false;
  }
  for (unsigned int i = 0; i < version_ids->size(); ++i) {
    rv |= stmt.bind_int64(i, (*version_ids)[i]);
  }
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_DONE) {
    return false;
  }

  return transaction.Commit();
}


//------------------------------------------------------------------------------
// InsertEntry
//------------------------------------------------------------------------------
bool WebCacheDB::InsertEntry(EntryInfo *entry) {
  ASSERT_SINGLE_THREAD();
  assert(entry);
  assert(!entry->url.empty());
  assert(!entry->ignore_query || (entry->url.find('?') == std::string16::npos));
  assert(entry->url.find('#') == std::string16::npos);

  const char16* sql = STRING16(L"INSERT INTO Entries"
                               L" (VersionID, Url, Src, PayloadID,"
                               L"  Redirect, IgnoreQuery)"
                               L" VALUES (?, ?, ?, ?, ?, ?)");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.InsertEntry failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int64(++param, entry->version_id);
  rv |= stmt.bind_text16(++param, entry->url.c_str());
  if (!entry->src.empty()) {
    rv |= stmt.bind_text16(++param, entry->src.c_str());
  } else {
    rv |= stmt.bind_null(++param);
  }
  if (entry->payload_id != kUnknownID) {
    rv |= stmt.bind_int64(++param, entry->payload_id);
  } else {
    rv |= stmt.bind_null(++param);
  }
  if (!entry->redirect.empty()) {
    rv |= stmt.bind_text16(++param, entry->redirect.c_str());
  } else {
    rv |= stmt.bind_null(++param);
  }
  rv |= stmt.bind_int(++param, entry->ignore_query ? 1 : 0);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_DONE) {
    return false;
  }

  entry->id = stmt.last_insert_rowid();

  return true;
}

//------------------------------------------------------------------------------
// DeleteEntry
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteEntry(int64 id) {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "DeleteEntry");
  if (!transaction.Begin()) {
    return false;
  }

  // Get the payload ID for this entry.
  const char16 *payload_sql = STRING16(L"Select PayloadID FROM Entries "
                                       L"WHERE EntryID=?");
  SQLStatement payload_stmt;
  int rv = payload_stmt.prepare16(&db_, payload_sql);
  rv |= payload_stmt.bind_int64(0, id);
  if (SQLITE_OK != rv) {
    LOG(("WebCacheDB.DeleteEntry failed\n"));
    return false;
  }

  // If there's no match, commit the database transaction and return.
  int step_status = payload_stmt.step();
  if (SQLITE_DONE == step_status ) {
    return transaction.Commit();
  } else if (SQLITE_ROW != step_status ) {
    LOG(("WebCacheDB.DeleteEntry failed\n"));
    return false;
  }
  int64 payload_id = payload_stmt.column_int64(0);

  // Delete the entry.
  const char16* delete_sql = STRING16(L"DELETE FROM Entries "
                                      L"WHERE EntryID=?");
  SQLStatement delete_stmt;
  rv = delete_stmt.prepare16(&db_, delete_sql);
  rv |= delete_stmt.bind_int64(0, id);
  if (SQLITE_OK != rv) {
    LOG(("WebCacheDB.DeleteEntry failed\n"));
    return false;
  }
  if (SQLITE_DONE != delete_stmt.step()) {
    LOG(("WebCacheDB.DeleteEntry failed\n"));
    return false;
  }

  // The payload_id may be NULL if the payload has not yet been inserted.
  if (kUnknownID == payload_id) {
    LOG(("WebCacheDB.DeleteEntry - payload_id is NULL\n"));
    return transaction.Commit();
  }

  // Delete the payload if it is orphaned.
  if (!MaybeDeletePayload(payload_id)) {
    LOG(("WebCacheDB.DeleteEntry failed\n"));
    return false;
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// DeleteEntries
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteEntries(int64 version_id) {
  ASSERT_SINGLE_THREAD();
  std::vector<int64> version_ids;
  version_ids.push_back(version_id);
  return DeleteEntries(&version_ids);
}


//------------------------------------------------------------------------------
// DeleteEntries
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteEntries(std::vector<int64> *version_ids) {
  ASSERT_SINGLE_THREAD();
  assert(version_ids);
  if (version_ids->size() == 0) {
    return true;
  }

  SQLTransaction transaction(&db_, "DeleteEntries");
  if (!transaction.Begin()) {
    return false;
  }

  // Delete all Entries table rows for version_ids

  std::string16 sql(STRING16(L"DELETE FROM Entries WHERE VersionID IN ("));
  for (unsigned int i = 0; i < version_ids->size(); ++i) {
    if (i == version_ids->size() - 1)
      sql += STRING16(L"?");
    else
      sql += STRING16(L"?, ");
  }
  sql += STRING16(L")");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql.c_str());
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.DeleteEntries failed\n"));
    return false;
  }
  for (unsigned int i = 0; i < version_ids->size(); ++i) {
    rv |= stmt.bind_int64(i, (*version_ids)[i]);
  }
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_DONE) {
    return false;
  }

  // Now delete all unreferenced payloads

  if (!DeleteUnreferencedPayloads()) {
    return false;
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// FindEntry
//------------------------------------------------------------------------------
bool WebCacheDB::FindEntry(int64 version_id,
                           const char16 *url,
                           EntryInfo *entry) {
  ASSERT_SINGLE_THREAD();
  assert(url);
  assert(entry);

  const char16 *sql = STRING16(L"SELECT * FROM Entries "
                               L"WHERE VersionID=? AND Url=?");
  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindEntry failed\n"));
    return false;
  }
  rv |= stmt.bind_int64(0, version_id);
  rv |= stmt.bind_text16(1, url);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  ReadEntryInfo(stmt, entry);
  return true;
}

//------------------------------------------------------------------------------
// FindEntries
//------------------------------------------------------------------------------
bool WebCacheDB::FindEntries(int64 version_id,
                             std::vector<EntryInfo> *entries) {
  ASSERT_SINGLE_THREAD();
  std::vector<int64> version_ids;
  version_ids.push_back(version_id);
  return FindEntries(&version_ids, entries);
}

//------------------------------------------------------------------------------
// FindEntries
//------------------------------------------------------------------------------
bool WebCacheDB::FindEntries(std::vector<int64> *version_ids,
                             std::vector<EntryInfo> *entries) {
  ASSERT_SINGLE_THREAD();
  assert(version_ids);
  assert(entries);
  assert(entries->empty());
  if (version_ids->size() == 0) {
    return true;
  }

  std::string16 sql(STRING16(L"SELECT * FROM Entries WHERE VersionId IN ("));
  for (unsigned int i = 0; i < version_ids->size(); ++i) {
    if (i == version_ids->size() - 1)
      sql += STRING16(L"?");
    else
      sql += STRING16(L"?, ");
  }
  sql += STRING16(L")");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql.c_str());
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindEntries failed\n"));
    return false;
  }
  for (unsigned int i = 0; i < version_ids->size(); ++i) {
    rv |= stmt.bind_int64(i, (*version_ids)[i]);
  }
  if (rv != SQLITE_OK) {
    return false;
  }

  while (stmt.step() == SQLITE_ROW) {
    entries->push_back(EntryInfo());
    ReadEntryInfo(stmt, &entries->back());
  }

  return true;
}

//------------------------------------------------------------------------------
// FindEntriesHavingNoResponse
//------------------------------------------------------------------------------
bool WebCacheDB::FindEntriesHavingNoResponse(int64 version_id,
                                             std::vector<EntryInfo> *entries) {
  ASSERT_SINGLE_THREAD();
  assert(entries);
  assert(entries->empty());

  const char16 *sql = STRING16(L"SELECT * FROM Entries "
                               L"WHERE VersionId=? AND PayloadId IS NULL");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindEntriesHavingNoResponse failed\n"));
    return false;
  }
  rv |= stmt.bind_int64(0, version_id);
  if (rv != SQLITE_OK) {
    return false;
  }

  while (stmt.step() == SQLITE_ROW) {
    entries->push_back(EntryInfo());
    ReadEntryInfo(stmt, &entries->back());
  }

  return true;
}


//------------------------------------------------------------------------------
// ReadEntryInfo
//------------------------------------------------------------------------------
void WebCacheDB::ReadEntryInfo(SQLStatement &stmt, EntryInfo *entry) {
  entry->id = stmt.column_int64(0);
  entry->version_id = stmt.column_int64(1);
  entry->url = stmt.column_text16_safe(2);
  entry->src = stmt.column_text16_safe(3);
  entry->payload_id = stmt.column_int64(4);
  entry->redirect = stmt.column_text16_safe(5);
  entry->ignore_query = (stmt.column_int(6) == 1);
}


//------------------------------------------------------------------------------
// DeleteEntry
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteEntry(int64 version_id, const char16 *url) {
  ASSERT_SINGLE_THREAD();
  assert(url);

  SQLTransaction transaction(&db_, "DeleteEntry");
  if (!transaction.Begin()) {
    return false;
  }

  // Get the entry ID for the requested URL and version.
  const char16* sql = STRING16(L"SELECT EntryID FROM Entries "
                               L"WHERE VersionID=? AND Url=?");
  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  rv |= stmt.bind_int64(0, version_id);
  rv |= stmt.bind_text16(1, url);
  if (SQLITE_OK != rv) {
    LOG(("WebCacheDB.DeleteEntry failed\n"));
    return false;
  }

  // There should be at most one match. If there are multiple matches, we still
  // delete the entries, but send a crash report.
  int num_matches = 0;
  int step_result;
  while (true) {
    step_result = stmt.step();
    if (SQLITE_ROW != step_result) {
      break;
    }
    ++num_matches;
    if (!DeleteEntry(stmt.column_int64(0))) {
      LOG(("WebCacheDB.DeleteEntry failed\n"));
      return false;
    }
  }
  if (SQLITE_DONE != step_result) {
    return false;
  }
  if (num_matches > 1) {
    ExceptionManager::ReportAndContinue();
    LOG(("WebCacheDB.DeleteEntry - multiple matches for requested URL\n"));
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// CountEntries
//------------------------------------------------------------------------------
bool WebCacheDB::CountEntries(int64 version_id, int64 *count) {
  ASSERT_SINGLE_THREAD();
  assert(count);

  const char16 *sql = STRING16(L"SELECT COUNT(*) FROM Entries "
                               L"WHERE VersionID=?");
  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.CountEntries failed\n"));
    return false;
  }
  rv |= stmt.bind_int64(0, version_id);
  if (rv != SQLITE_OK) {
    return false;
  }
  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  *count = stmt.column_int64(0);

  return true;
}

//------------------------------------------------------------------------------
// UpdateEntry
//------------------------------------------------------------------------------
bool WebCacheDB::UpdateEntry(int64 version_id,
                             const char16 *orig_url,
                             const char16 *new_url) {
  ASSERT_SINGLE_THREAD();
  assert(orig_url);
  assert(new_url);

  const char16* sql = STRING16(L"UPDATE Entries "
                               L"SET Url=? "
                               L"WHERE VersionID=? AND Url=?");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.UpdateEntry failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_text16(++param, new_url);
  rv |= stmt.bind_int64(++param, version_id);
  rv |= stmt.bind_text16(++param, orig_url);
  if (rv != SQLITE_OK) {
    return false;
  }

  return (stmt.step() == SQLITE_DONE);
}

//------------------------------------------------------------------------------
// UpdateEntriesWithNewPayload
//------------------------------------------------------------------------------
bool WebCacheDB::UpdateEntriesWithNewPayload(int64 version_id,
                                             const char16 *url,
                                             int64 payload_id,
                                             const char16 *redirect_url) {
  ASSERT_SINGLE_THREAD();
  assert(url);
  assert(payload_id != kUnknownID);

  // TODO(michaeln): I think this is linear with regard to the number of
  // entries in a version, could be improved.
  const char16* sql = STRING16(
                        L"UPDATE Entries SET PayloadId=?, Redirect=? "
                        L"WHERE VersionId=? AND PayloadId IS NULL AND "
                        L"      (Src=? OR (Src IS NULL AND Url=?))");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.UpdateEntry failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int64(++param, payload_id);
  if (redirect_url && redirect_url[0]) {
    rv |= stmt.bind_text16(++param, redirect_url);
  } else {
    rv |= stmt.bind_null(++param);
  }
  rv |= stmt.bind_int64(++param, version_id);
  rv |= stmt.bind_text16(++param, url);
  rv |= stmt.bind_text16(++param, url);
  if (rv != SQLITE_OK) {
    return false;
  }

  return (stmt.step() == SQLITE_DONE);
}

//------------------------------------------------------------------------------
// FindMostRecentPayload
//------------------------------------------------------------------------------
bool WebCacheDB::FindMostRecentPayload(int64 server_id,
                                       const char16 *url,
                                       PayloadInfo *payload) {
  ASSERT_SINGLE_THREAD();
  assert(url);
  assert(payload);

  // TODO(michaeln): This looks like a full table scan!
  const char16* sql = STRING16(
      L"SELECT p.PayloadID, p.CreationDate, p.Headers, "
      L"       p.StatusLine, p.StatusCode "
      L"FROM Payloads p, Entries e, Versions v "
      L"WHERE v.ServerID=? AND v.VersionID=e.VersionID AND "
      L"      e.PayloadID=p.PayloadID AND "
      L"      (e.Src=? OR (e.Src IS NULL AND e.Url=?)) "
      L"ORDER BY p.CreationDate DESC LIMIT 1");

  SQLStatement stmt;
  int rv = stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.FindMostRecentPayloadInfo failed\n"));
    return false;
  }
  int param = -1;
  rv |= stmt.bind_int64(++param, server_id);
  rv |= stmt.bind_text16(++param, url);
  rv |= stmt.bind_text16(++param, url);
  if (rv != SQLITE_OK) {
    return false;
  }

  if (stmt.step() != SQLITE_ROW) {
    return false;
  }

  return ReadPayloadInfo(stmt, payload, true);
}


//------------------------------------------------------------------------------
// DeleteUnreferencedPayload
// TODO(michaeln): This is terribly inefficient. In general, deleting stale
// payloads could be done lazily. Even if done lazily, these SQL operations
// are not good. Make this better.
//------------------------------------------------------------------------------
bool WebCacheDB::DeleteUnreferencedPayloads() {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "DeleteUnreferencedPayloads");
  if (!transaction.Begin()) {
    return false;
  }

  // Delete from the Payloads table
  const char16 *sql = STRING16(L"DELETE FROM Payloads WHERE PayloadID NOT IN "
                               L"(SELECT DISTINCT PayloadID FROM Entries)");
  SQLStatement delete_stmt;
  int rv = delete_stmt.prepare16(&db_, sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.DeleteUnreferencedPayloads failed\n"));
    return false;
  }
  if (delete_stmt.step() != SQLITE_DONE) {
    return false;
  }

  // Now delete the response bodies
  if (!response_bodies_store_->DeleteUnreferencedBodies()) {
    return false;
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// MaybeDeletePayload
//------------------------------------------------------------------------------
bool WebCacheDB::MaybeDeletePayload(int64 payload_id) {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "MaybeDeletePayload");
  if (!transaction.Begin()) {
    return false;
  }

  const char16* count_sql = STRING16(L"SELECT COUNT(*) FROM Entries "
                                     L"WHERE PayloadID=?");
  SQLStatement count_stmt;
  int rv = count_stmt.prepare16(&db_, count_sql);
  rv |= count_stmt.bind_int64(0, payload_id);
  if (SQLITE_OK != rv) {
    LOG(("WebCacheDB.MaybeDeletePayload failed\n"));
    return false;
  }
  if (SQLITE_ROW != count_stmt.step()) {
    LOG(("WebCacheDB.MaybeDeletePayload failed\n"));
    return false;
  }

  if (count_stmt.column_int64(0) == 0) {
    if (!DeletePayload(payload_id)) {
      return false;
    }
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// DeletePayload
//------------------------------------------------------------------------------
bool WebCacheDB::DeletePayload(int64 payload_id) {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "DeletePayload");
  if (!transaction.Begin()) {
    return false;
  }

  const char16 *sql = STRING16(L"DELETE FROM Payloads "
                               L"WHERE PayloadID=?");
  SQLStatement delete_stmt;
  int rv = delete_stmt.prepare16(&db_, sql);
  rv |= delete_stmt.bind_int64(0, payload_id);
  if (SQLITE_OK != rv) {
    LOG(("WebCacheDB.DeletePayload failed\n"));
    return false;
  }
  if (SQLITE_DONE != delete_stmt.step()) {
    LOG(("WebCacheDB.DeletePayload failed\n"));
    return false;
  }

  // Delete the response body.
  if (!response_bodies_store_->DeleteBody(payload_id)) {
    return false;
  }

  return transaction.Commit();
}

#ifdef USE_FILE_STORE
//------------------------------------------------------------------------------
// Called after a top transaction has begun
//------------------------------------------------------------------------------
void WebCacheDB::OnBegin() {
  response_bodies_store_->BeginTransaction();
}

//------------------------------------------------------------------------------
// Called after a top transaction has been commited
//------------------------------------------------------------------------------
void WebCacheDB::OnCommit() {
  response_bodies_store_->CommitTransaction();
}

//------------------------------------------------------------------------------
// Called after a top transaction has been rolledback
//------------------------------------------------------------------------------
void WebCacheDB::OnRollback() {
  LOG(("WebCacheDB.OnRollback\n"));
  response_bodies_store_->RollbackTransaction();
}
#endif


//------------------------------------------------------------------------------
// GetDB
//------------------------------------------------------------------------------
// static
WebCacheDB *WebCacheDB::GetDB() {
  if (ThreadLocals::HasValue(kThreadLocalKey)) {
    return reinterpret_cast<WebCacheDB*>
      (ThreadLocals::GetValue(kThreadLocalKey));
  }

  WebCacheDB *db = new WebCacheDB();

  // If we can't initialize, we store NULL in the map so that we don't keep
  // trying to Init() over and over.
  if (!db->Init()) {
    delete db;
    db = NULL;
  }

  ThreadLocals::SetValue(kThreadLocalKey, db, &DestroyDB);
  return db;
}


//------------------------------------------------------------------------------
// Destructor function called by ThreadLocals to dispose of a thread-specific
// DB instance when a thread dies.
//------------------------------------------------------------------------------
// static
void WebCacheDB::DestroyDB(void* pvoid) {
  WebCacheDB *db = reinterpret_cast<WebCacheDB*>(pvoid);
  if (db) {
    delete db;
  }
}


//------------------------------------------------------------------------------
// PayloadInfo::GetHeader
//------------------------------------------------------------------------------
bool WebCacheDB::PayloadInfo::GetHeader(const char16* name,
                                        std::string16 *value) {
  assert(name);
  assert(value);
  if (!name || !value) {
    return false;
  }

  std::string headers_ascii;
  String16ToUTF8(headers.c_str(), headers.length(), &headers_ascii);
  const char *body = headers_ascii.c_str();
  uint32 body_len = headers_ascii.length();
  HTTPHeaders parsed_headers;
  if (!HTTPUtils::ParseHTTPHeaders(&body, &body_len, &parsed_headers,
                                   true /* allow_const_cast */)) {
    return false;
  }

  std::string name_ascii;
  String16ToUTF8(name, &name_ascii);
  const char *value_ascii = parsed_headers.GetHeader(name_ascii.c_str());
  if (!value_ascii) {
    return false;
  }
  return UTF8ToString16(value_ascii, value);
}

bool WebCacheDB::PayloadInfo::IsHttpRedirect() {
  // TODO(michaeln): what about other redirects:
  // 300(multiple), 303(posts), 307(temp)
  return (status_code == HttpConstants::HTTP_FOUND) ||
         (status_code == HttpConstants::HTTP_MOVED);
}

bool WebCacheDB::PayloadInfo::ConvertToHtmlRedirect(bool head_only) {
  if (!IsHttpRedirect()) {
    return false;
  }
  std::string16 location;
  GetHeader(HttpConstants::kLocationHeader, &location);
  if (location.empty()) {
    return false;
  }
  SynthesizeHtmlRedirect(location.c_str(), head_only);
  return true;
}

void WebCacheDB::PayloadInfo::SynthesizeHtmlRedirect(const char16 *location,
                                                     bool head_only) {
  static const std::string16 kHeaders(
      STRING16(L"Content-Type: text/html\r\n\r\n"));

  status_line = STRING16(L"HTTP/1.0 200 OK");
  status_code = HttpConstants::HTTP_OK;
  headers = kHeaders;
  data.reset();
#ifdef USE_FILE_STORE
  cached_filepath.clear();
#endif

  if (!head_only) {
    static const std::string kHtmlRedirectStart(
        "<html><head><meta HTTP-EQUIV=\"REFRESH\" content=\"0; url=");
    static const std::string kHtmlRedirectEnd("\"></head></html>");

    std::string location_utf8;
    String16ToUTF8(location, &location_utf8);

    std::vector<uint8> *buf = new std::vector<uint8>;
    buf->resize(kHtmlRedirectStart.length()
                + location_utf8.length()
                + kHtmlRedirectEnd.length());
    memcpy(&(*buf)[0],
           kHtmlRedirectStart.c_str(),
           kHtmlRedirectStart.length());
    memcpy(&(*buf)[kHtmlRedirectStart.length()],
           location_utf8.c_str(),
           location_utf8.length());
    memcpy(&(*buf)[kHtmlRedirectStart.length() + location_utf8.length()],
           kHtmlRedirectEnd.c_str(),
           kHtmlRedirectEnd.length());

    data.reset(buf);
  }
}

bool
WebCacheDB::PayloadInfo::CanonicalizeHttpRedirect(const char16 *base_url) {
  if (!IsHttpRedirect()) {
    return false;
  }
  std::string16 location;
  GetHeader(HttpConstants::kLocationHeader, &location);
  if (location.empty()) {
    return false;
  }
  return SynthesizeHttpRedirect(base_url, location.c_str());
}

bool
WebCacheDB::PayloadInfo::SynthesizeHttpRedirect(const char16 *base_url,
                                                const char16 *location) {
  std::string16 full_location;
  if (base_url) {
    if (!ResolveAndNormalize(base_url, location, &full_location)) {
      return false;
    }
  } else {
#ifdef DEBUG
    static const char16 *kHttpPrefix = STRING16(L"http://");
    static const char16 *kHttpsPrefix = STRING16(L"https://");
    assert((memistr(location, kHttpPrefix) == location) ||
           (memistr(location, kHttpsPrefix) == location));
#endif
    full_location = location;
  }

  status_line = STRING16(L"HTTP/1.0 302 FOUND");
  status_code = HttpConstants::HTTP_FOUND;
  headers = HttpConstants::kLocationHeader;
  headers += STRING16(L": ");
  headers += full_location;
  headers += HttpConstants::kCrLf;
  headers += HttpConstants::kCrLf;
  data.reset(new std::vector<uint8>);
#ifdef USE_FILE_STORE
  cached_filepath.clear();
#endif
  is_synthesized_http_redirect = true;
  return true;
}

bool WebCacheDB::PayloadInfo::PassesValidationTests(
                                  std::string16 *adjusted_headers) {
  int status_line_status_code;
  if (!IsValidResponseCode(status_code) ||
      !ParseHttpStatusLine(status_line, NULL, &status_line_status_code, NULL) ||
      (status_code != status_line_status_code)) {
    ExceptionManager::ReportAndContinue();
    return false;
  }
  std::string headers_ascii;
  String16ToUTF8(headers.c_str(), headers.length(), &headers_ascii);
  const std::string kBlankLine("\r\n\r\n");
  if (!EndsWith(headers_ascii, kBlankLine)) {
    ExceptionManager::ReportAndContinue();
    return false;
  }
  const char *body = headers_ascii.c_str();
  uint32 body_len = headers_ascii.length();
  HTTPHeaders parsed_headers;
  if (!HTTPUtils::ParseHTTPHeaders(&body, &body_len, &parsed_headers,
                                   true /* allow_const_cast */)) {
    ExceptionManager::ReportAndContinue();
    return false;
  }

  int64 received_data_size = data.get() ? data->size() : 0;

  // If there is a custom 'X-Gears-Decoded-Content-Length' header, we
  // validate against that value.
  const char *decoded_length_header = parsed_headers.GetHeader(
                  HttpConstants::kXGearsDecodedContentLengthAscii);
  if (decoded_length_header) {
    if (received_data_size != static_cast<int64>(atoi(decoded_length_header))) {
      ExceptionManager::ReportAndContinue();
      return false;
    }
  }

  // This is to defend against inserting degenerate responses that we
  // occasionally capture in Firefox. In that case we get a valid status
  // line saying OK, and a no other headers except the content-length
  // header that we synthesized (rather than actually received). To be
  // consistent across platforms, we'll reject this form of response
  // in all cases, even if this is actually what the server sent us.
  //
  // TODO(michaeln): If and when we get a handle on the source of that
  // problem, revisit this part of the validity test. I think the addition
  // of content length validation fixes this, leaving in until we get
  // confirmation.
  if (status_code == HttpConstants::HTTP_OK && 
      parsed_headers.HeaderIs(HTTPHeaders::CONTENT_LENGTH, "0")) {
    parsed_headers.ClearHeader(HTTPHeaders::CONTENT_LENGTH);
    if (parsed_headers.IsEmpty()) {
      ExceptionManager::ReportAndContinue();
      return false;
    }
  }

  // We fix up the "Content-Length" header and remove any "Content-Encoding"
  // headers since the response body we have has already been decoded.
  if (adjusted_headers) {
    parsed_headers.SetHeader(HTTPHeaders::CONTENT_LENGTH,
                             Integer64ToString(received_data_size).c_str(),
                             HTTPHeaders::OVERWRITE);
    parsed_headers.ClearHeader(HTTPHeaders::CONTENT_ENCODING);

    std::string header_str;
    for (HTTPHeaders::const_iterator hdr = parsed_headers.begin();
         hdr != parsed_headers.end();
         ++hdr) {
      if (hdr->second != NULL) {  // NULL means do not output
        header_str += hdr->first;
        header_str += ": ";
        header_str += hdr->second;
        header_str += HttpConstants::kCrLfAscii;
      }
    }
    header_str += HttpConstants::kCrLfAscii;  // blank line at the end
    if (!UTF8ToString16(header_str.c_str(), header_str.length(),
                        adjusted_headers)) {
      return false;
    }
  }

  return true;
}
