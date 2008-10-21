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

#ifdef DEBUG
#if BROWSER_IE
#include "gears/base/ie/atl_headers.h"
#endif
#endif

#include "gears/localserver/common/file_store.h"
#include <time.h>
#include <algorithm>
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/localserver_db.h"

const char16 *kGenericCacheFilename = STRING16(L"File");
const int kMaxFilesPerDirectory = 500;
const int kMaxSubDirectoriesPerLevel = 100;

static bool CreateUniqueFile(const char16* full_dirpath,
                             int unique_hint,
                             const std::string16 &filename_unsanitized,
                             std::string16 *full_filepath);
static void AppendBracketedNumber(int number, std::string16 *str);
static bool GetFileNameFromUrl(const char16 *url, std::string16 *filename);

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
bool WebCacheFileStore::Init(WebCacheDB *db) {
  ASSERT_SINGLE_THREAD();
  if (!GetBaseDataDirectory(&root_dir_)) {
    return false;
  }
  if (!File::RecursivelyCreateDir(root_dir_.c_str())) {
    return false;
  }
  root_dir_ += kPathSeparator;
  began_ = false;
  return WebCacheBlobStore::Init(db);
}

//------------------------------------------------------------------------------
// InsertBody
//------------------------------------------------------------------------------
bool WebCacheFileStore::InsertBody(int64 server_id,
                                   const char16 *url,
                                   WebCacheDB::PayloadInfo *payload) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  assert(began_);
  assert(payload->id != WebCacheDB::kUnknownID);
  if (!began_) return false;

  // We only store the bodies of successful responses
  if (payload->status_code != HttpConstants::HTTP_OK) {
    return true;
  }

  // Put a file on disk, the relative file path is returned in
  // payload.cached_filepath. If the current transaction rollsback,
  // the newly created file will be deleted.
  if (!CreateAndWriteFile(server_id, url, payload)) {
    return false;
  }

  // Insert a row in the ResponseBodies table containing the filepath. Note
  // that BodyID is the same as PayloadID in the Payloads table.
  const char16* sql = STRING16(L"INSERT INTO ResponseBodies"
                               L" (BodyID, FilePath)"
                               L" VALUES (?, ?)");
  SQLStatement stmt;
  int rv = stmt.prepare16(db_->GetSQLDatabase(), sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.InsertBody failed\n"));
    return false;
  }
  int param = -1;
  rv = stmt.bind_int64(++param, payload->id);
  rv |= stmt.bind_text16(++param, payload->cached_filepath.c_str());
  if (rv != SQLITE_OK) {
    return false;
  }

  // Return the full filepath to the caller
  PrependRootFilePath(&payload->cached_filepath);

  return (stmt.step() == SQLITE_DONE);
}

//------------------------------------------------------------------------------
// ReadBody
//------------------------------------------------------------------------------
bool WebCacheFileStore::ReadBody(WebCacheDB::PayloadInfo *payload,
                                 bool info_only) {
  ASSERT_SINGLE_THREAD();
  assert(db_);

  // For bodyless responses, we don't hit the DB at all
  if (payload->status_code != HttpConstants::HTTP_OK) {
    payload->cached_filepath.clear();
    payload->data.reset(NULL);
    return true;
  }

  // Read the relative filepath from the ResponseBodies table
  if (!GetFilePath(payload->id, &payload->cached_filepath)) {
    return false;
  }

  if (!info_only) {
    if (!ReadFile(payload)) {
      // TODO(michaeln): handle this error condition, the file associated
      // with the payload has been deleted
      return false;
    }
  }

  // We return the full filepath to the caller
  PrependRootFilePath(&payload->cached_filepath);
  return true;
}

//------------------------------------------------------------------------------
// DeleteBody
//------------------------------------------------------------------------------
bool WebCacheFileStore::DeleteBody(int64 payload_id) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  assert(began_);
  if (!began_) return false;

  // Read the filepath from the ResponseBodies table.
  std::string16 filepath;
  if (GetFilePath(payload_id, &filepath)) {
    // Delete the file on disk. Note the file is not really deleted until
    // the current transaction commits.
    DeleteFile(filepath.c_str());
  }

  // Delete the row from the response bodies table.
  return WebCacheBlobStore::DeleteBody(payload_id);
}

