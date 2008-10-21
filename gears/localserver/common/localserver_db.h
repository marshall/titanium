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

#ifndef GEARS_LOCALSERVER_COMMON_LOCALSERVER_DB_H__
#define GEARS_LOCALSERVER_COMMON_LOCALSERVER_DB_H__

#include <vector>
#include "gears/base/common/common.h"
#include "gears/base/common/name_value_table.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/http_constants.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// IE and Firefox keep cached files in the file system. Safari does not
// yet do so.
#if BROWSER_IE
#define USE_FILE_STORE defined
#elif BROWSER_FF
#define USE_FILE_STORE defined
#elif BROWSER_NPAPI
#define USE_FILE_STORE defined
#elif BROWSER_SAFARI
#define USE_FILE_STORE defined
#else
#undef USE_FILE_STORE
#endif

class BrowsingContext;
class CookieMap;
class SecurityOrigin;

//------------------------------------------------------------------------------
// WebCacheDB
//
// This class provides a data access API for web capture data. The underlying
// repository is SQLite. This class encaplates the SQL required to insert,
// update, and delete records into the database.
//
// Note: An instance of this class can only be accessed from a single thread.
//
// TODO(michaeln): some more documentation goes here
//------------------------------------------------------------------------------
class WebCacheDB : SQLTransactionListener {
 public:
  // Gets a thread-specific WebCacheDB instance. If an instance does not
  // yet exist for the current thread, one is created and registered with
  // ThreadLocals.
  static WebCacheDB *GetDB();

  static const char16 *kFilename;      // the name of the database file
  // Note: This used to be kInvalidID, which conflicts with a system macro on
  // MacOSX.
  static const int64 kUnknownID;

  enum ServerType {
    MANAGED_RESOURCE_STORE = 0,
    RESOURCE_STORE = 1
  };

  enum VersionReadyState {
    VERSION_DOWNLOADING = 0,
    VERSION_CURRENT = 1
  };

  enum UpdateStatus {
    UPDATE_OK = 0,
    UPDATE_CHECKING = 1,
    UPDATE_DOWNLOADING = 2,
    UPDATE_FAILED = 3
  };

  struct ServerInfo {
    int64 id;
    bool enabled;
    std::string16 security_origin_url;
    std::string16 name;
    std::string16 required_cookie;
    ServerType server_type;
    std::string16 manifest_url;
    UpdateStatus update_status;
    std::string16 last_error_message;
    int64 last_update_check_time;
    std::string16 manifest_date_header;

    ServerInfo() : id(kUnknownID),
                   enabled(true),
                   server_type(MANAGED_RESOURCE_STORE),
                   update_status(UPDATE_OK),
                   last_update_check_time(0) {}
  };

  struct VersionInfo {
    int64 id;
    int64 server_id;
    std::string16 version_string;
    VersionReadyState ready_state;
    std::string16 session_redirect_url;

    VersionInfo() : id(kUnknownID),
                    server_id(kUnknownID),
                    ready_state(VERSION_DOWNLOADING) {}
  };

  struct EntryInfo {
    int64 id;
    int64 version_id;
    std::string16 url;
    std::string16 src;
    std::string16 redirect;
    bool ignore_query;
    int64 payload_id;

    EntryInfo() : id(kUnknownID),
                  version_id(kUnknownID),
                  ignore_query(false),
                  payload_id(kUnknownID) {}
  };

  struct PayloadInfo {
    int64 id;
    int64 creation_date;
    int status_code;
    std::string16 status_line;
    std::string16 headers;  // Must be terminated with a blank line

    // The following fields are empty for info_only queries
    scoped_ptr< std::vector<uint8> > data;
#ifdef USE_FILE_STORE
    std::string16 cached_filepath;
#endif

    bool is_synthesized_http_redirect;

    PayloadInfo() : id(kUnknownID),
                    creation_date(0),
                    status_code(0),
                    is_synthesized_http_redirect(false) {}

    // Returns a particular header value
    bool GetHeader(const char16* header, std::string16 *value);

    // Returns true if the payload represents an HTTP redirect
    bool IsHttpRedirect();

    // If the payload is an HTTP redirect, its contents are re-written with a
    // canonicalzed representation: 302, full url in a single location header,
    // and no response body. If base_url is not null, it is used to resolve
    // a relative location url. If base_url is null, the location url is
    // assumed to be a full url.
    bool CanonicalizeHttpRedirect(const char16 *base_url);

    // Forms an HTTP redirect payload as described above.
    bool SynthesizeHttpRedirect(const char16 *base_url, const char16 *location);

    // If the payload is an HTTP redirect, its contents are re-written with
    // an HTML document that performs a META REFRESH to the location url
    bool ConvertToHtmlRedirect(bool head_only);

