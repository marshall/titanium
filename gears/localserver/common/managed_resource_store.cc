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

#include "gears/localserver/common/managed_resource_store.h"
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"  // For BrowserCache
#endif
#include "gears/localserver/common/manifest.h"
#include "gears/localserver/common/update_task.h"


//------------------------------------------------------------------------------
// Exists
//------------------------------------------------------------------------------
// static
bool ManagedResourceStore::ExistsInDB(const SecurityOrigin &security_origin,
                                      const char16 *name,
                                      const char16 *required_cookie,
                                      int64 *server_id_out) {
  return LocalServer::ExistsInDB(security_origin, name, required_cookie,
                                 WebCacheDB::MANAGED_RESOURCE_STORE,
                                 server_id_out);
}

//------------------------------------------------------------------------------
// FindServer
//------------------------------------------------------------------------------
// static
bool ManagedResourceStore::FindServer(const SecurityOrigin &security_origin,
                                      const char16 *name,
                                      const char16 *required_cookie,
                                      WebCacheDB::ServerInfo *server) {
  return LocalServer::FindServer(security_origin, name, required_cookie,
                                 WebCacheDB::MANAGED_RESOURCE_STORE,
                                 server);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ManagedResourceStore::ManagedResourceStore()
    : LocalServer(WebCacheDB::MANAGED_RESOURCE_STORE) {
}

//------------------------------------------------------------------------------
// CreateOrOpen
//------------------------------------------------------------------------------
bool ManagedResourceStore::CreateOrOpen(const SecurityOrigin &security_origin,
                                        const char16 *name,
                                        const char16 *required_cookie) {
  if (!LocalServer::CreateOrOpen(security_origin, name, required_cookie)) {
    return false;
  }
  is_initialized_ = true;
  return true;
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
bool ManagedResourceStore::Open(int64 server_id) {
  if (!LocalServer::Open(server_id)) {
    return false;
  }
  is_initialized_ = true;
  return true;
}

//------------------------------------------------------------------------------
// GetManifestUrl
//------------------------------------------------------------------------------
bool ManagedResourceStore::GetManifestUrl(std::string16 *manifest_url) {
  WebCacheDB::ServerInfo server;
  if (!GetServer(&server)) {
    return false;
  }
  *manifest_url = server.manifest_url;
  return true;
}

//------------------------------------------------------------------------------
// SetManifestUrl
//------------------------------------------------------------------------------
bool ManagedResourceStore::SetManifestUrl(const char16 *manifest_url) {
  assert(manifest_url);
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  std::string16 existing_manifest_url;
  if (!GetManifestUrl(&existing_manifest_url)) {
    return false;
  }

  if (existing_manifest_url != manifest_url) {
    WebCacheDB *db = WebCacheDB::GetDB();
    if (!db || !db->UpdateServer(server_id_, manifest_url)) {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------------
// AddManifestAsDownloadingVersion
//------------------------------------------------------------------------------
bool ManagedResourceStore::AddManifestAsDownloadingVersion(Manifest *manifest,
                                                           int64 *version_id) {
  assert(manifest);

  if (!manifest->IsValid()) {
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  SQLTransaction transaction(db->GetSQLDatabase(),
                             "AddManifestAsDownloadingVersion");
  if (!transaction.Begin()) {
    return false;
  }

  // We can only do this if this application exists in the DB.
  if (!StillExistsInDB()) {
    return false;
  }

  // If this version is already added, fail.
  if (HasVersion(manifest->GetVersion())) {
    return false;
  }

  // If there is already a version being downloaded, delete it.
  WebCacheDB::VersionInfo existing_downloading_version;
  if (GetVersion(WebCacheDB::VERSION_DOWNLOADING,
                 &existing_downloading_version)) {
    if (!db->DeleteVersion(existing_downloading_version.id)) {
      return false;
    }
  }

  // Insert a new row in the Versions table.
  WebCacheDB::VersionInfo version;
  version.ready_state = WebCacheDB::VERSION_DOWNLOADING;
  version.server_id = server_id_;
  version.version_string = manifest->GetVersion();
  version.session_redirect_url = manifest->GetRedirectUrl();
  if (!db->InsertVersion(&version)) {
    return false;
  }

  // Insert a new row in the Entries table for each Manifest entry
  const std::vector<Manifest::Entry> *entries = manifest->GetEntries();
  for (std::vector<Manifest::Entry>::const_iterator iter = entries->begin();
       iter != entries->end(); ++iter) {
    WebCacheDB::EntryInfo entry;
    entry.version_id = version.id;
    entry.url = iter->url;
    entry.src = iter->src;
    entry.redirect = iter->redirect;
    entry.ignore_query = iter->ignore_query;

    // If the entry has a redirect, synthesize a 302 response and store
    // that as the payload for this entry. The redirect is expected to
    // be a full url
    if (!entry.redirect.empty()) {
      WebCacheDB::PayloadInfo payload;
      payload.SynthesizeHttpRedirect(NULL, entry.redirect.c_str());
      if (!db->InsertPayload(server_id_, entry.url.c_str(), &payload)) {
        return false;
      }
      entry.payload_id = payload.id;
    }

    if (!db->InsertEntry(&entry)) {
      return false;
    }
  }

  if (version_id) {
    *version_id = version.id;
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// SetDownloadingVersionAsCurrent
//------------------------------------------------------------------------------
bool ManagedResourceStore::SetDownloadingVersionAsCurrent() {
  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  SQLTransaction transaction(db->GetSQLDatabase(),
                             "SetDownloadingVersionAsCurrent");
  if (!transaction.Begin()) {
    return false;
  }

  // Get info about the downloading version, if there is none we fail
  WebCacheDB::VersionInfo downloading_version;
  if (!GetVersion(WebCacheDB::VERSION_DOWNLOADING,
                  &downloading_version)) {
    return false;
  }

  // Delete the current version if one exists
  WebCacheDB::VersionInfo existing_current_version;
  if (GetVersion(WebCacheDB::VERSION_CURRENT,
                 &existing_current_version)) {
    if (!db->DeleteVersion(existing_current_version.id)) {
      return false;
    }
  }

  // Set the downloading version to current
  if (!db->UpdateVersion(downloading_version.id,
                         WebCacheDB::VERSION_CURRENT)) {
    return false;
  }

  return transaction.Commit();
}

//------------------------------------------------------------------------------
// GetVersionString
//------------------------------------------------------------------------------
bool ManagedResourceStore::GetVersionString(WebCacheDB::VersionReadyState state,
                                            std::string16 *version_string) {
  WebCacheDB::VersionInfo version;
  if (GetVersion(state, &version)) {
    *version_string = version.version_string;
    return true;
  } else {
    return false;
  }
}

//------------------------------------------------------------------------------
// GetUpdateInfo
//------------------------------------------------------------------------------
bool ManagedResourceStore::GetUpdateInfo(WebCacheDB::UpdateStatus *status,
                                         int64 *last_time,
                                         std::string16 *manifest_date_header,
                                         std::string16 *update_error) {
  WebCacheDB::ServerInfo server;
  if (GetServer(&server)) {
    // For status, what's stored in the DB and what is actually happening
    // can be out of sync in the face of browser crashes. If the DB indicates
    // that a task is running, we test for that, and if no task is running
    // translate the value to UPDATE_FAILED
    if ((server.update_status == WebCacheDB::UPDATE_CHECKING ||
         server.update_status == WebCacheDB::UPDATE_DOWNLOADING) &&
        !UpdateTask::IsUpdateTaskForStoreRunning(server_id_)) {
      server.update_status = WebCacheDB::UPDATE_FAILED;
    }
    *status = server.update_status;
    *last_time = server.last_update_check_time;
    if (manifest_date_header) {
      *manifest_date_header = server.manifest_date_header;
    }

    // There can only be an error message if we are in the failed state
    if (update_error) {
      if (*status == WebCacheDB::UPDATE_FAILED) {
        *update_error = server.last_error_message;
      } else {
        update_error->clear();
      }
    }

    return true;
  } else {
    return false;
  }
}

//------------------------------------------------------------------------------
// SetUpdateInfo
//------------------------------------------------------------------------------
bool ManagedResourceStore::SetUpdateInfo(WebCacheDB::UpdateStatus status,
                                         int64 last_time,
                                         const char16 *manifest_date_header,
                                         const char16 *update_error) {
  WebCacheDB::ServerInfo server;
  if (GetServer(&server)) {
    WebCacheDB *db = WebCacheDB::GetDB();
    if (!db) {
      return false;
    }
    return db->UpdateServer(server.id, status, last_time,
                            manifest_date_header, update_error);
  } else {
    return false;
  }
}

#ifdef WINCE
//------------------------------------------------------------------------------
// InsertBogusBrowserCacheEntries
//------------------------------------------------------------------------------
bool ManagedResourceStore::InsertBogusBrowserCacheEntries() {
  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }
  WebCacheDB::VersionInfo version;
  if (!GetVersion(WebCacheDB::VERSION_CURRENT, &version)) {
    // No current version is not an error.
    return true;
  }
  typedef std::vector<WebCacheDB::EntryInfo> EntryInfoVector;
  EntryInfoVector entries;    
  if (!db->FindEntries(version.id, &entries)) {
    return false;
  }
  bool res = true;
  for (EntryInfoVector::iterator entry = entries.begin();
       entry != entries.end();
       ++entry) {
    res &= BrowserCache::EnsureBogusEntry(entry->url.c_str());
  }
  return res;
}

//------------------------------------------------------------------------------
// GetCurrentVersionUrls
//------------------------------------------------------------------------------
bool ManagedResourceStore::GetCurrentVersionUrls(
    std::vector<std::string16> *urls) {
  assert(urls);
  assert(urls->empty());
  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }
  WebCacheDB::VersionInfo version;
  if (!GetVersion(WebCacheDB::VERSION_CURRENT, &version)) {
    // No current version is not an error.
    return true;
  }
  std::vector<WebCacheDB::EntryInfo> entries;
  if (!db->FindEntries(version.id, &entries)) {
    return false;
  }
  for (int i = 0; i < static_cast<int>(entries.size()); ++ i) {
    urls->push_back(entries[i].url);
  }
  return true;
}
#endif