//------------------------------------------------------------------------------
// DeleteUnreferencedBodies
//------------------------------------------------------------------------------
bool WebCacheFileStore::DeleteUnreferencedBodies() {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  assert(began_);
  if (!began_) return false;

  // Delete files, note the files aren't actually deleted until the current
  // transaction is committed
  const char16* sql = STRING16(L"SELECT FilePath FROM ResponseBodies "
                               L"WHERE BodyID NOT IN "
                               L"(SELECT DISTINCT PayloadID FROM Payloads)");
  SQLStatement select_stmt;
  int rv = select_stmt.prepare16(db_->GetSQLDatabase(), sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheDB.DeleteUnreferencedBodies failed\n"));
    return false;
  }
  while (select_stmt.step() == SQLITE_ROW) {
    DeleteFile(select_stmt.column_text16(0));
  }
  sqlite3_finalize(select_stmt.release());

  // Now delete the rows
  return WebCacheBlobStore::DeleteUnreferencedBodies();
}

//------------------------------------------------------------------------------
// CreateDirectoryForServer
//------------------------------------------------------------------------------
bool WebCacheFileStore::CreateDirectoryForServer(int64 server_id) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  assert(began_);
  if (!began_) return false;

  std::string16 server_dir;
  if (!GetDirectoryPathForServer(server_id, &server_dir)) {
    return false;
  }
  if (File::DirectoryExists(server_dir.c_str())) {
    // If the directory already exists, we don't want to delete it
    // on rollback
    return true;
  }
  if (!File::RecursivelyCreateDir(server_dir.c_str())) {
    return false;
  }
  delete_directories_on_rollback_.push_back(server_dir);
  return true;
}

//------------------------------------------------------------------------------
// DeleteDirectoryForServer
//------------------------------------------------------------------------------
void WebCacheFileStore::DeleteDirectoryForServer(int64 server_id) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  assert(began_);
  if (!began_) return;

  std::string16 server_dir;
  if (!GetDirectoryPathForServer(server_id, &server_dir)) {
    return;
  }
  delete_directories_on_commit_.push_back(server_dir);
}

//------------------------------------------------------------------------------
// GetDirectoryPathForServer
//------------------------------------------------------------------------------
bool WebCacheFileStore::GetDirectoryPathForServer(int64 server_id,
                                                  std::string16 *server_dir) {
  // Determine the directory we'll put files in based on the security
  // origin, name, and type of the store. The directory path
  // also includes the server_id. Ex.
  // <data_dir>/<directory_for_origin>/Name[15]#localserver
  // <data_dir>/<directory_for_origin>/Name_managed[27]#localserver

  // Get "<data_dir>/<directory_for_origin>", a full directory path
  WebCacheDB::ServerInfo server;
  SecurityOrigin security_origin;
  if (!db_->FindServer(server_id, &server) ||
      !security_origin.InitFromUrl(server.security_origin_url.c_str()) ||
      !GetDataDirectory(security_origin, server_dir)) {
    return false;
  }

  // Form the "Name[id]" part of the path
  std::string16 server_dir_name(server.name);
  if (server.server_type == WebCacheDB::MANAGED_RESOURCE_STORE) {
    const char16 *kManagedSuffix = STRING16(L"_managed");
    server_dir_name += kManagedSuffix;
  }
  AppendBracketedNumber(static_cast<int>(server_id), &server_dir_name);

  // Stitch the two together, and append the "#localserver" suffix
  AppendDataName(server_dir_name.c_str(), kDataSuffixForLocalServer,
                 server_dir);
  return true;
}

//------------------------------------------------------------------------------
// Read the relative filepath from the ResponseBodies table
//------------------------------------------------------------------------------
bool WebCacheFileStore::GetFilePath(int64 payload_id, std::string16 *filepath) {
  const char16* sql = STRING16(L"SELECT FilePath FROM ResponseBodies "
                               L"WHERE BodyID=?");
  SQLStatement stmt;
  int rv = stmt.prepare16(db_->GetSQLDatabase(), sql);
  if (rv != SQLITE_OK) {
    LOG(("WebCacheFileStore.GetFilePath failed\n"));
    return false;
  }
  rv = stmt.bind_int64(0, payload_id);
  if (rv != SQLITE_OK) {
    return false;
  }
  if (stmt.step() != SQLITE_ROW) {
    return false;
  }
  *filepath = stmt.column_text16_safe(0);
  return true;
}

