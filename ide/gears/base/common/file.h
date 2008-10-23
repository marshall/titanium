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

#ifndef GEARS_BASE_COMMON_FILE_H__
#define GEARS_BASE_COMMON_FILE_H__

#include <vector>
#include "gears/base/common/basictypes.h"
#include "gears/base/common/string16.h"

class SecurityOrigin;  // base/common/security_model.h

class File {
 public:
  // Hard Limit on path component length, used for sanity checking.
  static const size_t kMaxPathComponentChars;

  // Return value if GetLastModifiedTime fails. Its value is negative.
  static const int64 kInvalidLastModifiedTime;

  // Return value if GetFileSize fails. Its value is negative.
  static const int64 kInvalidSize;

  // Return value if Read or Write fails. Its value is negative.
  static const int64 kReadWriteFailure;

  // Creates a new file. If the file already exists, returns false.
  // Returns true if a new file has been created.
  static bool CreateNewFile(const char16 *full_filepath);

  // Returns a File object associated with a read/write temporary file
  // created in the directory returned by GetBaseTemporaryDirectory(),
  // or NULL on error. The file will be deleted automatically when the
  // returned object is destroyed. If supported by the operating
  // system, the file will not have an associated path name and cannot
  // be opened through the filesystem.
  static File *CreateNewTempFile();

  // Returns a File object associated with a read/write temporary file
  // created in the directory returned by GetBaseTemporaryDirectory(),
  // or NULL on error. The file will be deleted automatically when the
  // returned object is destroyed. The file also has an associated
  // path name which is valid for the lifetime of the object.
  static File *CreateNewNamedTempFile();

  // Creates a unique directory under the system temporary directory.  Returns
  // the full path of the new directory in 'path'.
  // Returns true if the function succeeds.  'path' is unmodified on failure.
  static bool CreateNewTempDirectory(std::string16 *full_filepath);

  // Returns true and assigns the base path where temporary files will
  // be created on success. Returns false if no platform temporary
  // file directory is available and temporary file creation will
  // fail.
  static bool GetBaseTemporaryDirectory(std::string16 *return_path);

  // Ensures all directories along the specified path exist.  Any directories
  // that do not exist will be created. Returns true if the function succeeds.
  static bool RecursivelyCreateDir(const char16 *full_dirpath);

  // Deletes a file. If the file does not exist, returns false.
  // Returns true if the file was deleted.
  static bool Delete(const char16 *full_filepath);

  // Removes the directory and all of its children. If the directory does
  // not exist, returns false. Returns true if the function succeeds.
  static bool DeleteRecursively(const char16 *full_dirpath);

  // Returns true if the file exists.
  static bool Exists(const char16 *full_filepath);

  // Returns true if the directory exists.
  static bool DirectoryExists(const char16 *full_dirpath);

  // Returns the size of the file. If the file does not exist, or is otherwise
  // unreadable, returns kInvalidSize.
  static int64 GetFileSize(const char16 *full_filepath);

  // Returns the mtime of the file (or kInvalidLastModifiedTime on failure).
  static int64 LastModifiedTime(const char16 *full_filepath);

  // Reads part of the contents of the file into memory. Returns the number of
  // bytes read (or kReadWriteFailure on failure).
  static int64 ReadFileSegmentToBuffer(const char16 *full_filepath,
                                       uint8* destination,
                                       int64 position,
                                       int64 max_bytes);

  // Reads the contents of the file into memory. If the file does not exist,
  // returns false. Returns true on success
  static bool ReadFileToVector(const char16 *full_filepath,
                               std::vector<uint8> *data);

  // Writes raw data to a file.
  // If file doesn't exist or an error occurs, false is returned.
  static bool WriteBytesToFile(const char16 *full_filepath, const uint8 *data,
                               int length);

  // Writes the contents of the file. If the file does not exist, returns false.
  // Existing contents are overwritten. Returns true on success
  static bool WriteVectorToFile(const char16 *full_filepath,
                                const std::vector<uint8> *data);

  // Returns the number of files and directories contained in the given
  // directory. If the directory does not exist, returns 0.
  static int GetDirectoryFileCount(const char16 *full_dirpath);

  // Returns a pointer to the last '.' within 'filename' if an extension is
  // found, or a pointer to the trailing NULL otherwise.
  static const char16 *GetFileExtension(const char16 *filename);

