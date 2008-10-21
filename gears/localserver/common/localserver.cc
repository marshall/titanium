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

#include "gears/localserver/common/localserver.h"


//------------------------------------------------------------------------------
// CanServeLocally
//------------------------------------------------------------------------------
// static
bool LocalServer::CanServeLocally(const char16 *url, BrowsingContext *context) {
  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }
  return db->CanService(url, context);
}

//------------------------------------------------------------------------------
// ExistsInDB
//------------------------------------------------------------------------------
// static
bool LocalServer::ExistsInDB(const SecurityOrigin &security_origin,
                             const char16 *name,
                             const char16 *required_cookie,
                             WebCacheDB::ServerType type,
                             int64 *server_id_out) {
  assert(name && required_cookie && server_id_out);
  WebCacheDB::ServerInfo server;
  if (!FindServer(security_origin, name, required_cookie, type, &server))
    return false;
  *server_id_out = server.id;
  return true;
}

//------------------------------------------------------------------------------
// FindServer
//------------------------------------------------------------------------------
// static
bool LocalServer::FindServer(const SecurityOrigin &security_origin,
                             const char16 *name,
                             const char16 *required_cookie,
                             WebCacheDB::ServerType type,
                             WebCacheDB::ServerInfo *server) {
  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }
  return db->FindServer(security_origin, name, required_cookie, type, server);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LocalServer::LocalServer(WebCacheDB::ServerType type) :
    is_initialized_(false),
    server_type_(type),
    server_id_(WebCacheDB::kUnknownID),
    store_might_exist_(true) {
}

//------------------------------------------------------------------------------
// CreateOrOpen
//------------------------------------------------------------------------------
bool LocalServer::CreateOrOpen(const SecurityOrigin &security_origin,
                               const char16 *name,
                               const char16 *required_cookie) {
  assert(name);
  assert(required_cookie);

  if (is_initialized_) {
    assert(!is_initialized_);
    return false;
  }

  security_origin_ = security_origin;
  name_ = name;
  required_cookie_ = required_cookie;

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  // We start a transaction here to guard against two threads calling
  // InsertServer with the same ServerInfo.
  SQLTransaction transaction(db->GetSQLDatabase(), "LocalServer::CreateOrOpen");
  if (!transaction.Begin()) {
    return false;
  }

  WebCacheDB::ServerInfo server;
  if (!FindServer(security_origin, name, required_cookie,
                  server_type_, &server)) {
    server.server_type = server_type_;
    server.security_origin_url = security_origin_.url();
    server.name = name_;
    server.required_cookie = required_cookie_;
    server.last_update_check_time = 0;
    if (!db->InsertServer(&server)) {
      return false;
    }
  }

  server_id_ = server.id;
  return transaction.Commit();
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
bool LocalServer::Open(int64 server_id) {
  assert(server_id != WebCacheDB::kUnknownID);
  if (is_initialized_) {
    assert(!is_initialized_);
    return false;
  }
  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }
  WebCacheDB::ServerInfo server;
  if (!db->FindServer(server_id, &server)) {
    return false;
  }
  if (server.server_type != server_type_) {
    assert(false);
    return false;
  }

  server_id_ = server.id;
  name_ = server.name;
  required_cookie_ = server.required_cookie;
  return security_origin_.InitFromUrl(server.security_origin_url.c_str());
}

//------------------------------------------------------------------------------
// Clone
//------------------------------------------------------------------------------
bool LocalServer::Clone(LocalServer *local_server) {
  if (!local_server) {
    assert(local_server);
    return false;
  }
  if (!local_server->is_initialized_) {
    assert(local_server->is_initialized_);
    return false;
  }
  if (local_server->server_type_ != server_type_) {
    assert(local_server->server_type_ != server_type_);
    return false;
  }

  security_origin_ = local_server->security_origin_;
  name_ = local_server->name_;
  required_cookie_ = local_server->required_cookie_;
  server_id_ = local_server->server_id_;
  return true;
}

//------------------------------------------------------------------------------
// Returns true if local serving of entries in this LocalServer is enabled
//------------------------------------------------------------------------------
bool LocalServer::IsEnabled() {
  WebCacheDB::ServerInfo server;
  if (!GetServer(&server)) {
    return false;
  }

  return server.enabled;
}

//------------------------------------------------------------------------------
// Enable or disable local serving of entries in this LocalServer
//------------------------------------------------------------------------------
bool LocalServer::SetEnabled(bool enabled) {
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  return db->UpdateServer(server_id_, enabled);
}

//------------------------------------------------------------------------------
// Remove
//------------------------------------------------------------------------------
bool LocalServer::Remove() {
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  return db->DeleteServer(server_id_);
}

//------------------------------------------------------------------------------
// GetServer
//------------------------------------------------------------------------------
bool LocalServer::GetServer(WebCacheDB::ServerInfo *server) {
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  return db->FindServer(server_id_, server);
}

//------------------------------------------------------------------------------
// GetVersion
//------------------------------------------------------------------------------
bool LocalServer::GetVersion(WebCacheDB::VersionReadyState state,
                             WebCacheDB::VersionInfo *version) {
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  return db->FindVersion(server_id_, state, version);
}

//------------------------------------------------------------------------------
// GetVersion
//------------------------------------------------------------------------------
bool LocalServer::GetVersion(const char16* version_string,
                             WebCacheDB::VersionInfo *version) {
  if (!is_initialized_) {
    assert(is_initialized_);
    return false;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  return db->FindVersion(server_id_, version_string, version);
}

