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

#include "gears/database2/manager.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/database2/database2.h"
#include "gears/database2/database2_common.h"
#include "gears/database2/database2_metadata.h"

DECLARE_GEARS_WRAPPER(GearsDatabase2Manager);

template<>
void Dispatcher<GearsDatabase2Manager>::Init() {
  RegisterMethod("openDatabase", &GearsDatabase2Manager::OpenDatabase);
}

void GearsDatabase2Manager::OpenDatabase(JsCallContext *context) {
  std::string16 name;
  std::string16 version;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &name },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &version }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  // Find the filename and other metadata for this database.
  PermissionsDB *permissions_db = PermissionsDB::GetDB();
  assert(permissions_db);

  Database2Metadata *database2_metadata = permissions_db->database2_metadata();
  assert(database2_metadata);

  std::string16 filename;
  std::string16 found_version;
  int version_cookie = 0;
  std::string16 error;
  if (!database2_metadata->GetDatabaseInfo(EnvPageSecurityOrigin(), name,
                                           version, &filename, &found_version,
                                           &version_cookie, &error)) {
    context->SetException(error);
    return;
  }

  // Check for version mismatch.
  if (filename.empty()) {
    // TODO(aa): Raise INVALID_STATE_ERR exception per spec. Not sure what that
    // means exactly. See also issue 100.
    context->SetException(kInvalidStateError);
    return;
  }

  // create the Database2 instance
  scoped_ptr<Database2Connection> connection(
      new Database2Connection(EnvPageSecurityOrigin(), filename, version_cookie,
                              database2_metadata));

  scoped_refptr<GearsDatabase2> database;
  if (!GearsDatabase2::Create(module_environment_.get(), context, name,
                              found_version, connection.get(), &database)) {
    return;
  }

  // transfer ownership to database2
  connection.release();
  context->SetReturnValue(JSPARAM_MODULE, database.get());
}
