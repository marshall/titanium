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

#ifndef GEARS_DATABASE2_RESULT_SET2_H__
#define GEARS_DATABASE2_RESULT_SET2_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string16.h"
#include "gears/database2/connection.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// interface SQLResultSet
class Database2ResultSet : public ModuleImplBaseClass,
                           public Database2RowHandlerInterface,
                           public JsEventHandlerInterface {
 public:
  Database2ResultSet() : ModuleImplBaseClass("Database2ResultSet") {}
  ~Database2ResultSet() {}
  // Creates an instance, returns true if successful.
  static bool Create(ModuleEnvironment *module_environment,
                     JsCallContext *context,
                     scoped_refptr<Database2ResultSet> *instance);

  // OUT: int
  void GetInsertId(JsCallContext *context);
  // OUT: int
  void GetRowsAffected(JsCallContext *context);
  // We return a JS array full of result objects.
  // OUT: JsArray
  void GetRows(JsCallContext *context);

    // Database2RowHandlerInterface
  virtual void Init(int column_count, std::string16 *column_names);
  virtual void HandleNewRow();
  virtual bool HandleColumnInt(int index, int value);
  virtual bool HandleColumnDouble(int index, double value);
  virtual bool HandleColumnString(int index, const std::string16 &value);
  virtual bool HandleColumnNull(int index);
  virtual void HandleStats(int64 last_insert_rowid, int rows_affected);


  // Handles the unload event, cleans up the callbacks to prevent crashes 
  // in Firefox.
  virtual void HandleEvent(JsEventType event_type);

 private:
  int rows_affected_;
  int64 last_insert_rowid_;
  int column_count_;
  scoped_array<std::string16> column_names_;
  scoped_ptr<JsArray> rows_;

  // keeps track of the current number of rows
  int row_count_;
  // current row cursor, points to the current row object in the rows_ array
  scoped_ptr<JsObject> current_row_;

  // monitors JSEVENT_UNLOAD
  scoped_ptr<JsEventMonitor> unload_monitor_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2ResultSet);
};

#endif // GEARS_DATABASE2_RESULT_SET2_H__