//------------------------------------------------------------------------------
// Reads the contents of a cached file into memory. The file to read
// is determined by payload.cached_filepath. The file data is read into a new
// vector and a reference to that vector is placed in payload.data.
//------------------------------------------------------------------------------
bool WebCacheFileStore::ReadFile(WebCacheDB::PayloadInfo *payload) {
  ASSERT_SINGLE_THREAD();
  std::string16 full_filepath(payload->cached_filepath);
  PrependRootFilePath(&full_filepath);
  scoped_ptr< std::vector<uint8> > data(new std::vector<uint8>);
  if (!File::ReadFileToVector(full_filepath.c_str(), data.get())) {
    return false;
  }
  payload->data.reset(data.release());
  return true;
}

//------------------------------------------------------------------------------
// Creates and writes a new cached file on disk. The filename is determined
// by first examining the payload.headers for a filename. If that fails,
// the filename is based on the url.  The file is created in the directory for
// the given server_id. The relative filepath is returned in
// payload.cached_filepath. The contents of the file are determined by
// payload.data. If Rollback() is called prior to Commit(), the newly
// created file will be deleted.
//------------------------------------------------------------------------------
bool WebCacheFileStore::CreateAndWriteFile(int64 server_id,
                                           const char16 *url,
                                           WebCacheDB::PayloadInfo *payload) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  assert(payload->id != WebCacheDB::kUnknownID);
  assert(began_);
  if (!began_) return false;

  std::string16 server_dir;
  if (!GetDirectoryPathForServer(server_id, &server_dir)) {
    return false;
  }

  // Find a directory with space available within server_dir
  // This may be server_dir or a subdirectory of server_dir
  // TODO(michaeln): Perhaps revisit. Should we hash the 'url'
  // to a 32 bit number and use the four octets as the subdirectory
  // path instead of enumerating directory items to count them?
  // Our current scheme has the nice characteristic of all files
  // for a given store appearing in a single directory.
  std::string16 available_dir;
  FindDirectoryWithSpaceAvailable(server_dir.c_str(), &available_dir);
  if (!File::RecursivelyCreateDir(available_dir.c_str())) {
    return false;
  }

  // Determine a filename for the file we'll create
  std::string16 filename;
  GetCacheFileName(url, payload, &filename);

  // Create a uniquely named empty file
  // The filename param returns the unique file name
  // The full_filepath params returns the full path of the uniquely named file
  std::string16 full_filepath;
  if (!CreateUniqueFile(available_dir.c_str(),
                        static_cast<int>(payload->id),
                        filename, &full_filepath)) {
    return false;
  }

  // If the transaction fails, we will delete this file
  delete_on_rollback_.push_back(full_filepath);

  // Write the file
  if (!File::WriteVectorToFile(full_filepath.c_str(), payload->data.get())) {
    return false;
  }

  // Return to the caller the relative path of the file in our root_dir
  payload->cached_filepath = full_filepath.substr(root_dir_.length());

  return true;
}

//------------------------------------------------------------------------------
// Deletes a cached file from disk. The delete doesn't actually occur until
// Commit() is called. If Rollback() is called prior to Commit(), the file
// will not be deleted.
//------------------------------------------------------------------------------
bool WebCacheFileStore::DeleteFile(const char16 *filepath) {
  ASSERT_SINGLE_THREAD();
  assert(began_);
  if (!began_) return false;
  if (!filepath || !filepath[0]) return false;
  std::string16 full_filepath(filepath);
  PrependRootFilePath(&full_filepath);
  delete_on_commit_.push_back(full_filepath);
  return true;
}

//------------------------------------------------------------------------------
// Prepends the root directory path to the given relative filepath
//------------------------------------------------------------------------------
void WebCacheFileStore::PrependRootFilePath(std::string16 *filepath) {
  ASSERT_SINGLE_THREAD();
  assert(db_);
  filepath->insert(0, root_dir_);
}

//------------------------------------------------------------------------------
// BeginTransaction
//------------------------------------------------------------------------------
void WebCacheFileStore::BeginTransaction() {
  ASSERT_SINGLE_THREAD();
  assert(!began_);
  assert(delete_on_commit_.empty());
  assert(delete_on_rollback_.empty());
  assert(delete_directories_on_commit_.empty());
  assert(delete_directories_on_rollback_.empty());
  delete_on_commit_.clear();
  delete_on_rollback_.clear();
  delete_directories_on_commit_.clear();
  delete_directories_on_rollback_.clear();
  began_ = true;
}

// A DeleteRecursively wrapper that takes a std::string16& argument for use
// with the for_each algorithm
inline bool DeleteOneDirectory(std::string16 &full_dirpath) {
  return File::DeleteRecursively(full_dirpath.c_str());
}

