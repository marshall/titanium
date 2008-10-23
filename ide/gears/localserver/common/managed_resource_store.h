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

#ifndef GEARS_LOCALSERVER_COMMON_MANAGED_RESOURCE_STORE_H__
#define GEARS_LOCALSERVER_COMMON_MANAGED_RESOURCE_STORE_H__

#include "gears/base/common/common.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/localserver.h"
#include "gears/localserver/common/localserver_db.h"

class Manifest;

//------------------------------------------------------------------------------
// ManagedResourceStore
// TODO(michaeln): some documentation goes here
//------------------------------------------------------------------------------
class ManagedResourceStore : public LocalServer {
 public:
  // Returns true if the application exists in the DB
  static bool ExistsInDB(const SecurityOrigin &security_origin,
                         const char16 *name,
                         const char16 *required_cookie,
                         int64 *server_id_out);

  // Constructor
  ManagedResourceStore();

  // Initializes an instance and inserts a row in the Servers table of
  // of the DB if one is not already present.
  virtual bool CreateOrOpen(const SecurityOrigin &security_origin,
                            const char16 *name,
                            const char16 *required_cookie);

  // Initializes an instance from its server_id. Will not insert a row into
  // the Servers table of the DB. If there is no row present with this id,
  // the method fails and returns false.
  virtual bool Open(int64 server_id);

  // Initialzes one instance from another. Will not create the Server row
  // in the DB if needed.
  bool Clone(ManagedResourceStore *store) {
    if (!LocalServer::Clone(store)) {
      return false;
    }
    is_initialized_ = store->is_initialized_;
    return is_initialized_;
  }

  // Returns the manifest url for this application
  bool GetManifestUrl(std::string16 *manifest_url);

  // Sets the manifest url for this application
  bool SetManifestUrl(const char16 *manifest_url);

  // Returns if the application has a version in the desired state
  bool HasVersion(WebCacheDB::VersionReadyState state) {
    WebCacheDB::VersionInfo version;
    return GetVersion(state, &version);
  }

  // Returns if the application has a version with the desired version_string
  bool HasVersion(const char16 *version_string) {
    WebCacheDB::VersionInfo version;
    return GetVersion(version_string, &version);
  }

  // Returns the version string of the version having the desired state
  bool GetVersionString(WebCacheDB::VersionReadyState state,
                        std::string16 *version_string);

  // Retrieves the update info for this application
  bool GetUpdateInfo(WebCacheDB::UpdateStatus *status,
                     int64 *last_time,
                     std::string16 *manifest_date_header,
                     std::string16 *error_message);
#ifdef WINCE
  // Retrieves from the database the URLs for the current version and inserts an
  // empty entry in the browser cache for each.
  bool InsertBogusBrowserCacheEntries();
  bool GetCurrentVersionUrls(std::vector<std::string16> *urls);
#endif

 private:
  friend class UpdateTask;
#if BROWSER_IE
  friend class IEUpdateTask;
#elif BROWSER_FF || BROWSER_SAFARI || defined(OS_ANDROID)
  friend class UpdateTaskSingleProcess;
#elif BROWSER_NPAPI
  friend class NPUpdateTask;
#endif

  // Returns the the rowid or our Server record in the DB
  int64 GetServerID() {
    return server_id_;
  }

  // Retrieves from the DB the server info for this application based our
  // identifying origin/name/required_cookie tuple.
  static bool FindServer(const SecurityOrigin &security_origin,
                         const char16 *name,
                         const char16 *required_cookie,
                         WebCacheDB::ServerInfo *server);

  // Adds a new version to this application.  The new version is created
  // with the downloading ready state.  Any pre-existing version with the
  // downloading ready state is deleted.
  bool AddManifestAsDownloadingVersion(Manifest *manifest, int64 *version_id);

  // Transitions a version from the downloading ready state to the current
  // ready state. Any pre-existing version with the current ready state is
  // deleted.
  bool SetDownloadingVersionAsCurrent();

  // Sets the update info for this applicaiton
  bool SetUpdateInfo(WebCacheDB::UpdateStatus status,
                     int64 last_time,
                     const char16 *manifest_date_header,
                     const char16 *update_error);

#ifdef USING_CCTESTS
  friend bool TestManagedResourceStore(std::string16 *error);
#endif
};

#endif  // GEARS_LOCALSERVER_COMMON_MANAGED_RESOURCE_STORE_H__
