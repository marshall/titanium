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

#include "gears/database/database_utils_test.h"

#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/scoped_token.h"
#include "gears/database/database_utils.h"

// Hack together a scoped sqlite3 * so that TEST_ASSERT() failures
// dont't leave the database wedged.
typedef DECLARE_SCOPED_TRAITS(sqlite3*, sqlite3_close, NULL)
    sqlite3Traits;
typedef scoped_token<sqlite3*, sqlite3Traits> scoped_sqlite3_handle;

bool TestDatabaseUtilsAll(std::string16 *error) {
// TODO(aa): Refactor into a common location for all the internal tests.
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestDatabaseUtilsAll - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestDatabaseUtilsAll - failed. "); \
    return false; \
  } \
}

  // TODO(shess): I would like it if this code could clear out the
  // existing PermissionsDB database name mappings, both to prevent
  // growth of cruft in PermissionsDB, and to make sure we're testing
  // cases such as no mappings.  For now, this code manually deletes
  // the database files, because that has the biggest potential for
  // issues (too many files in a directory).  That will work as long
  // as assertions aren't failing...

  PermissionsDB *permissions = PermissionsDB::GetDB();
  TEST_ASSERT(permissions);

  // Get us an origin to work with.
  SecurityOrigin foo;
  foo.InitFromUrl(STRING16(L"http://unittest.foo.example.com"));
  permissions->SetPermission(foo,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);

  // Delete any existing database file.
  const char16 *kFooDatabaseName = STRING16(L"poison_test");
  std::string16 basename;
  TEST_ASSERT(permissions->GetDatabaseBasename(foo, kFooDatabaseName,
                                               &basename));

  std::string16 data_dir;
  TEST_ASSERT(GetDataDirectory(foo, &data_dir));

  std::string16 filename(data_dir);
  filename += kPathSeparator;
  filename += basename;
  File::Delete(filename.c_str());

  // Open a couple handles to the database.
  scoped_sqlite3_handle db, db2;
  TEST_ASSERT(OpenSqliteDatabase(kFooDatabaseName, foo, as_out_parameter(db)));
  TEST_ASSERT(OpenSqliteDatabase(kFooDatabaseName, foo, as_out_parameter(db2)));

  // Test that the database works.
  const char *kInsertSql("INSERT INTO t VALUES ('x')");
  TEST_ASSERT(SQLITE_OK == sqlite3_exec(db.get(), "CREATE TABLE t (c TEXT)",
                                        NULL, NULL, NULL));
  TEST_ASSERT(SQLITE_OK == sqlite3_exec(db.get(), kInsertSql,
                                        NULL, NULL, NULL));

  // This should have the side effect of poisoning the database.
  TEST_ASSERT(SQLITE_CORRUPT == SqlitePoisonIfCorrupt(db.get(),
                                                      SQLITE_CORRUPT));

  // Should map SQLITE_NOTADB to SQLITE_CORRUPT.
  TEST_ASSERT(SQLITE_CORRUPT == SqlitePoisonIfCorrupt(db.get(),
                                                      SQLITE_NOTADB));

  // Should fail now.
  TEST_ASSERT(SQLITE_NOTADB == sqlite3_exec(db.get(), kInsertSql,
                                            NULL, NULL, NULL));
  TEST_ASSERT(SQLITE_OK == sqlite3_close(db.release()));

  // Should also fail on the other handle.
  // TODO(shess): This is a problem.  I get SQLITE_ERROR, which will
  // NOT provide the desired SQLITE_CORRUPT signal to close the
  // database and re-open.
  TEST_ASSERT(SQLITE_ERROR == sqlite3_exec(db2.get(), kInsertSql,
                                           NULL, NULL, NULL));
  TEST_ASSERT(SQLITE_OK == sqlite3_close(db2.release()));

  // At this point we should still see the old name, because only
  // OpenSqliteDatabase() initiates the PermissionsDB change.
  std::string16 current_basename;
  TEST_ASSERT(permissions->GetDatabaseBasename(foo, kFooDatabaseName,
                                               &current_basename));
  TEST_ASSERT(basename == current_basename);

  // Open the database again.  It should work.
  TEST_ASSERT(OpenSqliteDatabase(kFooDatabaseName, foo, as_out_parameter(db)));

  // The basename should be different.
  TEST_ASSERT(permissions->GetDatabaseBasename(foo, kFooDatabaseName,
                                               &current_basename));
  TEST_ASSERT(basename != current_basename);

  // Delete the poisoned database now that we're done with it.
  filename = data_dir;
  filename += kPathSeparator;
  filename += basename;
  TEST_ASSERT(File::Delete(filename.c_str()));

  // The table we created now shouldn't be there.
  TEST_ASSERT(SQLITE_ERROR == sqlite3_exec(db.get(), kInsertSql,
                                           NULL, NULL, NULL));

  // The database should work just fine.
  TEST_ASSERT(SQLITE_OK == sqlite3_exec(db.get(), "CREATE TABLE t (c TEXT)",
                                        NULL, NULL, NULL));
  TEST_ASSERT(SQLITE_OK == sqlite3_exec(db.get(), kInsertSql,
                                        NULL, NULL, NULL));

  TEST_ASSERT(SQLITE_OK == sqlite3_close(db.release()));

  // Delete our new database file so we don't accumulate things in the
  // filesystem over many runs.
  filename = data_dir;
  filename += kPathSeparator;
  filename += current_basename;
  TEST_ASSERT(File::Delete(filename.c_str()));

  LOG(("TestPermissionsDBAll - passed\n"));
  return true;
}
