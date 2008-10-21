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

#ifndef GEARS_LOCALSERVER_COMMON_LOCALSERVER_H__
#define GEARS_LOCALSERVER_COMMON_LOCALSERVER_H__

#include "gears/base/common/common.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/http_cookies.h"
#include "gears/localserver/common/localserver_db.h"


//------------------------------------------------------------------------------
// A LocalServer represents a set of entries in the WebCacheDB and allows
// the set to be managed as a group. The identifying properties of a 
// LocalServer are its domain, name, required_cookie, and server_type.
// The webcache system supports two types of local servers, ResourceStores
// and ManagedResourceStores.
//
// See web_capture_store.h, captured_application.h
//------------------------------------------------------------------------------
class LocalServer {
 public:
  // Returns if the database has a response for the url at this time
  static bool CanServeLocally(const char16 *url, BrowsingContext *context);

  // Removes the server from the DB, deleting all related rows from
  // the Servers, Versions, Entries, and Payloads tables
  bool Remove();

  // Returns true if our server_id_ still exists in the DB.
  bool StillExistsInDB() {
    assert(is_initialized_);
    // This is an optimization to avoid hitting the database.
    // Once a store is removed, it can never come back with the same server id,
    // so we cache this value and if it goes to false we don't hit the db on
    // subsequent calls.
    if (store_might_exist_) {
      WebCacheDB::ServerInfo server;
      store_might_exist_ = GetServer(&server);
    }
    return store_might_exist_;
  }

  // Returns a copy of the SecurityOrigin that was passed into Init
  const SecurityOrigin &GetSecurityOrigin() const {
    assert(is_initialized_);
    return security_origin_;
  }

  // Returns the name passed into Init
  const char16* GetName() const {
    assert(is_initialized_);
    return name_.c_str();
  }

  // Returns the required_cookie passed into Init
  const char16* GetRequiredCookie() const  {
    assert(is_initialized_);
    return required_cookie_.c_str();
  }

  // Returns true if local serving of entries in this LocalServer is enabled
  bool IsEnabled();

  // Enable or disable local serving of entries in this LocalServer. Returns
  // true if successfully modifies the setting.
  bool SetEnabled(bool enabled);

 protected:
  friend class UpdateTask;

  // Returns true if the server exists in the DB
  static bool ExistsInDB(const SecurityOrigin &security_origin,
                         const char16 *name,
                         const char16 *required_cookie,
                         WebCacheDB::ServerType type,
                         int64 *server_id_out);

  // Retrieves from the DB the server info for this application based our
  // identifying domain/name/required_cookie/type.
  static bool FindServer(const SecurityOrigin &security_origin,
                         const char16 *name,
                         const char16 *required_cookie,
                         WebCacheDB::ServerType type,
                         WebCacheDB::ServerInfo *server);

  // Constructor
  LocalServer(WebCacheDB::ServerType type);

  // Initializes an instance and inserts a row in the Servers table of
  // of DB if one is not already present. Pre: this instance must be
  // uninitialized.   Upon return, sucessful and otherwise, the 
  // is_initialized_ data member will be set to false.
  // Derived classes must set to true.
  virtual bool CreateOrOpen(const SecurityOrigin &security_origin,
                            const char16 *name,
                            const char16 *required_cookie);

  // Initializes an instance given it's server_id. Pre: this instance must
  // be uninitialized and the server_id must exist in the DB. Upon return,
  // sucessful and otherwise, the  is_initialized_ data member will be set.
  // to false. Derived classes must set to true.
  virtual bool Open(int64 server_id);

  // Initialzes one instance from another which must be initialized and of
  // the same server_type. Pre: this instance must be uninitialized.  Upon
  // return, sucessful and otherwise, the is_initialized_ data member will
  // be set to false. Derived classes must set to true when.
  bool Clone(LocalServer *local_server);

  // Retrieves from the DB the server info for this server base on server_id
  bool GetServer(WebCacheDB::ServerInfo *server);

  // Retrieves from the DB the version info for the desired ready state
  bool GetVersion(WebCacheDB::VersionReadyState state,
                  WebCacheDB::VersionInfo *version);

  // Retrieves from the DB the version info for the desired version string
  bool GetVersion(const char16* version_string,
                  WebCacheDB::VersionInfo *version);

  bool is_initialized_;
  SecurityOrigin security_origin_;
  std::string16 name_;
  std::string16 required_cookie_;
  WebCacheDB::ServerType server_type_;
  int64 server_id_;
  bool store_might_exist_;
};

#endif  // GEARS_LOCALSERVER_COMMON_LOCALSERVER_H__
