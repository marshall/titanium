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

#include "gears/base/common/name_value_table.h"
#include "gears/base/common/sqlite_wrapper.h"

static const std::string16 kFoo(STRING16(L"foo"));
static const std::string16 kBar(STRING16(L"bar"));


bool TestNameValueTableAll(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestNameValueTable - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestNameValueTable - failed. "); \
    return false; \
  } \
}

  // Create a test database to test in
  SQLDatabase db;
  TEST_ASSERT(db.Open(STRING16(L"TestNameValueTable.db")));
  TEST_ASSERT(db.DropAllObjects());

  NameValueTable nvt(&db, STRING16(L"TestNameValueTable"));
  TEST_ASSERT(nvt.MaybeCreateTable());

  // set and get an int value
  TEST_ASSERT(nvt.SetInt(kFoo.c_str(), 42));
  int int_result;
  TEST_ASSERT(nvt.GetInt(kFoo.c_str(), &int_result));
  TEST_ASSERT(42 == int_result);

  // get a non-existant int value
  TEST_ASSERT(!nvt.GetInt(kBar.c_str(), &int_result));

  // set and get an string value
  TEST_ASSERT(nvt.SetString(kFoo.c_str(), kBar.c_str()));
  std::string16 str_result;
  TEST_ASSERT(nvt.GetString(kFoo.c_str(), &str_result));
  TEST_ASSERT(kBar == str_result);

  // get a non-existant string value
  TEST_ASSERT(!nvt.GetString(kBar.c_str(), &str_result));

  LOG(("TestNameValueTableAll - passed\n"));
  return true;
}
