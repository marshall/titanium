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
//
// Implementation of the File class for use on POSIX compliant platforms.
// Some methods implementations are browser neutral and can be found
// in file.cc.

#ifdef WIN32
// Currently all non-win32 gears targets are POSIX compliant.
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <vector>
#include "gears/base/common/android_compatibility.h"
#include "gears/base/common/basictypes.h"
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "genfiles/product_constants.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// paths.h defines a char16 version of this, but we use UTF8 internally here
// and POSIX systems are consistent in this regard.
static const char kPathSeparatorUTF8 = '/';

// Template used by mkstemp()/mkdtemp(). This must end in 6 'X'
// characters which are replaced to make a unique filename.
static const char16 *const kTemporaryFileTemplate =
    STRING16(PRODUCT_SHORT_NAME L"TempXXXXXX");

// Wraps filename, "is file a directory?" pair.
class DirEntry : public std::pair<std::string, bool> {
 public:
  DirEntry(const std::string &filename, bool is_dir) :
      std::pair<std::string,bool>(filename, is_dir) {}

  const std::string &Filename() const { return first; }
  bool IsDirectory() const { return second; }
};
typedef std::vector<DirEntry> DirContentsVector;
typedef DirContentsVector::const_iterator DirContentsVectorConstIterator;

File::~File() {
  Close();
  if (auto_delete_) {
    Delete(file_path_.c_str());
  }
}

void File::Close() {
  if (handle_) {
    fclose(handle_);
    handle_ = NULL;
  }
}

bool File::Flush() {
  return !fflush(handle_);
}

File *File::Open(const char16 *full_filepath, OpenAccessMode access_mode,
                 OpenExistsMode exists_mode) {
  scoped_ptr<File> file(new File(full_filepath));
  std::string file_path_utf8_;
  if (!String16ToUTF8(full_filepath, &file_path_utf8_)) {
    return false;
  }

  const char *mode = NULL;
  switch (exists_mode) {
    case NEVER_FAIL:
      break;
    case FAIL_IF_NOT_EXISTS:
      if (!File::Exists(full_filepath)) {
        return NULL;
      }
      break;
    case FAIL_IF_EXISTS:
      if (File::Exists(full_filepath)) {
        return NULL;
      }
      break;
  }
  switch (access_mode) {
    case READ:
      mode = "rb";
      break;
    case WRITE:
      // NOTE: Read will fail when opened for WRITE
    case READ_WRITE:
      // NOTE: rb+ does not create nonexistent files, and w+ truncates them
      mode = File::Exists(full_filepath) ? "rb+" : "w+";
      break;
  }
  file->mode_ = access_mode;
  file->handle_ = fopen(file_path_utf8_.c_str(), mode);
  if (file->handle_ == NULL) {
    return NULL;
  }
  return file.release();
}

int64 File::Read(uint8* destination, int64 max_bytes) const {
  if (mode_ == WRITE) {
    // NOTE: we may have opened the file with read-write access to avoid
    // truncating it, but we still want to refuse reads
    return kReadWriteFailure;
  }
  if (!destination || max_bytes < 0) {
    return kReadWriteFailure;
  }

  // Read its contents into memory.
  if (max_bytes > std::numeric_limits<size_t>::max()) {  // fread limit
    max_bytes = std::numeric_limits<size_t>::max();
  }
  size_t bytes_read = fread(destination, 1, static_cast<size_t>(max_bytes),
                            handle_);
  if (ferror(handle_) && !feof(handle_)) {
    return kReadWriteFailure;
  }
  return bytes_read;
}

bool File::Seek(int64 offset, SeekMethod seek_method) const {
  int whence = 0;
  switch (seek_method) {
    case SEEK_FROM_START:
      whence = SEEK_SET;
      break;
    case SEEK_FROM_CURRENT:
      whence = SEEK_CUR;
      break;
    case SEEK_FROM_END:
      whence = SEEK_END;
      break;
  }

  // handle 64-bit seek for 32-bit off_t
  while (offset > std::numeric_limits<off_t>::max()) {
    if (fseeko(handle_, std::numeric_limits<off_t>::max(), whence) != 0) {
      return false;
    }
    offset -= std::numeric_limits<off_t>::max();
    whence = SEEK_CUR;
  }
  while (offset < std::numeric_limits<off_t>::min()) {
    if (fseeko(handle_, std::numeric_limits<off_t>::min(), whence) != 0) {
      return false;
    }
    offset -= std::numeric_limits<off_t>::min();
    whence = SEEK_CUR;
  }

  return (fseeko(handle_, static_cast<long>(offset), whence) == 0);
}

