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

#ifndef GEARS_LOCALSERVER_COMMON_RESOURCE_STORE_H__
#define GEARS_LOCALSERVER_COMMON_RESOURCE_STORE_H__

#include "gears/base/common/common.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/localserver.h"
#include "gears/localserver/common/localserver_db.h"

class BlobInterface;

//------------------------------------------------------------------------------
// ResourceStore
// TODO(michaeln): some documentation goes here
//------------------------------------------------------------------------------
class ResourceStore : public LocalServer {
 public:
  // Returns true if the store exists in the DB
  static bool ExistsInDB(const SecurityOrigin &security_origin,
                         const char16 *name,
                         const char16 *required_cookie,
                         int64 *server_id_out);

  // Represents an item in the store
  struct Item {
    WebCacheDB::EntryInfo entry;
    WebCacheDB::PayloadInfo payload;
  };

  // Converts a Blob to an Item that could be served by ResourceStore
  static bool BlobToItem(BlobInterface *blob,
                         const char16 *full_url,
                         const char16 *optional_content_type,
                         const char16 *optional_x_captured_filename,
                         Item *out);

  // Constructor
  ResourceStore();

  // Initializes an instance and inserts rows in the Servers and Versions
  // table of the DB if needed
  virtual bool CreateOrOpen(const SecurityOrigin &security_origin,
                            const char16 *name,
                            const char16 *required_cookie);

  // Initializes an instance from its server_id. Will not insert rows into
  // the Servers or Versions table of the DB. If the expected rows are not
  // present in the Servers and Versions table, this method fails and returns
  // false.
  virtual bool Open(int64 server_id);

  // Initialzes one instance from another. Will not insert rows in the DB.
  bool Clone(ResourceStore *store) {
    if (!LocalServer::Clone(store)) {
      return false;
    }
    version_id_ = store->version_id_;
    is_initialized_ = store->is_initialized_;
    return is_initialized_;
  }

  // Gets an item from the store including the response body
  bool GetItem(const char16 *url, Item *item) {
    return GetItem(url, item, false);
  }

  // Gets an item from the store without retrieving the response body
  bool GetItemInfo(const char16 *url, Item *item) {
    return GetItem(url, item, true);
  }

  // Puts an item in the store. If an item already exists for this url, it is
  // overwritten.
  bool PutItem(Item *item);

  // Deletes all items in the store.
  bool DeleteAll();

  // Deletes a single item in the store.
  bool Delete(const char16 *url);

  // Renames an item in  the store. If an item already exists for new_url,
  // the pre-existing item is deleted.
  bool Rename(const char16 *orig_url, const char16 *new_url);

  // Copies an item in  the store. If an item already exists for dst_url,
  // the pre-existing item is deleted.
  bool Copy(const char16 *src_url, const char16 *dst_url);

  // Returns true if an item is captured for url.
  bool IsCaptured(const char16 *url);

  // Returns the filename of a captured local file.
  bool GetCapturedFileName(const char16 *url, std::string16 *file_name);

  // Returns the named http header value for url.
  bool GetHeader(const char16 *url, const char16 *header, std::string16 *value);

  // Returns all http headers for url.
  bool GetAllHeaders(const char16 *url, std::string16 *headers);

 private:
  // Gets an item from the store. If info_only is true, the response body
  // of is not retrieved.
  bool GetItem(const char16 *url, Item *item, bool info_only);

  // Retrieves from the DB the server info for this store based our
  // identifying domain/name/required_cookie tuple.
  static bool FindServer(const SecurityOrigin &security_origin,
                         const char16 *name,
                         const char16 *required_cookie,
                         WebCacheDB::ServerInfo *server);

  // The rowid of the version record in the DB associated with this
  // ResourceStore. Each ResourceStore has exactly one related
  // version record. The schema requires that Servers contain Versions
  // contain Entries in order to support the other class of Server, the
  // ManagedResourceStore. To satisfy the schema, this class manages
  // the lifecyle of this record in the version table. When the Server
  // record is inserted and delete, the Version record goes in lock step.
  int64 version_id_;
};

#endif  // GEARS_LOCALSERVER_COMMON_RESOURCE_STORE_H__
