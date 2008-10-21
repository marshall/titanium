// Copyright 2006, Google Inc.
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

#include "gears/database/database_utils.h"

#include "gears/base/common/exception_handler.h"
#include "gears/base/common/file.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h" // for IsStringValidPathComponent()

int ForbidActions(void *userData, int iType,
                  const char *zPragma, const char *zArg,
                  const char *zDatabase, const char *zView) {
  if (iType == SQLITE_PRAGMA) {
    // TODO(shess): Could test zPragma for the specific pragma being
    // accessed, and allow some of them.  Could allow zArg==NULL
    // through to enable read-only access.  Deny-all for now to leave
    // open the possibility of compiling the pragma code out entirely.
    return SQLITE_DENY;
  }
#ifdef OS_ANDROID
  if (iType == SQLITE_ATTACH) {
    // On Android, we're using the system version of SQLite which
    // isn't compiled with SQLITE3_OMIT_ATTACH, so we have to deny
    // access here.
    return SQLITE_DENY;
  }
#endif
  return SQLITE_OK;
}

// SQLITE_CORRUPT handling.
//
// There is a possible-irreducable minimum of corruption which will
// happen in the real world, for reasons not pertinent to this
// discussion.  We handle this by deleting the database and opening a
// new database with no data.  This is admittedly a coarse approach,
// but it is important not to make perfect the enemy of good enough,
// since corrupt databases can effectively hang apps until the user
// manually deletes the database file.
//
// The process comes in two parts.  When SqlitePoisonIfCorrupt() sees
// SQLITE_CORRUPT, it calls sqlite3Poison() to write an invalid header
// into the database file.  This will cause further access in any
// thread of control to fail with error SQLITE_NOTADB (which
// SqlitePoisonIfCorrupt() will helpfully coerce to SQLITE_CORRUPT).
// All Database calls should throw SQLITE_CORRUPT until close() is
// called.
//
// The second part happens at open().  If the database is detected to
// be poisoned, the record in PermissionsDB which represents this
// database is marked so that future opens roll over to a new database
// file, which will be empty.
//
// The above is complicated by the fact that we cannot delete database
// files while they are open in other threads of control.  On Windows,
// this is simply not possible (open files cannot be deleted or
// renamed).  Additionally, even on Unix-based systems you could end
// up with distinct physical databases using a shared journal file,
// which could cause problems.  Lastly, we might want to anyhow save
// the corrupt files for debugging purposes.
//
// A further complication is that we may not have an easy way to force
// other threads of control to execute anything.  A Worker may have a
// sqlite3 handle open to the database but may be executing code
// unrelated to the database.  So this code has to assume that the
// other handles may try to access the database at some arbitrary
// future time.
//
// TODO(shess): Encapsulate the SQLite database in a way that allows
// us to poison the database (to cause other handles to that database
// to start throwing) and mark the entry in PermissionsDB corrupt at
// the same time.  This requires the ability to remember the specific
// basename associated with the database handle.

// TODO(shess): Get this exported from sqlite more cleanly.
extern "C" int sqlite3Poison(sqlite3 *db);

// If rc indicates corruption, poison the database so that other users
// don't trust it.  Coerce SQLITE_NOTADB to SQLITE_CORRUPT because if
// the file is not a database, Gears clients cannot deal with the
// problem.  If they close and re-open, we will shift it aside.
int SqlitePoisonIfCorrupt(sqlite3 *db, int rc) {
  if (rc == SQLITE_CORRUPT) {
    sqlite3Poison(db);
  } else if (rc == SQLITE_NOTADB) {
    rc = SQLITE_CORRUPT;
  }

  return rc;
}