int64 File::Size() const {
  struct stat stat_data;
  if (fstat(fileno(handle_), &stat_data) != 0) {
    return kInvalidSize;
  }
  return static_cast<int64>(stat_data.st_size);
}

int64 File::Tell() const {
  return ftello(handle_);
}

bool File::Truncate(int64 length) {
  if (length < 0) {
    return false;
  }
  return ftruncate(fileno(handle_), length) == 0;
}

int64 File::Write(const uint8 *source, int64 length) {
  if (mode_ == READ) {
    // NOTE: fwrite doesn't fail after opening in READ mode
    return kReadWriteFailure;
  }
  if (!source || length < 0) {
    return kReadWriteFailure;
  }
  // can't write more data than fwrite can handle
  assert(length <= std::numeric_limits<size_t>::max());

  size_t bytes_written = fwrite(source, 1, length, handle_);
  if (ferror(handle_)) {
    return kReadWriteFailure;
  }
  return bytes_written;
}

// Places the direct contents of the 'path' directory into results.  This
// method is not recursive.
// 'results' may not be NULL.
//
// Returns false on error in which case 'results' isn't modified.
static bool ReadDir(const std::string16 &path, DirContentsVector *results) {
  std::string path_utf8;
  if (!String16ToUTF8(path.c_str(), &path_utf8)) {
    return false;
  }
  DIR *the_dir = opendir(path_utf8.c_str());
  if (the_dir == NULL) {
    return false;
  }

  DirContentsVector local_dir_contents;
  bool error_reading_dir_contents = false;

  // Zero errno - as a NULL return value from readdir() can mean we're done
  // reading a directory OR that an error has occured, so our only way of
  // knowing the difference is via errno.
  // The need to do this arose from a unit test failing - readdir() does not
  // zero errno itself (at least under OSX).
  //
  // To clarify, the error case we're solving here, is when we reach this point
  // with errno != 0.  Then when readdir() returns NULL inside the loop to
  // signify that it's finished reading the directory - when we check errno we
  // will find a non-zero value and mistakenly think that readdir() has failed.
  //
  // We do not need to move this inside the loop, because no change will be
  // made to errno inside the loop if there are no errors, and if there are
  // we want to bail immediately.
  errno = 0;
  while (true) {
    struct dirent *dir_info = readdir(the_dir);
    if (dir_info == NULL) {
      if (errno != 0) {
        error_reading_dir_contents = true;
      }
      break;  // Reached end of directory contents.
    }
    if ((strcmp(dir_info->d_name, "..") == 0) ||
        (strcmp(dir_info->d_name, ".") == 0)) {
      continue;  // Skip parent and current directories.
    }

    std::string filename(dir_info->d_name);
    bool is_dir = (dir_info->d_type == DT_DIR);
    local_dir_contents.push_back(DirEntry(filename, is_dir));
  }

  if (closedir(the_dir) != 0) {
    return false;
  }

  if (error_reading_dir_contents) {
    return false;
  }
  results->swap(local_dir_contents);
  return true;
}

bool File::CreateNewFile(const char16 *path) {
  // TODO(fry): implement using File in file.cc
  std::string path_utf8;
  if (!String16ToUTF8(path, &path_utf8)) {
    return false;
  }

  // Create new file with permission 0600, fail if the file already exists.
  int fd = open(path_utf8.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    return false;
  }

  if (close(fd) != 0) {
    return false;
  }
  return true;
}

static bool StatFile(const char16 *path, struct stat *stat_data) {
  std::string path_utf8;
  if (!String16ToUTF8(path, &path_utf8)) {
    return false;
  }
  struct stat tmp;
  if (stat(path_utf8.c_str(), &tmp) != 0) {
    return false;
  }
  *stat_data = tmp;
  return true;
}

bool File::Exists(const char16 *full_filepath) {
  struct stat stat_data;
  if (!StatFile(full_filepath, &stat_data)) {
    return false;
  }

  return S_ISREG(stat_data.st_mode);
}

bool File::DirectoryExists(const char16 *full_dirpath) {
  struct stat stat_data;
  if (!StatFile(full_dirpath, &stat_data)) {
    return false;
  }

  return S_ISDIR(stat_data.st_mode);
}

