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

#include "gears/localserver/common/resource_store.h"
#include <vector>
#include "gears/base/common/string16.h"
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"  // For BrowserCache
#endif
#include "gears/blob/blob_interface.h"

//------------------------------------------------------------------------------
// ExistsInDB
//------------------------------------------------------------------------------
// static
bool ResourceStore::ExistsInDB(const SecurityOrigin &security_origin,
                               const char16 *name,
                               const char16 *required_cookie,
                               int64 *server_id_out) {
  return LocalServer::ExistsInDB(security_origin, name, required_cookie,
                                 WebCacheDB::RESOURCE_STORE,
                                 server_id_out);
}

//------------------------------------------------------------------------------
// FindServer
//------------------------------------------------------------------------------
// static
bool ResourceStore::FindServer(const SecurityOrigin &security_origin,
                               const char16 *name,
                               const char16 *required_cookie,
                               WebCacheDB::ServerInfo *server) {
  return LocalServer::FindServer(security_origin, name, required_cookie,
                                 WebCacheDB::RESOURCE_STORE,
                                 server);
}

//------------------------------------------------------------------------------
// AppendHeader
//------------------------------------------------------------------------------
static void AppendHeader(std::string16 &headers,
                         const char16 *name,
                         const char16 *value) {
  const char16 *kDelimiter = STRING16(L": ");
  headers.append(name);
  headers.append(kDelimiter);
  headers.append(value);
  headers.append(HttpConstants::kCrLf);
}

//------------------------------------------------------------------------------
// BlobToItem
//------------------------------------------------------------------------------
// static
bool ResourceStore::BlobToItem(BlobInterface *blob,
                               const char16 *full_url,
                               const char16 *optional_content_type,
                               const char16 *optional_x_captured_filename,
                               Item *item) {
  int64 file_size = blob->Length();
  // We don't support very large files yet.
  if (file_size > kint32max) {
    return false;
  }
  int data_len = static_cast<int>(file_size);
  if (optional_content_type == NULL) {
    optional_content_type = STRING16(L"application/octet-stream");
  }

  item->entry.url = full_url;
  item->payload.status_code = HttpConstants::HTTP_OK;
  item->payload.status_line = HttpConstants::kOKStatusLine;

  // Copy the blob data into the Item
  item->payload.data.reset(new std::vector<uint8>);
  if (data_len > 0) {
    item->payload.data->resize(data_len);
    if (item->payload.data->size() != static_cast<uint32>(file_size)) {
      return false;
    }
    blob->Read(reinterpret_cast<uint8*>(&(item->payload.data->at(0))),
               0, data_len);
  }

  // Synthesize the http headers we'll store with this item
  std::string16 headers;
  std::string16 data_len_str = IntegerToString16(data_len);
  AppendHeader(headers, HttpConstants::kContentLengthHeader,
               data_len_str.c_str());
  AppendHeader(headers, HttpConstants::kContentTypeHeader,
               optional_content_type);
  if (optional_x_captured_filename) {
    AppendHeader(headers, HttpConstants::kXCapturedFilenameHeader,
                 optional_x_captured_filename);
  }
  headers.append(HttpConstants::kCrLf);  // Terminiate with a blank line
  item->payload.headers = headers;
  return true;
}


//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ResourceStore::ResourceStore()
    : LocalServer(WebCacheDB::RESOURCE_STORE),
      version_id_(WebCacheDB::kUnknownID) {
}