    // Forms an HTML redirect as described above
    void SynthesizeHtmlRedirect(const char16 *location, bool head_only);

    // Test that the status code is valid, and that the status line can
    // be parsed, and that the status code found there in string form agrees
    // with the status code data member, and that the headers end with a
    // blank line and can be parsed without error, and that the received
    // content length matches the custome 'X-Gears-Decoded-Content-Length'
    // header value if present.  'adjusted_headers' is an optional output
    // param. If non-NULL, it will be set to an adjusted set of headers
    // suitable for insertion into the LocalServer database.
    bool PassesValidationTests(std::string16 *adjusted_headers);
  };

  // Returns a pointer to the underlying SQLDatabase
  SQLDatabase *GetSQLDatabase() {
    return &db_;
  }

  // Returns if the database has a response for the url at this time
  bool CanService(const char16 *url, BrowsingContext *browsing_context);

  // Returns a response for the url at this time, if head_only is requested
  // the payload's data will not be populated
  bool Service(const char16 *url, BrowsingContext *browsing_context,
               bool head_only, PayloadInfo *payload);

  // Returns server info for the given server_id
  bool FindServer(int64 server_id, ServerInfo *server);

  // Returns the server info identified by origin, name, required_cookie, and
  // server_type
  bool FindServer(const SecurityOrigin &security_origin,
                  const char16 *name,
                  const char16 *required_cookie,
                  ServerType server_type,
                  ServerInfo *server);

  // Returns all servers for the given origin
  bool FindServersForOrigin(const SecurityOrigin &origin,
                            std::vector<ServerInfo> *versions);

  // Deletes all servers and associated data for the given origin
  bool DeleteServersForOrigin(const SecurityOrigin &origin);

  // Inserts a new row into the Servers table. The id field of the server
  // parameter is updated with the id of the inserted row.
  bool InsertServer(ServerInfo *server);

  // Updates the ManifestUrl column of the Server row and resets the
  // ManifestDateHeader and LastUpdateCheckTime columns.
  bool UpdateServer(int64 server_id, const char16 *manifest_url);

  // Updates the UpdateStatus, LastUpdateCheckTime and optionally the
  // ManifestDateHeader columns of the Servers row.  If
  // manifest_date_header is not null, that column is updated.
  bool UpdateServer(int64 server_id,
                    UpdateStatus update_status,
                    int64 last_update_check_time,
                    const char16 *manifest_date_header,
                    const char16 *error_message);

  // Updates the  Enabled column of Server row
  bool UpdateServer(int64 server_id, bool enabled);

  // Deletes the Server row and all related versions and entries and
  // no longer referenced payloads.
  bool DeleteServer(int64 server_id);


  // Returns the version info for the given server_id and ready_state
  bool FindVersion(int64 server_id,
                   VersionReadyState ready_state,
                   VersionInfo *version);

  // Returns the version info for the given server_id and version_string
  bool FindVersion(int64 server_id,
                   const char16 *version_string,
                   VersionInfo *version);

  // Returns the version info for all versions associated with the given
  // server_id
  bool FindVersions(int64 server_id, std::vector<VersionInfo> *versions);

  // Inserts a new row into the Versions table. The id field of the version
  // parameter is updated with the id of the inserted row.
  bool InsertVersion(VersionInfo *version);

  // Updates the ReadyState column for the given version_id
  bool UpdateVersion(int64 version_id, VersionReadyState ready_state);

  // Deletes the Version row and all related entries and no longer referenced
  // payloads
  bool DeleteVersion(int64 version_id);

  // Deletes all versions related to the given server_id, and all related
  // entries and no longer referenced payloads
  bool DeleteVersions(int64 server_id);

  // Deletes the given version_ids and related entries and no longer referenced
  // payloads
  bool DeleteVersions(std::vector<int64> *versions_ids);

  // Inserts a new row into the Entries table. The id field of the entry
  // parameter is updated with the id of the inserted row.
  bool InsertEntry(EntryInfo *entry);

  // Deletes the entry with the given entry_id. Does not fail if there
  // is no matching entry.
  bool DeleteEntry(int64 entry_id);

  // Returns the entry for the given version_id and url
  bool FindEntry(int64 version_id, const char16 *url, EntryInfo *entry);

  // Returns the entries for the given version_id
  bool FindEntries(int64 version_id, std::vector<EntryInfo> *entries);

  // Returns the entries for the given vector of version_id
  bool FindEntries(std::vector<int64> *version_ids,
                   std::vector<EntryInfo> *entries);

  // Returns the entries for version_id that do no have an associated payload
  // or a redirect specified.
  bool FindEntriesHavingNoResponse(int64 version_id,
                                   std::vector<EntryInfo> *entries);

  // Deletes the entry with the given version_id and url. Does not fail if
  // there is no matching entry.
  bool DeleteEntry(int64 version_id, const char16 *url);

