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

//
// This file contains code that is specific to Symbian.
// Differently from the rest of SQLite, it is implemented in C++ as this is
// the native language of the OS and all interfaces we need to use are C++.
//
// Most of the code is just a stub for the moment
// TODO(marcogelmi): implement the missing functions
//

#ifdef OS_SYMBIAN
#include <coemain.h>
#include <e32math.h>
#include <f32file.h>
#include <utf.h>

extern "C" {
#include "sqliteInt.h"
// Include code that is common to all os_*.c files
#include "os_common.h"
}

// The SymbianFile structure is a subclass of OsFile specific for the
// Symbian portability layer.
struct SymbianFile {
  IoMethod const *methods;  // Always the first entry
  RFile file;               // The file descriptor
  unsigned char locktype;   // The type of lock held on this fd
  TInt16 shared_lock_byte;  // Randomly chosen byte used as a shared lock
  int delete_on_close;
};


static TInt UTF8ToUTF16(const char *in, TDes* out16) {
  assert(in);
  TPtrC8 in_des(reinterpret_cast<const unsigned char*>(in));
  return CnvUtfConverter::ConvertToUnicodeFromUtf8(*out16, in_des);
}

static TInt UTF16ToUTF8(TDesC16& in16, TDes8* out8) {
  return CnvUtfConverter::ConvertFromUnicodeToUtf8(*out8, in16);
}

static int SymbianClose(OsFile **id) {
  assert(id);
  assert(*id);

  TFileName file_name;
  SymbianFile *file_id = reinterpret_cast<SymbianFile*>(*id);
  if (file_id->delete_on_close &&
      file_id->file.FullName(file_name) != KErrNone) {
    return SQLITE_ERROR;
  }
  file_id->file.Close();
  if (file_id->delete_on_close &&
      CCoeEnv::Static()->FsSession().Delete(file_name) != KErrNone) {
    return SQLITE_ERROR;
  }
  OpenCounter(-1);
  sqliteFree(file_id);
  return SQLITE_OK;
}

static int SymbianOpenDirectory(OsFile *id, const char *dir_name) {
  // noop on Symbian
  return SQLITE_OK;
}

static int SymbianRead(OsFile *id, void *buffer, int amount) {
  assert(id);
  assert(buffer);
  assert(amount >=0);

  SymbianFile* file_id = reinterpret_cast<SymbianFile*>(id);
  TPtr8 dest(static_cast<unsigned char*>(buffer), amount);

  if (KErrNone == file_id->file.Read(dest, amount)) {
    if (dest.Length() == amount) {
      return SQLITE_OK;
    } else {
      return SQLITE_IOERR_SHORT_READ;
    }
  } else {
    return SQLITE_IOERR;
  }
}

static int SymbianWrite(OsFile *id, const void *buffer, int amount) {
  assert(id);
  assert(buffer);
  assert(amount >=0);

  SymbianFile *file_id = reinterpret_cast<SymbianFile*>(id);
  TPtrC8 src(static_cast<const unsigned char*>(buffer), amount);
  if (file_id->file.Write(src) != KErrNone) return SQLITE_IOERR_WRITE;
  
  return SQLITE_OK;
}

static int SymbianSeek(OsFile *id, i64 offset) {
  assert(id);
  assert(offset >=0);

  SymbianFile* file_id = reinterpret_cast<SymbianFile*>(id);
  TInt pos = static_cast<TInt>(offset);
  if (file_id->file.Seek(ESeekStart, pos) != KErrNone) return SQLITE_IOERR;

  return SQLITE_OK;
}

static int SymbianTruncate(OsFile *id, i64 bytes) {
  assert(id);
  assert(bytes >=0);

  SymbianFile* file_id = reinterpret_cast<SymbianFile*>(id);
  TInt length =  static_cast<TInt>(bytes);
  if (file_id->file.SetSize(length) != KErrNone) return SQLITE_ERROR;

  return SQLITE_OK;
}

static int SymbianSync(OsFile *id, int /*flags*/) {
  assert(id);
  TInt err = KErrGeneral;
  SymbianFile *file_id = reinterpret_cast<SymbianFile*>(id);
  if (file_id->file.Flush() != KErrNone) return SQLITE_ERROR;

  return SQLITE_OK;
}

static void SymbianSetFullSync(OsFile* /*id*/, int /*v*/) {
  // noop on Symbian
  return;
}

static int SymbianFileHandle(OsFile* /*id*/) {
  return 0;
}

static int SymbianFileSize(OsFile *id, i64 *size_out) {
  assert(id);
  assert(size_out);

  TInt size;
  SymbianFile *file_id = reinterpret_cast<SymbianFile*>(id);
  if (file_id->file.Size(size) != KErrNone) return SQLITE_ERROR;

  *size_out = static_cast<i64>(size);
  return SQLITE_OK;
}

