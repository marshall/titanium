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
// The methods of the File class implemented for use in Windows CE.
//
// Note that this file completely ignores long pathnames, an alternate solution
// for this needs to be found.
//
// Some methods implementations are browser neutral and can be found
// in file.cc.

#ifdef WINCE
#include <assert.h>
#include <limits>
#include <windows.h>
#include <shlobj.h>  // Must include windows.h before this file.
#include "gears/base/common/basictypes.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/scoped_win32_handles.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/wince_compatibility.h"
#include "genfiles/product_constants.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

static const DWORD FILE_SHARE_ALL(FILE_SHARE_READ | FILE_SHARE_WRITE);

File::~File() {
  Close();
  if (auto_delete_) {
    Delete(file_path_.c_str());
  }
}


void File::Close() {
  if (handle_ != INVALID_HANDLE_VALUE) {
    ::CloseHandle(handle_);
    handle_ = INVALID_HANDLE_VALUE;
  }
}


bool File::Flush() {
  return ::FlushFileBuffers(handle_) != 0;
}


File *File::Open(const char16 *full_filepath, OpenAccessMode access_mode,
                 OpenExistsMode exists_mode) {
  scoped_ptr<File> file(new File(full_filepath));
  DWORD desired_access = 0;
  switch (access_mode) {
    case READ:
      desired_access = GENERIC_READ;
      break;
    case WRITE:
      desired_access = GENERIC_WRITE;
      break;
    case READ_WRITE:
      desired_access = GENERIC_READ | GENERIC_WRITE;
      break;
  }
  DWORD creation_disposition = 0;
  switch (exists_mode) {
    case NEVER_FAIL:
      // OPEN_ALWAYS creates nonexistent files which is not desired in read mode
      creation_disposition = (access_mode == READ) ?
          OPEN_EXISTING : OPEN_ALWAYS;
      break;
    case FAIL_IF_NOT_EXISTS:
      creation_disposition = OPEN_EXISTING;
      break;
    case FAIL_IF_EXISTS:
      // CREATE_NEW creates nonexistent files, which is not desired in read mode
      if (access_mode == READ) {
        return NULL;
      }
      creation_disposition = CREATE_NEW;
      break;
  }
  file->mode_ = access_mode;
  file->handle_ = ::CreateFileW(full_filepath,
                                desired_access,
                                FILE_SHARE_ALL,
                                NULL,
                                creation_disposition,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
  if (file->handle_ == INVALID_HANDLE_VALUE) {
    return NULL;
  }
  return file.release();
}


int64 File::Read(uint8* destination, int64 max_bytes) const {
  if (!destination || max_bytes < 0) {
    return kReadWriteFailure;
  }

  // Read its contents into memory.
  if (max_bytes > std::numeric_limits<DWORD>::max()) {  // ReadFile limit
    max_bytes = std::numeric_limits<DWORD>::max();
  }
  DWORD bytes_read;
  if (!::ReadFile(handle_, destination,
                  static_cast<DWORD>(max_bytes), &bytes_read, NULL)) {
    return kReadWriteFailure;
  }

  return bytes_read;
}


bool File::Seek(int64 offset, SeekMethod seek_method) const {
  DWORD move_method = 0;
  switch (seek_method) {
    case SEEK_FROM_START:
      move_method = FILE_BEGIN;
      break;
    case SEEK_FROM_CURRENT:
      move_method = FILE_CURRENT;
      break;
    case SEEK_FROM_END:
      move_method = FILE_END;
      break;
  }

  LARGE_INTEGER pos;
  pos.QuadPart = offset;
  pos.LowPart = SetFilePointer(handle_, pos.LowPart, &pos.HighPart,
                               move_method);

  return (pos.LowPart != 0xFFFFFFFF || GetLastError() == NO_ERROR);
}


int64 File::Size() const {
  LARGE_INTEGER size;
  size.LowPart = ::GetFileSize(handle_,
                               reinterpret_cast<LPDWORD>(&size.HighPart));
  if (size.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
    return kInvalidSize;
  }
  return size.QuadPart;
}


int64 File::Tell() const {
  LARGE_INTEGER pos;
  pos.QuadPart = 0;
  pos.LowPart = SetFilePointer(handle_, pos.LowPart, &pos.HighPart,
                               FILE_CURRENT);

  if (pos.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
    return kInvalidSize;
  }
  return pos.QuadPart;
}


bool File::Truncate(int64 length) {
  if (length < 0) {
    return false;
  }
  int64 pos = Tell();
  if (!Seek(length, SEEK_FROM_START)) {
    return false;
  }
  bool success = (SetEndOfFile(handle_) != FALSE);
  // try to seek back, even if truncate failed
  Seek(pos, SEEK_FROM_START);
  // TODO(fry): return false if Seek fails?
  return success;
}


int64 File::Write(const uint8 *source, int64 length) {
  if (mode_ == READ) {
    // NOTE: WriteFile doesn't fail after opening in READ mode
    return kReadWriteFailure;
  }
  if (!source || length < 0) {
    return kReadWriteFailure;
  }
  // NOTE: disallow DWORD overflows since they won't fit in memory anyway
  assert(length < std::numeric_limits<DWORD>::max());
  DWORD data_size = static_cast<DWORD>(length);
  DWORD bytes_written;
  return ::WriteFile(handle_, source, data_size, &bytes_written, NULL)
      ? bytes_written : kReadWriteFailure;
}


bool File::CreateNewFile(const char16 *full_filepath) {
  // Create a new file, if a file already exists this will fail
  SAFE_HANDLE safe_file_handle(::CreateFileW(full_filepath,
                                             GENERIC_WRITE,
                                             FILE_SHARE_ALL,
                                             NULL,
                                             CREATE_NEW,
                                             FILE_ATTRIBUTE_NORMAL,
                                             NULL));
  if (safe_file_handle.get() == INVALID_HANDLE_VALUE) {
    return false;
  }
  return true;
}


bool File::Delete(const char16 *full_filepath) {
  return ::DeleteFileW(full_filepath) ? true : false;
}


bool File::Exists(const char16 *full_filepath) {
  DWORD attrs = GetFileAttributesW(full_filepath);
  return (attrs != INVALID_FILE_ATTRIBUTES) &&
         ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
}


bool File::DirectoryExists(const char16 *full_dirpath) {
  DWORD attrs = GetFileAttributesW(full_dirpath);
  return (attrs != INVALID_FILE_ATTRIBUTES) &&
         ((attrs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
}


int64 File::LastModifiedTime(const char16 *full_filepath) {
  SAFE_HANDLE file_handle(CreateFileW(full_filepath, GENERIC_READ,
                                      FILE_SHARE_ALL,
                                      NULL, OPEN_EXISTING, 0, NULL));
  if (file_handle.get() == INVALID_HANDLE_VALUE) {
    return kInvalidLastModifiedTime;
  }

  FILETIME filetime;
  if (!GetFileTime(file_handle.get(), NULL, NULL, &filetime)) {
    return kInvalidLastModifiedTime;
  }
  return (static_cast<int64>(filetime.dwHighDateTime) << 32) |
      (static_cast<int64>(filetime.dwLowDateTime) & 0xFFFFFFFF);
}


int File::GetDirectoryFileCount(const char16 *full_dirpath) {
  std::string16 find_spec(full_dirpath);
  find_spec += L"\\*";
  WIN32_FIND_DATA find_data;
  HANDLE find_handle = FindFirstFile(find_spec.c_str(), &find_data);
  if (find_handle == INVALID_HANDLE_VALUE) {
    return 0;  // expected if the directory does not exist
  }
  int count = 0;
  do {
    if ((wcscmp(find_data.cFileName, L"..") == 0) ||
        (wcscmp(find_data.cFileName, L".") == 0)) {
      continue;  // don't count parent and current directories
    }
    ++count;
  } while (FindNextFile(find_handle, &find_data) != 0);
  FindClose(find_handle);
  return count;
}

bool CreateNewTempFileName(std::string16 *path) {
  static const char16 *kTempFilePrefix = STRING16(PRODUCT_SHORT_NAME);

  // Get the system temp directory.
  std::string16 temp_dir;
  if (!File::GetBaseTemporaryDirectory(&temp_dir)) {
    return false;
  }

  // Create a uniquely named temp file in that directory.
  // Note: GetTempFileName() uses 3 chars max of the suggested prefix
  wchar_t file[MAX_PATH];
  UINT id = GetTempFileNameW(temp_dir.c_str(), kTempFilePrefix, 0, file);
  if (0 == id) {
    return false;
  }
  (*path) = file;
  return true;
}

File *File::CreateNewTempFile() {
  // Can't create unnamed temporary files on Win32.
  return CreateNewNamedTempFile();
}

File *File::CreateNewNamedTempFile() {
  std::string16 filename;
  if (!CreateNewTempFileName(&filename)) {
    return NULL;
  }
  scoped_ptr<File> file(Open(filename.c_str(), READ_WRITE, FAIL_IF_NOT_EXISTS));
  if (file.get()) {
    file->auto_delete_ = true;
  }
  return file.release();
}


bool File::CreateNewTempDirectory(std::string16 *path) {
  std::string16 temp;  // to avoid modifying 'path' if something fails
  if (!CreateNewTempFileName(&temp)) {
    return false;
  }

  // Delete that file, and create a directory with the same name,
  // now that we know it's unique.
  if (0 == ::DeleteFileW(temp.c_str())) {
    return false;
  }
  if (0 == ::CreateDirectoryW(temp.c_str(), NULL)) {
    return false;
  }
  (*path) = temp;
  return true;
}


bool File::GetBaseTemporaryDirectory(std::string16 *path) {
  // Get the system temp directory.
  wchar_t root[MAX_PATH];
  DWORD chars = ::GetTempPathW(MAX_PATH, root);
  if (chars >= MAX_PATH) {
    return false;
  }
  *path = root;
  return true;
}

bool File::RecursivelyCreateDir(const char16 *full_dirpath) {
  // Note: SHCreateDirectoryEx is available in shell32.dll version 5.0+,
  // which means Win2K/XP and higher, plus WinME.
  // For Windows Mobile, we implement it in wince_compatibility.cc.
  int r = SHCreateDirectoryEx(NULL,   // parent HWND, if UI desired
                              full_dirpath,
                              NULL);  // security attributes for new folders
  if (r != ERROR_SUCCESS &&
      r != ERROR_FILE_EXISTS && r != ERROR_ALREADY_EXISTS) {
    return false;
  } else if (r == ERROR_ALREADY_EXISTS && File::Exists(full_dirpath)) {
    return false;
  }

  return true;
}

bool File::DeleteRecursively(const char16 *full_dirpath) {
  std::string16 delete_op_path(full_dirpath);
  delete_op_path += L'\0';  // SHFileOperation needs double null termination

  SHFILEOPSTRUCTW fileop = {0};
  fileop.wFunc = FO_DELETE;
  fileop.pFrom = delete_op_path.c_str();
  fileop.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
#ifdef WINCE
  // FOF_NOERRORUI is not defined in Windows Mobile.
#else
  fileop.fFlags |= FOF_NOERRORUI;
#endif
  return (SHFileOperationW(&fileop) == 0);
}
#endif  // WINCE
