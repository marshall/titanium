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

#ifndef GEARS_LOCALSERVER_COMMON_BLOB_STORE_H__
#define GEARS_LOCALSERVER_COMMON_BLOB_STORE_H__

#include "gears/base/common/common.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/localserver_db.h"

//------------------------------------------------------------------------------
// This class is an implementation detail of WebCacheDB.
//
// A WebCacheBlobStore is used by a WebCacheDB to store the response
// bodies in the SQLite database as blobs. At this time, all browsers,
// except IE, use this class. IE stores body data as flat files in the
// file system using class WebCacheFileStore in place of this class.
//
// The only intended client is the WebCacheDB class, so this entire class
// is marked protected and the WebCacheDB is allowed access as a friend.
//------------------------------------------------------------------------------
class WebCacheBlobStore {
 protected:
  // The following API is intended for use by WebCacheDB
  friend class WebCacheDB;

  // Constructs an instance
  WebCacheBlobStore() : db_(NULL) {}

  // Initializes the file store, returns false if initialization fails
  virtual bool Init(WebCacheDB *db);

  // Inserts a body into the store
  virtual bool InsertBody(int64 server_id,
                          const char16 *url,
                          WebCacheDB::PayloadInfo *payload);

  // Reads a body from the store
  virtual bool ReadBody(WebCacheDB::PayloadInfo *payload, bool info_only);

  // Deletes a body from the store
  virtual bool DeleteBody(int64 payload_id);
  
  // Deletes all unreferenced bodies from the store
  virtual bool DeleteUnreferencedBodies();

  // The WebCacheDB that owns us
  WebCacheDB *db_;

  // This class is not thread safe
  DECL_SINGLE_THREAD

 private:
  DISALLOW_EVIL_CONSTRUCTORS(WebCacheBlobStore);
};

#endif // GEARS_LOCALSERVER_COMMON_BLOB_STORE_H__
