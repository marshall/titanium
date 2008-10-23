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

#include "gears/database2/result_set2.h"

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/module_wrapper.h"

DECLARE_GEARS_WRAPPER(Database2ResultSet);

template<>
void Dispatcher<Database2ResultSet>::Init() {
  RegisterProperty("insertId", &Database2ResultSet::GetInsertId, NULL);
  RegisterProperty("rowsAffected", &Database2ResultSet::GetRowsAffected, NULL);
  RegisterProperty("rows", &Database2ResultSet::GetRows, NULL);
}


// static
bool Database2ResultSet::Create(ModuleEnvironment *module_environment,
                                JsCallContext *context,
                                scoped_refptr<Database2ResultSet> *instance) {
  assert(instance);
  if (!CreateModule<Database2ResultSet>(module_environment, context,
                                        instance)) {
    return false;
  }

  Database2ResultSet *result_set = instance->get();
  // register unload handler
  result_set->unload_monitor_.reset(new JsEventMonitor(
      module_environment->js_runner_, JSEVENT_UNLOAD, result_set));
  return true;
}

void Database2ResultSet::Init(int column_count, std::string16 *column_names) {
  // We don't support re-initialization, thus the rows array must be empty.
  assert(!rows_.get());
  column_count_ = column_count;
  column_names_.reset(column_names);
  row_count_ = 0;
  rows_.reset(GetJsRunner()->NewArray());
}

void Database2ResultSet::HandleNewRow() {
  // Array must be initialized when this method is called.
  assert(rows_.get());
  current_row_.reset(GetJsRunner()->NewObject(false));
  rows_->SetElementObject(row_count_, current_row_.get());
  ++row_count_;
}

bool Database2ResultSet::HandleColumnInt(int index, int value) {
  assert(index >= 0 && index < column_count_);
  assert(current_row_.get());
  return current_row_->SetPropertyInt(column_names_[index], value);
}

bool Database2ResultSet::HandleColumnDouble(int index, double value) {
  assert(index >= 0 && index < column_count_);
  assert(current_row_.get());
  return current_row_->SetPropertyDouble(column_names_[index], value);
}

bool Database2ResultSet::HandleColumnString(int index,
                                            const std::string16 &value) {
  assert(index >= 0 && index < column_count_);
  assert(current_row_.get());
  return current_row_->SetPropertyString(column_names_[index], value);
}

bool Database2ResultSet::HandleColumnNull(int index) {
  assert(index >= 0 && index < column_count_);
  assert(current_row_.get());
  JsScopedToken value;
  // TODO(dimitri.glazkov): Add JsArray::SetElementNull and refactor.
  NullToJsToken(GetJsRunner()->GetContext(), &value);
  return current_row_->SetProperty(column_names_[index], value);
}

void Database2ResultSet::HandleStats(int64 last_insert_rowid,
                                     int rows_affected) {
  last_insert_rowid_ = last_insert_rowid;
  rows_affected_ = rows_affected;
}

void Database2ResultSet::GetInsertId(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT64, &last_insert_rowid_);
}

void Database2ResultSet::GetRowsAffected(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_INT, &rows_affected_);
}

void Database2ResultSet::GetRows(JsCallContext *context) {
  // This method should not be called until the result set is populated.
  assert(rows_.get());
  context->SetReturnValue(JSPARAM_ARRAY, rows_.get());
}

void Database2ResultSet::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  // Clear rows and current row, because in FF the JS runtime may go away
  // without finishing garbage-collection.
  rows_.reset();
  current_row_.reset();

  unload_monitor_.reset();
}