  // The basename (last path component) for the given path is returned in the
  // basename parameter.
  //
  // Example usage is splitting the filename off the end of a path for the
  // purpose of sanity checking.
  // Edge case: if a Windows \\share path is used, 'share' will be returned
  //  and not '\\share' as may be expected.
  //
  // Returns true if function succeeds.
  static bool GetBaseName(const std::string16 &path,  std::string16 *basename);

  // Gets the parent directory for a path.
  // Corner cases:
  // * 'path' parameter may not be empty.
  // * Fails if path doesn't specify a parent directory e.g. 'filename'.
  static bool GetParentDirectory(const std::string16 &path,
                                 std::string16 *parent);

  enum OpenAccessMode { READ, WRITE, READ_WRITE };

  enum OpenExistsMode { NEVER_FAIL, FAIL_IF_NOT_EXISTS, FAIL_IF_EXISTS };

  // Opens a file for reading, writing, or both. Optionally fail if the file
  // already exists or does not already exist. Files opened for read and write
  // must seek between read and write operations.
  // Returns NULL if there is an error opening the file, or if the
  // constraints of access_mode or exists_mode can't be met.
  static File *Open(const char16 *full_filepath, OpenAccessMode access_mode,
                    OpenExistsMode exists_mode);

#ifdef WIN32
  // Prepend long path prefix, "\\?\", onto 'path' so we can handle filepaths
  // longer than 256 characters. This function does nothing if the prefix
  // already exists on 'path'.
  static std::string16 File::ToLongPath(const std::string16 &path);
#endif

  // Reads at most max_bytes of file data at the current seek position into
  // destination.
  // Returns the number of bytes read, or kReadWriteFailure on failure or if
  // the file is not opened for read.
  int64 Read(uint8 *destination, int64 max_bytes) const;

  // Writes length bytes of source to the file.
  // Returns the number of bytes written, or kReadWriteFailure on failure or if
  // the file is not opened for write.
  int64 Write(const uint8 *source, int64 length);

  // Forces a write of all buffered data to the file on disk.
  bool Flush();

  enum SeekMethod { SEEK_FROM_START, SEEK_FROM_CURRENT, SEEK_FROM_END };

  // Sets the file position indicator to the specified offset.
  // Returns false if there is an error.
  bool Seek(int64 offset, SeekMethod seek_method = SEEK_FROM_START) const;

  // Returns the current position in the file, or -1 on failure.
  int64 Tell() const;

  // Returns the size of the file, or -1 on failure.
  int64 Size() const;

  // Truncates the file to the specified length.
  // Returns false if there is an error or if the file is not opened for write.
  bool Truncate(int64 length);

  const std::string16 &GetFilePath() const { return file_path_; }

  ~File();

 private:
  // Used by SplitPath
  typedef std::vector<std::string16> PathComponents;

  // Splits a path into it's individual Components, on windows if a drive
  // letter is part of the path, then this is the first item in the list e.g.
  // 'c:\a.txt' -> ['c:', 'a.txt']
  //
  // This function doesn't handle file shares on Win32, caller must strip them 
  // out before calling this function.
  //
  // Multiple consecutive path separators are expanded into empty strings, e.g.
  // 'c:\\a.txt' -> ['c:', '', 'a.txt'] - '\\' becomes ''.
  static void SplitPath(const std::string16 &path, 
                        PathComponents *exploded_path);

  // Test friends :)
  friend bool TestSplitPath(std::string16 *error);

  // TODO(miket): someone fix common.h so that it doesn't require a browser
  // flag to be defined! Or better yet, someone shoot common.h in the head!
  //  DISALLOW_EVIL_CONSTRUCTORS(File);

  // Closes the open file handle. Should only be called by the destructor.
  void Close();

  // NOTE: handle_ is _not_ automatically closed. Currently it is only ever set
  //       on construction, and is never modified until destruction. If this
  //       ever changes, care must be taken to close old handles before they
  //       change.
#ifdef WIN32
  HANDLE handle_;
  explicit File(const char16 *path)
      : handle_(NULL), auto_delete_(false), file_path_(path) {}
#else
// Currently all non-win32 targets are POSIX compliant.
  FILE *handle_;
  explicit File(const char16 *path)
      : handle_(NULL), auto_delete_(false), file_path_(path) {}
#endif
  bool auto_delete_;
  OpenAccessMode mode_;
  std::string16 file_path_;
};


#endif  // GEARS_BASE_COMMON_FILE_H__
