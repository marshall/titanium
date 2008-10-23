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
#ifdef USING_CCTESTS

#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/sqlite_wrapper_test.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/string_utils.h"

static bool TestSQLDatabaseTransactions();
static bool TestSQLTransaction();
static bool CreateTable(SQLDatabase &db);
static bool InsertRow(SQLDatabase &db);

// Not static because this function is declared as a friend of SQLDatabase
bool TestSQLConcurrency();


//------------------------------------------------------------------------------
// TestSqliteUtilsAll
//------------------------------------------------------------------------------
bool TestSqliteUtilsAll(std::string16 *error) {
  bool ok = true;
  ok &= TestSQLDatabaseTransactions();
  ok &= TestSQLTransaction();
  ok &= TestSQLConcurrency();
  if (!ok) {
    assert(error); \
    *error += STRING16(L"TestSqliteUtilsAll - failed. "); \
  }
  return ok;
}


//------------------------------------------------------------------------------
// TestSQLDatabaseTransactions
//------------------------------------------------------------------------------
static bool TestSQLDatabaseTransactions() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestSQLDatabaseTransactions - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}

  // Put something into the DB using a nested transaction
  {
    SQLDatabase db1;
    TEST_ASSERT(db1.Open(STRING16(L"SqliteUtils_test.db")));

    TEST_ASSERT(SQLITE_OK == db1.Execute("DROP TABLE IF EXISTS test"));

    const char *kTransactionLabel = "TestSQLDatabaseTransactions";

    TEST_ASSERT(db1.BeginTransaction(kTransactionLabel));
    TEST_ASSERT(SQLITE_OK == db1.Execute("CREATE TABLE test (val TEXT)"));

    TEST_ASSERT(db1.BeginTransaction(kTransactionLabel));
    TEST_ASSERT(SQLITE_OK == db1.Execute("INSERT INTO test VALUES ('foo')"));

    TEST_ASSERT(db1.CommitTransaction(kTransactionLabel));
    TEST_ASSERT(db1.CommitTransaction(kTransactionLabel));
  }

  // Now check that it is there
  {
    SQLDatabase db2;
    TEST_ASSERT(db2.Open(STRING16(L"SqliteUtils_test.db")));

    SQLStatement stmt;
    TEST_ASSERT(SQLITE_OK ==
      stmt.prepare16(&db2, STRING16(L"SELECT 1 FROM test WHERE val='foo'")));
    TEST_ASSERT(SQLITE_ROW == stmt.step());
    TEST_ASSERT(1 == stmt.column_int(0));
  }

  LOG(("TestSQLDatabaseTransactions - passed\n"));
  return true;
}


//------------------------------------------------------------------------------
// TestSQLTransaction
//------------------------------------------------------------------------------
static bool TestSQLTransaction() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestSQLTransaction - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}

  // Put something into the DB using a nested transaction
  {
    SQLDatabase db1;
    TEST_ASSERT(db1.Open(STRING16(L"SqliteUtils_test.db")));

    TEST_ASSERT(SQLITE_OK == db1.Execute("DROP TABLE test"));

    TEST_ASSERT(CreateTable(db1));
  }

  // Now verify that the table is not there
  {
    SQLDatabase db2;
    TEST_ASSERT(db2.Open(STRING16(L"SqliteUtils_test.db")));

    SQLStatement stmt;
    TEST_ASSERT(SQLITE_OK == 
      stmt.prepare16(&db2, 
        STRING16(L"SELECT 1 FROM sqlite_master WHERE tbl_name='test'")));
    TEST_ASSERT(SQLITE_DONE == stmt.step());
  }

  LOG(("TestSQLTransaction - passed\n"));
  return true;
}


//------------------------------------------------------------------------------
// CreateTable helper used by TestSQLTransaction
//------------------------------------------------------------------------------
static bool CreateTable(SQLDatabase &db) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("CreateTable - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}

  SQLTransaction tx(&db, "TestSQLTransaction::CreateTable");
  TEST_ASSERT(tx.Begin());

  TEST_ASSERT(SQLITE_OK == db.Execute("CREATE TABLE test (val TEXT)"));

  TEST_ASSERT(InsertRow(db));
  return true;
}


//------------------------------------------------------------------------------
// InsertRow helper used by TestSQLTransaction
//------------------------------------------------------------------------------
static bool InsertRow(SQLDatabase &db) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("InsertRow - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}

  SQLTransaction tx(&db, "TestSQLTransaction::InsertRow");

  TEST_ASSERT(tx.Begin());

  // There should be an error because test doesn't exist
  TEST_ASSERT(SQLITE_OK == db.Execute("INSERT INTO test VALUES ('foo')"));

  // Now roll it back
  tx.Rollback();

  return true;
}


bool TestSQLConcurrency() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestSQLConcurrency - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}

  // db1, open and configure our connection with pragmas as desired
  SQLDatabase db1;
  TEST_ASSERT(db1.Open(STRING16(L"SqliteUtils_test.db")));
  
  // db2, open a connection but don't configure yet
  SQLDatabase db2;
  TEST_ASSERT(db2.OpenConnection(STRING16(L"SqliteUtils_test.db")));

  // db1, get an exclusive lock
  TEST_ASSERT(SQLITE_OK == db1.Execute("BEGIN EXCLUSIVE"));
    
  // db2, now try to configure our pragmas.
  // This should fail with a busy error after a timeout 
  int64 start_msec = GetCurrentTimeMillis();
  TEST_ASSERT(!db2.ConfigureConnection());
  TEST_ASSERT(db2.GetErrorCode() == SQLITE_BUSY);
  int kSlopMillis = 500;
  TEST_ASSERT(GetCurrentTimeMillis() - start_msec >
              SQLDatabase::kBusyTimeout - kSlopMillis);
  db2.Close();  // cleanup
  
  // After releasing the exclusive lock, we should be able to
  // open and configure a second connection
  TEST_ASSERT(SQLITE_OK == db1.Execute("ROLLBACK"));
  TEST_ASSERT(db2.Open(STRING16(L"SqliteUtils_test.db")));

  return true;
}


#endif  // USING_CCTESTS