// A DeleteFile wrapper that takes a std::string16& argument for use
// with the for_each algorithm
inline bool DeleteOneFile(std::string16 &full_filepath) {
  return File::Delete(full_filepath.c_str());
}

//------------------------------------------------------------------------------
// CommitTransaction
//------------------------------------------------------------------------------
void WebCacheFileStore::CommitTransaction() {
  ASSERT_SINGLE_THREAD();
  assert(began_);
  std::for_each(delete_on_commit_.begin(),
                delete_on_commit_.end(),
                DeleteOneFile);
  std::for_each(delete_directories_on_commit_.begin(),
                delete_directories_on_commit_.end(),
                DeleteOneDirectory);
  delete_on_commit_.clear();
  delete_on_rollback_.clear();
  delete_directories_on_commit_.clear();
  delete_directories_on_rollback_.clear();
  began_ = false;
}

//------------------------------------------------------------------------------
// RollbackTransaction
//------------------------------------------------------------------------------
void WebCacheFileStore::RollbackTransaction() {
  ASSERT_SINGLE_THREAD();
  assert(began_);
  std::for_each(delete_on_rollback_.begin(),
                delete_on_rollback_.end(),
                DeleteOneFile);
  std::for_each(delete_directories_on_rollback_.begin(),
                delete_directories_on_rollback_.end(),
                DeleteOneDirectory);
  delete_on_commit_.clear();
  delete_on_rollback_.clear();
  delete_directories_on_commit_.clear();
  delete_directories_on_rollback_.clear();
  began_ = false;
}

//------------------------------------------------------------------------------
// We limit the number of files we store in a single directory. This
// method identifies a directory with parent in its path that has space
// available. If there is room in parent, it will be returned as available.
//------------------------------------------------------------------------------
void
WebCacheFileStore::FindDirectoryWithSpaceAvailable(const char16 *parent,
                                                   std::string16 *available) {
  std::string16 dir(parent);
  while (File::GetDirectoryFileCount(dir.c_str()) >
         kMaxFilesPerDirectory) {
    // There are too many files in this dir, use a subdirectory.
    // We create up to kMaxSubDirectoriesPerLevel and randomly place
    // files into them. Subdirectory names are numbers in the
    // range [0, kMaxSubDirectoriesPerLevel).
    int random = rand() % kMaxSubDirectoriesPerLevel;
    dir += kPathSeparator;
    dir += IntegerToString16(random);
  }
  *available = dir;
}

//------------------------------------------------------------------------------
// GetCacheFileName
// Determines the preferred file name based on response headers and the url.
//------------------------------------------------------------------------------
void WebCacheFileStore::GetCacheFileName(const char16 *url,
                                         WebCacheDB::PayloadInfo *payload,
                                         std::string16 *filename) {
  filename->clear();

  // Look for the custom header we add when store.CaptureFile is called
  if (payload->GetHeader(HttpConstants::kXCapturedFilenameHeader, filename)) {
    if (!filename->empty()) {
      return;
    }
  }

  // Look for a Content-Disposition header with a filename attribute
  // TODO(michaeln): this is a hack, need a better parser, perhaps CAtlRegExp
  // The value uses a character encoding that is particular to this header
  // The value has multiple semi-colon delimited fields
  // See the following for an implementation
  // '//depot/google3/java/com/google/httputil/ContentDisposition.java'
  // '//depot/google3/java/com/google/parser/Parser.java'
  std::string16 content_disposition;
  payload->GetHeader(HttpConstants::kContentDispositionHeader,
                     &content_disposition);
  size_t pos = content_disposition.find(STRING16(L"attachment"));
  if (pos != std::string16::npos) {
    pos = content_disposition.find(STRING16(L"filename"), pos);
    if (pos != std::string16::npos) {
      pos = content_disposition.find('=', pos);
      if (pos != std::string16::npos) {
        ++pos;
        std::string16 value = content_disposition.substr(pos);
        pos = value.find(';');
        if (pos != std::string16::npos) {
          value.resize(pos);
        }
        const char16 *trimmed = value.c_str();
        int trimmed_len = value.length();
        StripWhiteSpace(&trimmed, &trimmed_len);
        // Strip a leading and trailing quote
        if (*trimmed == '"' || *trimmed == '\'') {
          ++trimmed;
          trimmed_len -= 2;
        }
        if (trimmed_len > 0) {
          filename->assign(trimmed, trimmed_len);
          return;
        }
      }
    }
  }

  // Crack the url and use the name found there
  GetFileNameFromUrl(url, filename);

  // Otherwise, just use a constant
  if (filename->empty()) {
    *filename = kGenericCacheFilename;
  }
}