//------------------------------------------------------------------------------
// CreateOrOpen
//------------------------------------------------------------------------------
bool ResourceStore::CreateOrOpen(const SecurityOrigin &security_origin,
                                 const char16 *name,
                                 const char16 *required_cookie) {
  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  SQLTransaction transaction(db->GetSQLDatabase(),
                             "ResourceStore::CreateOrOpen");
  if (!transaction.Begin()) {
    return false;
  }

  if (!LocalServer::CreateOrOpen(security_origin, name, required_cookie)) {
    return false;
  }

  // We set is_initialized_ here so GetVersion will function for us
  is_initialized_ = true;

  WebCacheDB::VersionInfo version;
  if (!GetVersion(WebCacheDB::VERSION_CURRENT, &version)) {
    version.ready_state = WebCacheDB::VERSION_CURRENT;
    version.server_id = server_id_;
    if (!db->InsertVersion(&version)) {
      is_initialized_ = false;  // have to reset, initialization has failed
      return false;
    }
  }
  version_id_ = version.id;

  is_initialized_ = transaction.Commit();

  return is_initialized_;
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
bool ResourceStore::Open(int64 server_id) {
  if (!LocalServer::Open(server_id)) {
    return false;
  }
  is_initialized_ = true;
  WebCacheDB::VersionInfo version;
  if (!GetVersion(WebCacheDB::VERSION_CURRENT, &version)) {
    is_initialized_ = false;  // have to reset, initialization has failed
    return false;
  }
  version_id_ = version.id;
  return true;
}

//------------------------------------------------------------------------------
// GetItem
//------------------------------------------------------------------------------
bool ResourceStore::GetItem(const char16 *url, Item *item, bool info_only) {
  assert(url);
  assert(item);
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  if (!db->FindEntry(version_id_, url, &item->entry)) {
    return false;
  }

  // All entries in a ResourceStore are expected to have a payload
  assert(item->entry.payload_id != WebCacheDB::kUnknownID);

  return db->FindPayload(item->entry.payload_id, &item->payload, info_only);
}


//------------------------------------------------------------------------------
// PutItem
//------------------------------------------------------------------------------
bool ResourceStore::PutItem(Item *item) {
  assert(item);
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  SQLTransaction transaction(db->GetSQLDatabase(), "ResourceStore::PutItem");
  if (!transaction.Begin()) {
    return false;
  }

  if (!StillExistsInDB()) {
    return false;
  }

  db->DeleteEntry(version_id_, item->entry.url.c_str());

  if (!db->InsertPayload(server_id_, item->entry.url.c_str(), &item->payload)) {
    return false;
  }

  item->entry.version_id = version_id_;
  item->entry.payload_id = item->payload.id;
  if (!db->InsertEntry(&item->entry)) {
    return false;
  }

  bool committed = transaction.Commit();
#ifdef WINCE
  if (committed) {
    BrowserCache::EnsureBogusEntry(item->entry.url.c_str());
  }
#endif
  return committed;
}


//------------------------------------------------------------------------------
// DeleteAll
//------------------------------------------------------------------------------
bool ResourceStore::DeleteAll() {
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  SQLTransaction transaction(db->GetSQLDatabase(), "ResourceStore::DeleteAll");
  if (!transaction.Begin()) {
    return false;
  }

#ifdef WINCE
  std::vector<WebCacheDB::EntryInfo> entries;
  db->FindEntries(version_id_, &entries);
#endif

  if (!db->DeleteEntries(version_id_)) {
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
// Delete
//------------------------------------------------------------------------------
bool ResourceStore::Delete(const char16* url) {
  assert(url);
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  bool deleted = db->DeleteEntry(version_id_, url);
#ifdef WINCE
  if (deleted) {
    BrowserCache::RemoveBogusEntry(url);
  }
#endif
  return deleted;
}


//------------------------------------------------------------------------------
// Rename
//------------------------------------------------------------------------------
bool ResourceStore::Rename(const char16* orig_url, const char16 *new_url) {
  assert(orig_url);
  assert(new_url);
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  SQLTransaction transaction(db->GetSQLDatabase(), "ResourceStore::Rename");
  if (!transaction.Begin()) {
    return false;
  }

  if (!IsCaptured(orig_url)) {
    return false;
  }

  // Delete any pre-existing entry stored under the new_url
  // Note: delete does not fail is there is no entry for new_url
  if (!db->DeleteEntry(version_id_, new_url)) {
    return false;
  }

  if (!db->UpdateEntry(version_id_, orig_url, new_url)) {
    return false;
  }

  bool committed = transaction.Commit();
#ifdef WINCE
  if (committed) {
    BrowserCache::RemoveBogusEntry(orig_url);
    BrowserCache::EnsureBogusEntry(new_url);
  }
#endif
  return committed;
}


//------------------------------------------------------------------------------
// Copy
//------------------------------------------------------------------------------
bool ResourceStore::Copy(const char16* src_url, const char16 *dst_url) {
  assert(src_url);
  assert(dst_url);
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  SQLTransaction transaction(db->GetSQLDatabase(), "ResourceStore::Copy");
  if (!transaction.Begin()) {
    return false;
  }

  Item item;
  if (!GetItemInfo(src_url, &item)) {
    return false;
  }

  // Delete any pre-existing entry stored under the dst_url
  // Note: delete does not fail is there is no entry for dst_url
  if (!db->DeleteEntry(version_id_, dst_url)) {
    return false;
  }

  // Insert a new entry for dst_url that references the same payload
  item.entry.id = WebCacheDB::kUnknownID;
  item.entry.url = dst_url;
  item.entry.version_id = version_id_;
  if (!db->InsertEntry(&item.entry)) {
    return false;
  }

  bool committed = transaction.Commit();
#ifdef WINCE
  if (committed) {
    BrowserCache::EnsureBogusEntry(item.entry.url.c_str());
  }
#endif
  return committed;
}


//------------------------------------------------------------------------------
// IsCaptured
//------------------------------------------------------------------------------
bool ResourceStore::IsCaptured(const char16 *url) {
  Item item;
  return GetItemInfo(url, &item);
}


//------------------------------------------------------------------------------
// GetCapturedFileName
//------------------------------------------------------------------------------
bool ResourceStore::GetCapturedFileName(const char16* url,
                                        std::string16 *file_name) {
  Item item;
  if (!GetItemInfo(url, &item)) {
    return false;
  }
  // If this resource was not captured via captureFile, we return an empty
  // string instead of an error.
  file_name->clear();
  item.payload.GetHeader(HttpConstants::kXCapturedFilenameHeader, file_name);
  return true;
}


//------------------------------------------------------------------------------
// GetAllHeaders
//------------------------------------------------------------------------------
bool ResourceStore::GetAllHeaders(const char16 *url, std::string16 *headers) {
  Item item;
  if (!GetItemInfo(url, &item)) {
    return false;
  }
  *headers = item.payload.headers;
  return true;
}


//------------------------------------------------------------------------------
// GetHeader
//------------------------------------------------------------------------------
bool ResourceStore::GetHeader(const char16* url,
                              const char16* name,
                              std::string16 *value) {
  Item item;
  if (!GetItemInfo(url, &item)) {
    return false;
  }
  return item.payload.GetHeader(name, value);
}