  // Deletes all entries with the given version_id. Does not fail if there
  // are no matching entries.
  bool DeleteEntries(int64 version_id);

  // Deletes all entries for each of the given version_ids. Does not fail
  // if there are no matching entries.
  bool DeleteEntries(std::vector<int64> *versions_ids);

  // Counts the number of entries for the given version_id
  bool CountEntries(int64 version_id, int64 *count);

  // Updates the entry for the given version_id and orig_url to associate
  // it with new_url. Does not fail if there is no matching orig entry.
  bool UpdateEntry(int64 version_id,
                   const char16 *orig_url,
                   const char16 *new_url);

  // Updates all entries for the given version_id and url (or src) to
  // associate them with the given payload_id and redirect_url.
  bool UpdateEntriesWithNewPayload(int64 version_id,
                                   const char16 *url,
                                   int64 payload_id,
                                   const char16 *redirect_url);

  // Returns the payload with the given payload_id. If info_only is true,
  // the data (response body) and cached_filepath fields will be empty
  bool FindPayload(int64 payload_id, PayloadInfo *payload, bool info_only);

  // Inserts a new row into the Payloads table and into the the ResponseBodies
  // table if the payload has a body. If the payload is a redirect, it is
  // canonicalized prior to insertion. We represent redirects in a basic
  // form: 302, full url in the location header, no body. Only payloads passing
  // validation tests (see Payload.PassesValidationTests) will be inserted.
  bool InsertPayload(int64 server_id, const char16 *url, PayloadInfo *payload);

  bool DeleteUnreferencedPayloads();
  bool FindMostRecentPayload(int64 server_id,
                             const char16 *url,
                             PayloadInfo *payload);

 private:
  // Private constructor & destructor, callers must use GetDB()
  WebCacheDB();
  ~WebCacheDB();

  // Must be called prior to any other methods
  bool Init();

  // Helpers used by our public Service and CanService methods
  bool ServiceImpl(const char16 *url,
                   BrowsingContext *browsing_context,
                   PayloadInfo *payload,
                   bool payload_head_only);

  bool DoServiceQuery(const char16 *url,
                      bool exact_match,
                      BrowsingContext *context,
                      const char16 *requested_url,
                      bool *loaded_cookie_map,
                      bool *loaded_cookie_map_ok,
                      CookieMap *cookie_map,
                      std::string16 *possible_session_redirect,
                      int64 *payload_id_out);

  bool ServiceInspectorUrl(const char16 *url,
                           const SecurityOrigin &origin,
                           PayloadInfo *payload);

  // Starts an update task for the specified managed store
  void MaybeInitiateUpdateTask(int64 server_id, BrowsingContext *context);

  // Reads a server record from a row in result set
  void ReadServerInfo(SQLStatement &stmt, ServerInfo *server);

  // Reads a version record from a row in result set
  void ReadVersionInfo(SQLStatement &stmt, VersionInfo *version);

  // Reads an entry record from a row in a result set
  void ReadEntryInfo(SQLStatement &stmt, EntryInfo *entry);

  // Reads a payload record from a row in a result set
  bool ReadPayloadInfo(SQLStatement &stmt,
                       PayloadInfo *payload,
                       bool info_only);

  bool CreateOrUpgradeDatabase();
  bool CreateDatabase();
  bool CreateTables();
  bool CreateIndexes();

#ifdef USING_CCTESTS
  friend bool RunLocalServerPerfTests(int, int, int, std::string16*);
  bool DropIndexes();
#endif

  bool UpgradeFrom10To11();
  bool UpgradeFrom11To12();

  bool ExecuteSqlCommandsInTransaction(const char *commands[], int count);
  bool ExecuteSqlCommands(const char *commands[], int count);

  bool MaybeDeletePayload(int64 payload_id);
  bool DeletePayload(int64 payload_id);

  SQLDatabase db_;
  NameValueTable system_info_table_;

#ifdef USE_FILE_STORE
  friend class WebCacheBlobStore;
  friend class WebCacheFileStore;
  class WebCacheFileStore *response_bodies_store_;

  // Implementation of SQLTransactionListener used to inform the file store
  // of transactions
  virtual void OnBegin();
  virtual void OnCommit();
  virtual void OnRollback();
#else
  friend class WebCacheBlobStore;
  class WebCacheBlobStore *response_bodies_store_;

  // These are a noop when everything is stored in the database
  virtual void OnBegin() {}
  virtual void OnCommit() {}
  virtual void OnRollback() {}
#endif  // USE_FILE_STORE

  static void DestroyDB(void* pvoid);

  DECL_SINGLE_THREAD
  DISALLOW_EVIL_CONSTRUCTORS(WebCacheDB);
};

#endif  // GEARS_LOCALSERVER_COMMON_LOCALSERVER_DB_H__
