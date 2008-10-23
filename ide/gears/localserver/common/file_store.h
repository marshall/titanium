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

#ifndef GEARS_LOCALSERVER_COMMON_FILE_STORE_H__
#define GEARS_LOCALSERVER_COMMON_FILE_STORE_H__

#include <vector>
#include "gears/localserver/common/blob_store.h"

//------------------------------------------------------------------------------
// This class is an implementation detail of WebCacheDB.
//
// A WebCacheFileStore is used by a WebCacheDB to store the response
// bodies in the file system as flat files. At this time, this only
// applies to IE, other browsers store body data in the SQLite database.
// using class WebCacheBlobStore in place of this class.
//
// The only intended client is the WebCacheDB class, so this entire class
// is marked private and the WebCacheDB is allowed access as a friend.
//------------------------------------------------------------------------------
class WebCacheFileStore : public WebCacheBlobStore {
 protected:
  // The following API is intended for use by WebCacheDB
  friend class WebCacheDB;

  // Constructs an instance
  WebCacheFileStore() : began_(false) {}

  // Initializes the file store, returns false if initialization fails
  virtual bool Init(WebCacheDB *db);

  // Inserts a body into the store
  virtual bool InsertBody(int64 server_id,
                          const char16 *url,
                          WebCacheDB::PayloadInfo *payload);

  // Reads a body from the store. If 'info_only' is true, the cached_filepath
  // member of the payload will be set, however the data vector will not be
  // populated.
  virtual bool ReadBody(WebCacheDB::PayloadInfo *payload, bool info_only);

  // Deletes a body from the store
  virtual bool DeleteBody(int64 payload_id);
  
  // Deletes all unreferenced bodies from the store
  virtual bool DeleteUnreferencedBodies();

  // Starts a transaction.
  void BeginTransaction();

  // Commits a transaction. Files and server directories that were scheduled
  // for deletion during the transaction are deleted at this time.
  void CommitTransaction();

  // Rollsback a transaction. Files and server directories that were created
  // during the transaction are deleted at this time.
  void RollbackTransaction();

  // Creates a directory to contain the files associated with server_id. Should
  // only be called with a transaction. If the transaction is rolled back,
  // the newly created directory is deleted.
  bool CreateDirectoryForServer(int64 server_id);

  // Delete the directory associated with server_id. Should only be called 
  // with a transactions. The directory is not deleted until the transaction
  // commits.
  void DeleteDirectoryForServer(int64 server_id);

 private:
  // The following methods are intended to be private to all

  // Determines the full path of the directory containing files associated
  // with server_id. Returns true if the function succeeds.
  bool GetDirectoryPathForServer(int64 server_id,
                                 std::string16 *server_dir);

  // Returns the relative filepath of the file containing the body
  // for the payload_id
  bool GetFilePath(int64 payload_id, std::string16 *filepath);

  // Reads the contents of a cached file into memory. The file to read
  // is determined by payload.cached_filepath. The file data is read into
  // a new vector and a reference to that vector is placed in payload.data
  bool ReadFile(WebCacheDB::PayloadInfo *payload);

  // Creates and writes a new cached file on disk. The filename is determined
  // by first examining the payload.headers for a Content-Disposition header
  // that contains a filename. If that fails, the filename is based on the
  // filename part of the url.  The file is created in the directory for
  // the given server_id. The relative filepath is returned in 
  // payload.cached_filepath. The contents of the file are determined by
  // payload.data. If a transaction has been started and Rollback() is
  // called prior to Commit(), the newly  created file will be deleted.
  // See BeginTransaction()
  bool CreateAndWriteFile(int64 server_id,
                          const char16 *url,
                          WebCacheDB::PayloadInfo *payload);
  
  // Deletes a cached file from disk. The delete doesn't actually occur until
  // Commit() is called. If a transaction has been started and Rollback() is
  // called prior to Commit(), the file will not be deleted.
  // See BeginTransaction()
  bool DeleteFile(const char16 *filepath);

  // Prepends the root directory path to the given relative filepath
  void PrependRootFilePath(std::string16 *filepath);

  // TODO(michaeln): A means to walk the file system removing files that are
  // not referenced in the WebCacheDB. Under normal circumstances this is not
  // needed; however if the browser process is killed in the midst of a
  // transaction, or if a file is pinned open when we attempt to delete it,
  // files can be left dangling.

  // We limit the number of files we store in a single directory. This
  // method identifies a directory with parent in its path that has space
  // available. If there is room in parent, it will be returned as available.
  void FindDirectoryWithSpaceAvailable(const char16 *parent,
                                       std::string16 *available);

  // Determines the preferred file name based on response headers and the url.
  void GetCacheFileName(const char16 *url, 
                        WebCacheDB::PayloadInfo *payload,
                        std::string16 *filename);

  std::string16 root_dir_;
  bool began_;
  std::vector<std::string16> delete_directories_on_commit_;  // full dirpaths
  std::vector<std::string16> delete_directories_on_rollback_; // full dirpaths
  std::vector<std::string16> delete_on_commit_;  // full filepaths
  std::vector<std::string16> delete_on_rollback_; // full filepaths

  DISALLOW_EVIL_CONSTRUCTORS(WebCacheFileStore);
};

#endif // GEARS_LOCALSERVER_COMMON_FILE_STORE_H__
