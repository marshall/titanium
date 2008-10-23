// Copyright 2008, Google Inc.
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

#include "gears/geolocation/geolocation_db.h"

static const char16 *kDatabaseName = STRING16(L"geolocation.db");
static const char16 *kVersionTableName = STRING16(L"VersionInfo");
static const char16 *kVersionKey = STRING16(L"Version");
static const int kCurrentVersion = 1;

const ThreadLocals::Slot GeolocationDB::kThreadLocalKey =
    ThreadLocals::Alloc();


GeolocationDB::GeolocationDB()
    : version_table_(&db_, kVersionTableName),
      position_table_(&db_) {
}

//static
GeolocationDB *GeolocationDB::GetDB() {
  if (ThreadLocals::HasValue(kThreadLocalKey)) {
    return reinterpret_cast<GeolocationDB*>(
        ThreadLocals::GetValue(kThreadLocalKey));
  }

  GeolocationDB *db = new GeolocationDB();

  // If we can't initialize, we store NULL in the map so that we don't keep
  // trying to Init() over and over.
  if (!db->Init()) {
    delete db;
    db = NULL;
  }

  ThreadLocals::SetValue(kThreadLocalKey, db, &DestroyDB);
  return db;
}

bool GeolocationDB::StorePosition(const std::string16 &name,
                                  const Position &position) {
  return position_table_.SetPosition(name, position);
}

bool GeolocationDB::RetrievePosition(const std::string16 &name,
                                     Position *position) {
  return position_table_.GetPosition(name, position);
}

bool GeolocationDB::Create() {
  ASSERT_SINGLE_THREAD();

  SQLTransaction transaction(&db_, "GeolocationDB::Create");
  if (!transaction.Begin()) {
    return false;
  }

  if (!db_.DropAllObjects()) {
    return false;
  }

  if (!version_table_.MaybeCreateTable() || !position_table_.Create()) {
    return false;
  }

  // set the current version
  if (!version_table_.SetInt(kVersionKey, kCurrentVersion)) {
    return false;
  }

  return transaction.Commit();
}

bool GeolocationDB::Init() {
  // Initialize the database and tables
  if (!db_.Open(kDatabaseName)) {
    return false;
  }

  // Examine the contents of the database and determine if we have to
  // instantiate or updgrade the schema.
  int version = 0;
  version_table_.GetInt(kVersionKey, &version);

  // If it's the version we're expecting, great.
  if (version == kCurrentVersion) {
    return true;
  }

  // Doing this in a transaction effectively locks the database file and
  // ensures that this is synchronized across all threads and processes.
  SQLTransaction transaction(&db_, "GeolocationDB::Init");
  if (!transaction.Begin()) {
    return false;
  }

  // Fetch the version again in case someone else beat us to the
  // upgrade.
  version_table_.GetInt(kVersionKey, &version);
  if (version == kCurrentVersion) {
    return true;
  }

  if (0 == version) {
    // No database in place, create it.
    if (!Create()) {
      return false;
    }
  } else {
    // If the database schema is modified in the future, upgrade the
    // database here. For now, this should never happen.
    assert(false);
    return false;
  }

  // Double-check that we ended up with the right version.
  version_table_.GetInt(kVersionKey, &version);
  if (version != kCurrentVersion) {
    return false;
  }

  return transaction.Commit();
}

// static
void GeolocationDB::DestroyDB(void *context) {
  GeolocationDB *db = reinterpret_cast<GeolocationDB*>(context);
  if (db) {
    delete db;
  }
}