int64 File::LastModifiedTime(const char16 *full_filepath) {
  struct stat stat_data;
  if (!StatFile(full_filepath, &stat_data)) {
    return kInvalidLastModifiedTime;
  }
  return stat_data.st_mtime;
}

bool File::Delete(const char16 *full_filepath) {
  std::string path_utf8;
  if (!String16ToUTF8(full_filepath, &path_utf8)) {
    return false;
  }

  return unlink(path_utf8.c_str()) == 0;
}

int File::GetDirectoryFileCount(const char16 *full_dirpath) {
  std::string16 the_dir(full_dirpath);
  DirContentsVector dir_contents;
  bool success = ReadDir(the_dir, &dir_contents);
  if (!success) {
    return 0;
  } else {
    return dir_contents.size();
  }
}

File *File::CreateNewTempFile() {
  // Create a named temporary file.
  scoped_ptr<File> file(CreateNewNamedTempFile());
  if (!file.get()) {
    return NULL;
  }
  // Unlink it from the filesystem so it becomes unnamed.
  std::string16 path = file->GetFilePath();
  if (unlink(String16ToUTF8(path).c_str()) != 0) {
    LOG(("Couldn't make temporary file \"%s\" unnamed\n",
         String16ToUTF8(path).c_str()));
    return NULL;
  }
  // Clear the path and turn off auto deletion.
  file->file_path_ = std::string16();
  file->auto_delete_ = false;
  return file.release();
}

File *File::CreateNewNamedTempFile() {
  // Get the base temporary file directory.
  std::string16 path;
  if (!GetBaseTemporaryDirectory(&path)) {
    return NULL;
  }
  // Append a temporary file template.
  path += kPathSeparator;
  path += kTemporaryFileTemplate;
  std::string path8;
  if (!String16ToUTF8(path.c_str(), &path8)) {
    LOG(("Bad temporary directory encoding\n"));
    return NULL;
  }
  // We need a non-const string to modify with mkstemp(), in UTF-8.
  scoped_array<char> template_path(new char[path8.size() + 1]);
  memcpy(template_path.get(), path8.c_str(), path8.size() + 1);
  // Atomically create a temporary file with the template.
  int fd = mkstemp(template_path.get());
  if (fd < 0) {
    LOG(("Failed to create temp with template \"%s\"\n", template_path.get()));
    return NULL;
  }
  // And bounce back to UTF-16 again...
  if (!UTF8ToString16(template_path.get(), &path)) {
    LOG(("Bad encoding in template result \"%s\"\n", template_path.get()));
    return NULL;
  }
  // Create a File using the handle.
  scoped_ptr<File> file(new File(path.c_str()));
  file->mode_ = READ_WRITE;
  file->handle_ = fdopen(fd, "rb+");
  if (file->handle_ == NULL) {
    close(fd);
    return NULL;
  }
  LOG(("Created temporary file \"%s\"\n", template_path.get()));
  // Delete when this instance is destructed.
  file->auto_delete_ = true;
  return file.release();
}

bool File::CreateNewTempDirectory(std::string16 *full_filepath) {
  std::string16 path;
  if (!GetBaseTemporaryDirectory(&path)) {
    return false;
  }
  // Append a directory template and ensure it exists.
  path += kPathSeparator;
  path += kTemporaryFileTemplate;
  std::string path8;
  if (!String16ToUTF8(path.c_str(), &path8)) {
    LOG(("Bad temporary directory encoding\n"));
    return false;
  }
  // We need a non-const string to modify with mkdtemp(), in UTF-8.
  scoped_array<char> template_path(new char[path8.size() + 1]);
  memcpy(template_path.get(), path8.c_str(), path8.size() + 1);
  // mkdtemp() creates the directory with permissions set to 0700.
  if (mkdtemp(template_path.get()) == NULL) {
    return false;
  }
  // And bounce back to UTF-16 again...
  if (!UTF8ToString16(template_path.get(), &path)) {
    return false;
  }
  LOG(("Created temporary directory \"%s\"\n", template_path.get()));
  full_filepath->swap(path);
  return true;
}

