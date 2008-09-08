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

#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/blob_store.h"
#include "gears/localserver/common/localserver_db.h"


//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
bool WebCacheBlobStore::Init(WebCacheDB *db) {
  db_ = db;
  return true;
}

//------------------------------------------------------------------------------
// InsertBody
//------------------------------------------------------------------------------
bool WebCacheBlobStore::InsertBody(int64 server_id,
                                   const char16 *url,
                                   WebCacheDB::PayloadInfo *payload) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  assert(payload->id != WebCacheDB::kUnknownID);

  // We only store the bodies of successful responses
  if (payload->status_code != HttpConstants::HTTP_OK) {
    return true;
  }

  // Insert a row in the ResponseBodies table containing the data. Note
  // that BodyID is the same as PayloadID in the Payloads table.
  const char16* sql = STRING16(L"INSERT INTO ResponseBodies"
                               L" (BodyID, Data)"
                               L" VALUES (?, ?)");
  SQLStatement stmt;
  int rv = stmt.prepare16(db_->GetSQLDatabase(), sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.InsertBody failed\n"));
    return false;
  }
  int param = -1;
  rv = stmt.bind_int64(++param, payload->id);
  rv |= stmt.bind_blob(++param, payload->data.get());
  if (rv != SQLITE_OK) {
    return false;
  }

  return (stmt.step() == SQLITE_DONE);
}

//------------------------------------------------------------------------------
// ReadBody
//------------------------------------------------------------------------------
bool WebCacheBlobStore::ReadBody(WebCacheDB::PayloadInfo *payload,
                                 bool info_only) {
  ASSERT_SINGLE_THREAD();
  assert(db_);

  if (!info_only && (payload->status_code == HttpConstants::HTTP_OK)) {
    const char16* sql = STRING16(L"SELECT Data FROM ResponseBodies "
                                 L"WHERE BodyID=?");
    SQLStatement stmt;
    if ((stmt.prepare16(db_->GetSQLDatabase(), sql) != SQLITE_OK) ||
        (stmt.bind_int64(0, payload->id) != SQLITE_OK) ||
        (stmt.step() != SQLITE_ROW)) {
      LOG(("WebCacheBlobStore.ReadBody failed\n"));
      return false;
    }

    if (stmt.column_type(0) == SQLITE_BLOB) {
      payload->data.reset(new std::vector<uint8>);
      if (!stmt.column_blob_as_vector(0, payload->data.get())) {
        LOG(("WebCacheBlobStore.ReadBody failed\n"));
        return false;
      }
    } else {
      payload->data.reset(NULL);
    }
  } else {
    payload->data.reset(NULL);
  }

  return true;
}

//------------------------------------------------------------------------------
// DeleteBody
//------------------------------------------------------------------------------
bool WebCacheBlobStore::DeleteBody(int64 payload_id) {
  ASSERT_SINGLE_THREAD();
  assert(db_);

  // Delete the row from the response bodies table
  const char16* sql = STRING16(L"DELETE FROM ResponseBodies WHERE BodyID=?");
  SQLStatement stmt;
  int rv = stmt.prepare16(db_->GetSQLDatabase(), sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.DeleteBody failed\n"));
    return false;
  }
  rv = stmt.bind_int64(0, payload_id);
  if (rv != SQLITE_OK) {
    return false;
  }
  return (stmt.step() == SQLITE_DONE);
}
  
//------------------------------------------------------------------------------
// DeleteUnreferencedBodies
//------------------------------------------------------------------------------
bool WebCacheBlobStore::DeleteUnreferencedBodies() {
  ASSERT_SINGLE_THREAD();
  assert(db_);

  // Delete the rows
  const char16* sql = STRING16(
                          L"DELETE FROM ResponseBodies WHERE BodyID NOT IN "
                          L"(SELECT DISTINCT PayloadID FROM Payloads)");
  SQLStatement delete_stmt;
  int rv = delete_stmt.prepare16(db_->GetSQLDatabase(), sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.DeleteUnreferencedBodies failed\n"));
    return false;
  }
  return (delete_stmt.step() == SQLITE_DONE);
}