// TODO(marcogelmi): implement using RFile::Lock()
static int SymbianLock(OsFile *id, int locktype) {
  return SQLITE_OK;
}

// TODO(marcogelmi): implement (see above)
static int SymbianUnlock(OsFile *id, int locktype) {
  return SQLITE_OK;
}

// TODO(marcogelmi): implement
static int SymbianLockState(OsFile *id) {
  assert(id);
  return (reinterpret_cast<SymbianFile*>(id))->locktype;
}

// TODO(marcogelmi): implement
static int SymbianCheckReservedLock(OsFile *id) {
  return 0;
}

static int SymbianVectorSize(OsFile *id) {
  return SQLITE_DEFAULT_SECTOR_SIZE;
}

// This vector defines all the methods that can operate on an
// OsFile for Symbian.
static const IoMethod SymbianIoMethod = {
  SymbianClose,
  SymbianOpenDirectory,
  SymbianRead,
  SymbianWrite,
  SymbianSeek,
  SymbianTruncate,
  SymbianSync,
  SymbianSetFullSync,
  SymbianFileHandle,
  SymbianFileSize,
  SymbianLock,
  SymbianUnlock,
  SymbianLockState,
  SymbianCheckReservedLock,
  SymbianVectorSize,
};

static int AllocateSymbianFile(SymbianFile *file_in,
                               OsFile **id,
                               int del_flag) {
  assert(file_in);
  assert(id);

  SymbianFile *tmp_file;
  tmp_file = static_cast<SymbianFile*>(sqliteMalloc(sizeof(*tmp_file)));
  if ( NULL == tmp_file ) {
    file_in->file.Close();
    *id = NULL;
    return SQLITE_NOMEM;
  } else {
    *tmp_file = *file_in;
    tmp_file->methods = &SymbianIoMethod;
    tmp_file->locktype = NO_LOCK;
    tmp_file->shared_lock_byte = 0;
    tmp_file->delete_on_close = del_flag;
    *id = reinterpret_cast<OsFile*>(tmp_file);
    OpenCounter(+1);
    return SQLITE_OK;
  }
}

int sqlite3SymbianOpenReadOnly(const char *file_name, OsFile **id) {
  assert(file_name);
  assert(id);

  SymbianFile sfile;
  TFileName file_name_utf16;

  if (UTF8ToUTF16(file_name, &file_name_utf16) != KErrNone) 
    return SQLITE_CANTOPEN;

  if (sfile.file.Open(CCoeEnv::Static()->FsSession(), 
                      file_name_utf16, 
                      EFileRead) !=  KErrNone)
    return SQLITE_CANTOPEN;

  return AllocateSymbianFile(&sfile, id, 0);
}

int sqlite3SymbianOpenReadWrite(const char *file_name,
                                OsFile **id,
                                int *read_only) {
  assert(file_name);
  assert(id);
  assert(read_only);

  TInt err = KErrGeneral;
  SymbianFile sfile;
  TFileName file_name_utf16;

  if (UTF8ToUTF16(file_name, &file_name_utf16) != KErrNone)
    return SQLITE_CANTOPEN;

  err = sfile.file.Create(CCoeEnv::Static()->FsSession(), 
                          file_name_utf16, 
                          EFileWrite);

  if (KErrAlreadyExists == err) {
    err = sfile.file.Open(CCoeEnv::Static()->FsSession(), 
                          file_name_utf16, 
                          EFileWrite);
  }

  if (KErrNone == err) {
    *read_only = 0;
    return AllocateSymbianFile(&sfile, id, 0);
  }
  
  return SQLITE_CANTOPEN;
}

int sqlite3SymbianOpenExclusive(const char *file_name,
                                OsFile **id,
                                int del_flag) {
  assert(file_name);
  assert(id);

  SymbianFile sfile;
  TFileName file_name_utf16;

  if (UTF8ToUTF16(file_name, &file_name_utf16) != KErrNone)
    return SQLITE_CANTOPEN;

  if (sfile.file.Create(CCoeEnv::Static()->FsSession(),
                        file_name_utf16,
                        EFileWrite | EFileShareExclusive) ==  KErrNone) {
    return AllocateSymbianFile(&sfile, id, del_flag);
  }

  return SQLITE_CANTOPEN;
}

int sqlite3SymbianDelete(const char *file_name) {
  assert(file_name);
  TFileName file_name_utf16;

  if (UTF8ToUTF16(file_name, &file_name_utf16) != KErrNone)
    return SQLITE_ERROR;

  return KErrNone == CCoeEnv::Static()->FsSession().Delete(file_name_utf16) ?
         SQLITE_OK : SQLITE_ERROR;
}