//------------------------------------------------------------------------------
// CreateUniqueFile
//------------------------------------------------------------------------------
static bool CreateUniqueFile(const char16* full_dirpath,
                             int unique_hint,
                             const std::string16 &filename_unsanitized,
                             std::string16 *full_filepath) {
  // Sanitize the filename.
  std::string16 filename = filename_unsanitized;
  EnsureStringValidPathComponent(filename, true);

  // Split the name and extension.
  std::string16 new_name = filename;
  std::string16 extension = File::GetFileExtension(filename.c_str());
  if (extension.length() > 0) {
    new_name = filename.substr(0, filename.find(extension));
  }

  // Limit length of extension.
  if (extension.length() > kFileExtensionMaxChars) {
    extension = extension.substr(0, kFileExtensionMaxChars);
  }

  // We embed the unique_hint into the name to help with uniqueness. Since
  // this is the payload_id (SQLite rowid), this should be enough to make
  // it unique.
  std::string16 name_suffix;
  AppendBracketedNumber(unique_hint, &name_suffix);

  // Shorten new_name to fit into kUserPathComponentMaxChars chars along
  // with the unique & retry suffixes.
  // The 3 refers to the length of the retry suffix which can be [0]-[9].
  int extra_length = name_suffix.length() + 3 + extension.length();
  if ((new_name.length() + extra_length) > kUserPathComponentMaxChars) {
    int chars_to_keep = kUserPathComponentMaxChars - extra_length;
    new_name = new_name.substr(0, chars_to_keep);
  }

  // Append unique hint.
  new_name += name_suffix;

  // Try to save the file, if this fails then append retry id string and re-try.
  const int kMaxAttempts = 10;
  int attempts = 0;
  std::string16 attempts_suffix;
  while (attempts < kMaxAttempts) {
    *full_filepath = full_dirpath;
    *full_filepath += kPathSeparator;
    *full_filepath += new_name + attempts_suffix + extension;

    // Create a new file, if a file already exists this will fail
    if (File::CreateNewFile(full_filepath->c_str())) {
      return true;
    }

    // It's unlikely but possible that a file with this name already exists,
    // so we embellish the filename and re-try a few times. The filenames
    // we generate in this case are of the form name[hint][attempt].ext.
    attempts_suffix.clear();
    AppendBracketedNumber(++attempts, &attempts_suffix);
  }

  // a folder full of files that shouldn't be there, something is wrong!
#ifdef DEBUG
#if BROWSER_IE
  LOG16((L"Failed: CreateUniqueFile( %s ) = %d\n",
         full_filepath->c_str(), GetLastError()));
#endif
#endif
  assert(false);
  return false;
}

//------------------------------------------------------------------------------
// AppendBracketedNumber
//------------------------------------------------------------------------------
static void AppendBracketedNumber(int number, std::string16 *str) {
  *str += STRING16(L"[");
  *str += IntegerToString16(number);
  *str += STRING16(L"]");
}

//------------------------------------------------------------------------------
// Attempts to extract a 'filename' from a url. Typically, this is the last
// path component. However given a URL without a path component, this function
// will return the <prehost>@<host>:<port> substring. This is not undesireable
// for our use case (come up with names for cached files).
// http://www.google.com/path/filename --> filename
// http://www.google.com/path/lastcomponent/ --> lastcomponent
// http://www.google.com/path/filename?foo=bar --> filename
// http://www.google.com/path/filename#fragment --> filename
// http://www.google.com/ --> www.google.com
//------------------------------------------------------------------------------
static bool GetFileNameFromUrl(const char16 *url, std::string16 *filename) {
  filename->clear();

  std::string16 url_str(url);

  // truncate at the query or fragment divider
  size_t pos = url_str.find_first_of(STRING16(L"?#"));
  if (pos != std::string16::npos)
    url_str.resize(pos);

  if (url_str.empty())
    return false;

  // strip a trailing slash
  if (url_str[url_str.length() - 1] == '/')
    url_str.resize(url_str.length() - 1);

  // find the last slash and take what comes after
  pos = url_str.find_last_of('/');
  if (pos != std::string16::npos)
    *filename = url_str.substr(pos + 1);
  else
    *filename = url_str;

  return !filename->empty();
}