// Open filename as a SQLite database, and setup appropriately for
// Gears use.  Returns SQLITE_OK in case of success, otherwise returns
// the error code sqlite returned.  The caller must arrange to
// eventually call sqlite3_close() on the handle returned in *db even
// if an error is returned.
static int OpenAndSetupDatabase(const std::string16 &filename, sqlite3 **db) {
  // Make sure we aren't overwriting an existing handle (leading to a
  // leak), or garbage (leading to anything).  sqlite3_close(NULL) is
  // a valid thing to do, so that's where we want to start.
  assert(*db == NULL);

  int sql_status = sqlite3_open16(filename.c_str(), db);
  if (sql_status != SQLITE_OK) {
    return sql_status;
  }

#ifdef OS_ANDROID
  // The system SQLite doesn't have BEGIN defaulting to
  // IMMEDIATE. This Android-specific function call sets an individual
  // instance to this behavior.
  sql_status = sqlite3_set_transaction_default_immediate(*db, 1);
  if (sql_status != SQLITE_OK) {
    return sql_status;
  }
#endif

  // Set the busy timeout value before executing any SQL statements.
  // With the timeout value set, SQLite will wait and retry if another
  // thread has the database locked rather than immediately fail with an
  // SQLITE_BUSY error.
  const int kSQLiteBusyTimeout = 5000;
  sqlite3_busy_timeout(*db, kSQLiteBusyTimeout);

  // Set reasonable defaults.
  sql_status = sqlite3_exec(*db,
                            "PRAGMA encoding = 'UTF-8';"
                            "PRAGMA auto_vacuum = 1;"
                            "PRAGMA cache_size = 2048;"
                            "PRAGMA page_size = 4096;"
#ifdef WINCE
                            // Using in-memory temp files gives approximately
                            // 3x speed improvement on Windows Mobile.
                            "PRAGMA temp_store = MEMORY;"
#endif
                            "PRAGMA synchronous = NORMAL;",
                            NULL, NULL, NULL
                            );
  if (sql_status != SQLITE_OK) {
    return sql_status;
  }

  sql_status = sqlite3_set_authorizer(*db, ForbidActions, NULL);
  if (sql_status != SQLITE_OK) {
    return sql_status;
  }

  return SQLITE_OK;
}

// Combine dirname and dbname to create an appropriate internal
// database name, and open that database.  If the database is corrupt,
// poison it (so that other clients don't trust it) and signal
// PermissionsDB to stop using this name.  db is only a valid sqlite3
// handle if true is returned.
static bool OpenAndCheckDatabase(const SecurityOrigin &origin,
                                 const std::string16 &dirname,
                                 const std::string16 &dbname,
                                 sqlite3 **db) {
  PermissionsDB *pdb = PermissionsDB::GetDB();
  if (pdb == NULL) {
    return false;
  }

  std::string16 basename;
  if (!pdb->GetDatabaseBasename(origin, dbname.c_str(), &basename)) {
    return false;
  }

  std::string16 filename(dirname);
  filename += kPathSeparator;
  filename += basename;

  sqlite3 *temp_db = NULL;
  int sql_status = OpenAndSetupDatabase(filename, &temp_db);
  if (sql_status != SQLITE_OK) {
    sql_status = SqlitePoisonIfCorrupt(temp_db, sql_status);
    if (sql_status == SQLITE_CORRUPT) {
      ExceptionManager::ReportAndContinue();
      pdb->MarkDatabaseCorrupt(origin, dbname.c_str(), basename.c_str());
    }

    sqlite3_close(temp_db);
    return false;
  }

  *db = temp_db;
  return true;
}

bool OpenSqliteDatabase(const char16 *name, const SecurityOrigin &origin,
                        sqlite3 **db) {
  std::string16 dirname;

  // Setup the directory.
  if (!GetDataDirectory(origin, &dirname)) {
    return false;
  }

  // Ensure directory exists; sqlite_open does not do this.
  if (!File::RecursivelyCreateDir(dirname.c_str())) {
    return false;
  }

  if (!IsUserInputValidAsPathComponent(name, NULL)) {
    return false;
  }

  // Open the SQLite database and check for corruption.
  if (OpenAndCheckDatabase(origin, dirname, name, db)) {
    return true;
  }

  // If the existing database was corrupt, the first open will fail.
  // Give it one more chance.  If this call fails, further calls are
  // likely to also fail, so don't bother.
  return OpenAndCheckDatabase(origin, dirname, name, db);
}
