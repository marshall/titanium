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

#ifndef GEARS_GEOLOCATION_GEOLOCATION_DB_H__
#define GEARS_GEOLOCATION_GEOLOCATION_DB_H__

#include "gears/base/common/name_value_table.h"
#include "gears/base/common/position_table.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/thread_locals.h"
#include "gears/geolocation/geolocation.h"

class GeolocationDB {
 public:
  // Gets a thread-specific GeolocationDB instance.
  static GeolocationDB *GetDB();

  bool StorePosition(const std::string16 &name, const Position &position);
  bool RetrievePosition(const std::string16 &name, Position *position);

  // The key used to cache instances of GeolocationDB in ThreadLocals.
  static const ThreadLocals::Slot kThreadLocalKey;

 private:
  // Private constructor, callers must use GetDB().
  GeolocationDB();

  // Creates the database at the latest version.
  bool Create();

  // Initializes the database. Must be called before other methods.
  bool Init();

  // Destructor function called by ThreadLocals to dispose of a thread-specific
  // DB instance when a thread dies.
  static void DestroyDB(void *context);

  // Database we use to store geolocation information.
  SQLDatabase db_;

  // Table used to store version metadata.
  NameValueTable version_table_;

  // Table used to store positions.
  PositionTable position_table_;

  DISALLOW_EVIL_CONSTRUCTORS(GeolocationDB);
  DECL_SINGLE_THREAD
};

#endif  // GEARS_GEOLOCATION_GEOLOCATION_DB_H__
