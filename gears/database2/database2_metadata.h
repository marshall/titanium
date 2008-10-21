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

#ifndef GEARS_DATABASE2_DATABASE2_METADATA_H__
#define GEARS_DATABASE2_DATABASE2_METADATA_H__

#include "gears/base/common/security_model.h"
#include "gears/base/common/sqlite_wrapper.h"

class PermissionsDB;

// Stores metadata about the database2 instances in use in Gears.
class Database2Metadata {
 public:

  // Create the table holding the metadata
  bool MaybeCreateTableLatestVersion();

  // Version numbers correspond to the version number of the entire permissions
  // database.
  bool CreateTableVersion8();

  // Gets the filename and other metadata for a Database2 instance. If there is
  // no active (non-deleted, non-corrupt) matching database, a new filename is
  // generated and stored, and it is returned. Both name and version may be the
  // empty string. If version is empty, any active version will be returned. If
  // version is specified and does not match the current active version, empty
  // filename and error parameters are returned.
  bool GetDatabaseInfo(const SecurityOrigin &origin,
                       const std::string16 &database_name,
                       const std::string16 &version,
                       std::string16 *filename,
                       std::string16 *found_version,
                       int *version_cookie,
                       std::string16 *error);

  // Mark the given database corrupt so that future calls to GetDatabaseInfo()
  // will no longer return it.
  bool MarkDatabaseCorrupt(const SecurityOrigin &origin,
                           const std::string16 &database_name);

  // TODO(aa): Add DeleteDatabase(origin, database_name)?

 private:
  // For access to the private constructor.
  friend class PermissionsDB;

  // Use PermissionsDB::GetDatabase2Metadata() instead.
  Database2Metadata(SQLDatabase *db);

  // A pointer to the SQLDatabase our table lives in.
  SQLDatabase *db_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2Metadata);
};

#endif  // GEARS_DATABASE2_DATABASE2_METADATA_H__