int sqlite3SymbianFileExists(const char *file_name) {
  assert(file_name);

  TEntry entry;
  TFileName file_name_utf16;

  if (UTF8ToUTF16(file_name, &file_name_utf16) != KErrNone)
    return SQLITE_ERROR;

  if (KErrNone == CCoeEnv::Static()->FsSession().Entry(file_name_utf16, entry))
    return !entry.IsDir();

  return SQLITE_ERROR;
}

char *sqlite3SymbianFullPathname(const char *relative) {
  assert(relative);
  // TODO(marcogelmi): implement this properly using TParse
  return sqliteStrDup(relative);
}

int sqlite3SymbianIsDirWritable(char *dir_name) {
  assert(dir_name);
  assert(strlen(dir_name) < KMaxPath);

  TEntry entry;
  TFileName dir_name_utf16;
  
  if (UTF8ToUTF16(dir_name, &dir_name_utf16) != KErrNone)
    return SQLITE_ERROR;

  if (KErrNone == CCoeEnv::Static()->FsSession().Entry(dir_name_utf16, entry))
    return entry.IsDir() && !entry.IsReadOnly();

  return SQLITE_ERROR;
}

int sqlite3SymbianSyncDirectory(const char * /*dir_name*/) {
  // noop on Symbian
  return SQLITE_OK;
}

int sqlite3SymbianTempFileName(char *buffer) {
  const char kChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  _LIT(kSqliteTempFilePrefix, "etilqs_");

  assert(buffer);
  TInt64 seed = User::TickCount();
  TUint8 c;
  TPtr8 buffer_des((unsigned char*) buffer, KMaxPath);
  TFileName private_path;
  TBuf8<KMaxPath> private_path_utf8;

  if (CCoeEnv::Static()->FsSession().PrivatePath(private_path) != KErrNone)
    return SQLITE_ERROR;

  if (UTF16ToUTF8(private_path, &private_path_utf8) != KErrNone)
    return SQLITE_ERROR;

  buffer_des.Zero();
  buffer_des.Append(CCoeEnv::Static()->FsSession().GetSystemDriveChar());
  buffer_des.Append(KDriveDelimiter);
  buffer_des.Append(private_path_utf8);
  buffer_des.Append(kSqliteTempFilePrefix);

  for (TInt i = 0; i < 20; i++) {
    c = kChars[Math::Rand(seed) % (sizeof(kChars)-1)];
    buffer_des.Append(c);
  }
  buffer_des.PtrZ();

  return SQLITE_OK;
}

// Get information to seed the random number generator.  The seed
// is written into the buffer[256].  The calling function must
// supply a sufficiently large buffer.
int sqlite3SymbianRandomSeed(char *buffer) {
  assert(buffer);

  TInt64 seed = User::TickCount();
  for (TInt i = 0; i < 256; i++) {
    buffer[i] = Math::Rand(seed) % 255;
  }
  return SQLITE_OK;
}

int sqlite3SymbianSleep(int ms) {
  assert(ms >= 0);
  User::After(TTimeIntervalMicroSeconds32(ms*1000));
  return SQLITE_OK;
}

// returns the Julian Date
// = (number of seconds from 1-1-4713 BC) / (seconds per day)
int sqlite3SymbianCurrentTime(double *now) {
  _LIT(kEpoch, "19700101:000000.000000");
  assert(now);
  TTime time;
  TTime epoch_time(kEpoch);
  TTimeIntervalSeconds interval;

  time.HomeTime();
  // calculate seconds elapsed since 1-1-1970
  time.SecondsFrom(epoch_time, interval);

  // Julian date @ 1-1-1970 = 2440587.5
  // seconds per day = 86400.0
  *now = interval.Int()/86400.0 + 2440587.5;
  return 0;
}

// ========================================================
// TODO(marcogelmi): implement the following functions
// ========================================================
static int g_in_mutex = 0;
void sqlite3SymbianEnterMutex() {
  g_in_mutex++;
  return;
}

void sqlite3SymbianLeaveMutex() {
  g_in_mutex--;
  return;
}

int sqlite3SymbianInMutex(int this_thread_only) {
  return g_in_mutex;
}

ThreadData *sqlite3SymbianThreadSpecificData(int allocate_flag) {
  return NULL;
}

void *sqlite3SymbianDlopen(const char *file_name) {
  return NULL;
}

void *sqlite3SymbianDlsym(void *pHandle, const char *zSymbol) {
  return NULL;
}

int sqlite3SymbianDlclose(void *pHandle) {
  return 0;
}

#endif /* OS_SYMBIAN*/
