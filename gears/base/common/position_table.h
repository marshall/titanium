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

#ifndef GEARS_BASE_COMMON_POSITION_TABLE_H__
#define GEARS_BASE_COMMON_POSITION_TABLE_H__

#include "gears/base/common/sqlite_wrapper.h"
#include "gears/geolocation/geolocation.h"

// This class provides an API to manage storing a Position object.
class PositionTable {
 public:
  PositionTable(SQLDatabase *db);

  // Creates the latest version of the table. Should only be called if the
  // table does not already exist.
  bool Create();

  // Add (or overwrite) a position with the given name.
  bool SetPosition(const std::string16 &name, const Position &position);

  // Get the position with the given name.
  bool GetPosition(const std::string16 &name, Position *position);

  // Delete the position with the given name.
  bool DeletePosition(const std::string16 &name);

 private:
  // Creates version 1 of the table. This assumes that the table does not
  // already exist.
  bool CreateVersion1();

  // A pointer to the SQLDatabase our table lives in.
  SQLDatabase *db_;

  DISALLOW_EVIL_CONSTRUCTORS(PositionTable);
};

#endif  // GEARS_BASE_COMMON_POSITION_TABLE_H__
