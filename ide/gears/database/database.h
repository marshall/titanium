// Copyright 2007, Google Inc.
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

#ifndef GEARS_DATABASE_DATABASE_H__
#define GEARS_DATABASE_DATABASE_H__

#include <set>
#include "third_party/scoped_ptr/scoped_ptr.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/stopwatch.h"

struct sqlite3;
struct sqlite3_stmt;
class GearsResultSet;

class GearsDatabase
    : public ModuleImplBaseClass,
      public JsEventHandlerInterface {
 public:
  static const std::string kModuleName;

  GearsDatabase();
  virtual ~GearsDatabase();

  // IN: optional string database_name
  // OUT: -
  void Open(JsCallContext *context);

  // IN: string expression, optional array args
  // OUT: GearsResultSet
  void Execute(JsCallContext *context);

  // IN: -
  // OUT: -
  void Close(JsCallContext *context);

  // IN: -
  // OUT: int
  void GetLastInsertRowId(JsCallContext *context);

  // IN: -
  // OUT: int
  void GetRowsAffected(JsCallContext *context);

  void HandleEvent(JsEventType event_type);

// Right now this is just used for testing perf. If we ever want to make it a
// real feature of Gears, then it will need to keep separate stopwatches for
// each database file, not a single stopwatch for the entire process as it does
// now.
#ifdef DEBUG
  // IN: -
  // OUT: int
  void GetExecuteMsec(JsCallContext *context);
  static Stopwatch g_stopwatch_;
#endif // DEBUG

  friend class GearsResultSet;

 private:
  void AddResultSet(GearsResultSet *rs);
  void RemoveResultSet(GearsResultSet *rs);
  bool CloseInternal();
  bool BindArgsToStatement(JsCallContext *context,
                           const JsArray *arg_array, sqlite3_stmt *stmt);

  sqlite3 *db_;
  std::set<GearsResultSet *> result_sets_;
  scoped_ptr<JsEventMonitor> unload_monitor_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsDatabase);
};

#endif // GEARS_DATABASE_DATABASE_H__