#if defined(OS_ANDROID)
bool File::GetBaseTemporaryDirectory(std::string16 *return_path) {
  // Create a temporary file in a specific application directory as
  // /tmp goes to ramdisk.
  std::string16 path;
  // Get the base data directory.
  if (!GetBaseDataDirectory(&path)) {
    LOG(("Couldn't get base directory\n"));
    return false;
  }
  // Append a temporary directory under the base directory.
  path += kPathSeparator;
  path += STRING16(L"temp");
  // Ensure the directory exists.
  if (!RecursivelyCreateDir(path.c_str())) {
    LOG(("Couldn't create temporary directory \"%s\"\n",
         String16ToUTF8(path).c_str()));
    return false;
  }
  *return_path = path;
  return true;
}
#elif defined(BROWSER_WEBKIT)
bool File::GetBaseTemporaryDirectory(std::string16 *return_path) {
  // OSX has a per-user temporary directory.
  return GetUserTempDirectory(return_path);
}
#else // Other POSIX system.
bool File::GetBaseTemporaryDirectory(std::string16 *return_path) {
  // stdio.h defines P_tmpdir as the location for tmpfile(). This is
  // usually "/tmp".
  std::string16 path;
  if (UTF8ToString16(P_tmpdir, &path)) {
    if (File::DirectoryExists(path.c_str())) {
      *return_path = path;
      return true;
    } else {
      LOG(("Temporary directory \"%s\" doesn't exist\n", P_tmpdir));
      return false;
    }
  } else {
    LOG(("Bad encoding of P_tmpdir \"%s\"\n", P_tmpdir));
    assert(false);
    return false;
  }
}
#endif

bool File::RecursivelyCreateDir(const char16 *full_dirpath) {
   // If directory already exists, no need to do anything.
  if(File::DirectoryExists(full_dirpath)) {
    return true;
  }

  File::PathComponents path_components;
  File::SplitPath(std::string16(full_dirpath), &path_components);

  std::string16 long_path;
  // Recursively create directories.
  for (File::PathComponents::const_iterator it = path_components.begin();
       it != path_components.end();
       ++it) {
    // '//', '.' & '..' shouldn't be present in the path, but if they are fail
    // hard!
    if (it->empty() || *it == STRING16(L".") || *it == STRING16(L"..")) {
      assert("Badly formed pathname" == NULL);
      return false;
    }

    long_path = long_path + kPathSeparator + *it;

    std::string path_utf8;
    if (!String16ToUTF8(long_path.c_str(), &path_utf8)) {
      return false;
    }

    // Create directory with permissions set to 0700.
    if (mkdir(path_utf8.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
      // Any error value other than "directory already exists" is considered
      // fatal.
      if (errno != EEXIST) {
        return false;
      } else if (!File::DirectoryExists(long_path.c_str())) {
        return false; // We've collided with a file having the same name.
      }
    }
  }

  return true;
}

// Recursive function for use by DeleteRecursively.
static bool DeleteRecursivelyImpl(const std::string &del_path) {
  std::string16 del_path_utf16;
  if (!UTF8ToString16(del_path.c_str(), &del_path_utf16)) {
    return false;
  }
  DirContentsVector dir_contents;
  if (!ReadDir(del_path_utf16, &dir_contents)) {
    return false;
  }

  for (DirContentsVectorConstIterator it = dir_contents.begin();
       it != dir_contents.end();
       ++it) {
    std::string path_component_to_delete = del_path + kPathSeparatorUTF8 +
                                           it->Filename();
    if (it->IsDirectory()) {
      if (!DeleteRecursivelyImpl(path_component_to_delete)) {
        return false;
      }
    } else {
      if (unlink(path_component_to_delete.c_str()) != 0) {
        return false;
      }
    }
  }

  if (rmdir(del_path.c_str()) != 0) {
    return false;
  }

  return true;
}

bool File::DeleteRecursively(const char16 *full_dirpath) {
  std::string dir_to_delete;
  if (!String16ToUTF8(full_dirpath, &dir_to_delete)) {
    return false;
  }

  // We can only operate on a directory.
  if(!File::DirectoryExists(full_dirpath)) {
    return false;
  }

  // Cut off trailing slash from directory name if any.
  std::string path_sep(&kPathSeparatorUTF8, 1);
  if (EndsWith(dir_to_delete, path_sep)) {
    dir_to_delete.erase(dir_to_delete.end()-1);
  }

  return DeleteRecursivelyImpl(dir_to_delete);
}

#endif  // !WIN32
