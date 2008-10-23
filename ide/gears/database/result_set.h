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

#ifndef GEARS_DATABASE_RESULT_SET_H__
#define GEARS_DATABASE_RESULT_SET_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"

struct sqlite3_stmt;
class GearsDatabase;

class GearsResultSet : public ModuleImplBaseClass {
 public:
  static const std::string kModuleName;

  GearsResultSet();
  virtual ~GearsResultSet();

  // IN: int index
  // OUT: variant
  void Field(JsCallContext *context);

  // IN: string field_name
  // OUT: variant
  void FieldByName(JsCallContext *context);

  // IN: int index
  // OUT: string
  void FieldName(JsCallContext *context);

  // IN: -
  // OUT: int
  void FieldCount(JsCallContext *context);

  // IN: -
  // OUT: -
  void Close(JsCallContext *context);

  // IN: -
  // OUT: -
  void Next(JsCallContext *context);

  // IN: -
  // OUT: bool
  void IsValidRow(JsCallContext *context);

 private:
  friend class GearsDatabase;

  // Helper called by GearsDatabase.execute to initialize the result set
  bool InitializeResultSet(sqlite3_stmt *statement,
                           GearsDatabase *db,
                           std::string16 *error_message);

  // Called by GearsDatabase to let us know the page is going away, and so
  // our database_ pointer won't be valid for long.
  void PageUnloading();

  // Helper shared by Field() and FieldByName()
  void FieldImpl(JsCallContext *context, int index);

  // Helper shared by Next() and SetStatement()
  bool NextImpl(std::string16 *error_message);
  bool Finalize();

  scoped_refptr<GearsDatabase> database_;
  sqlite3_stmt *statement_;
  bool is_valid_row_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsResultSet);
};

#endif // GEARS_DATABASE_RESULT_SET_H__
