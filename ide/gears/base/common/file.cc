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
// The methods of the File class with a browser neutral implementation.
// Most methods implementations are browser specific and can be found
// in the file_xx.cc files.

#include <assert.h>
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread_locals.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// An arbitrary number that is a good limit on the filename length we should
// be creating internally.
const size_t File::kMaxPathComponentChars = 128;

const int64 File::kInvalidLastModifiedTime = -1;
const int64 File::kInvalidSize = -1;
const int64 File::kReadWriteFailure = -1;

const char16 *File::GetFileExtension(const char16 *filename) {
  assert(filename);
  size_t len = std::char_traits<char16>::length(filename);
  const char16* p = filename + (len - 1);
  while (p >= filename) {
    if (*p == kPathSeparator) {
      return filename + len; // Return the address of the trailing NULL
    }
    if (*p == '.') {
      return p;  // Return the address of the "."
    }
    --p;
  }
  return filename + len; // Return the address of the trailing NULL
}

bool File::GetBaseName(const std::string16 &path,  std::string16 *basename) {
  assert(basename);

  const std::string16 kPathSep(&kPathSeparator, 1);
  const std::string16 kDoublePathSep = kPathSep + kPathSep;
  std::string16 collapsed_path = path;

  size_t collapsed_path_length = collapsed_path.length();

  // Quick return for case of '', '\' & 'C:\'.
  if (collapsed_path_length <= 1) {
    *basename = collapsed_path;
#ifdef WIN32
  // Paths starting with \\\ are illegal on windows.
  } else if (collapsed_path_length >= 3 &&
             collapsed_path.find(kDoublePathSep + kPathSep) == 0) {
      return false;
#endif
  // Extract basename.
  } else {
    // Remove all trailing slashes.
    while (collapsed_path_length > 0 &&
           collapsed_path[collapsed_path_length - 1] == kPathSeparator) {
      collapsed_path.erase(collapsed_path_length - 1);
      collapsed_path_length -= 1;
    }

    // If we got here we know that the string doesn't contain multiple \s
    // isn't the root directory, and doesn't end with a \, so just do our stuff!
    size_t idx = collapsed_path.rfind(kPathSep);

    if (idx == std::string16::npos) {
      // No path separator in string.
      *basename = collapsed_path;
    } else {
      *basename = collapsed_path.substr(idx + 1);
    }

    // A corner case - if we got here it means that the input consisted entirely
    // of path separators, so return a single path separator. Note that if we
    // get an empty string as input we want to return an empty string.
    if (basename->empty() && path.length() > 0) {
      *basename = kPathSep;
    }
  }
  return true;
}

bool File::GetParentDirectory(const std::string16 &path,
                              std::string16 *parent) {
  assert(parent);

  if (path.empty()) {
    return false;
  }

  std::string16 base_name;
  if (!GetBaseName(path, &base_name)) {
    return false;
  }

  // Return false if no parent specified in path.
  int parent_length = path.length() - base_name.length() - 1;
  if (parent_length < 1) {
    return false;
  }

  std::string16 tmp_parent = path.substr(0, parent_length);

  // Clean trailing '/s' off parent.  Note that '/' is a legal value for the
  // parent directory.
  while (parent_length > 1 &&
         tmp_parent[parent_length - 1] == kPathSeparator) {
    tmp_parent.erase(parent_length - 1);
    parent_length -= 1;
  }

  *parent = tmp_parent;

  return true;
}

void File::SplitPath(const std::string16 &path,
                     PathComponents *exploded_path) {
  assert(exploded_path);

  const std::string16 path_sep(&kPathSeparator, 1);
  Tokenize(path, path_sep, exploded_path);
}

int64 File::GetFileSize(const char16 *full_filepath) {
  scoped_ptr<File> file(Open(full_filepath, READ, FAIL_IF_NOT_EXISTS));
  if (!file.get()) {
    return -1;
  }
  return file->Size();
}


int64 File::ReadFileSegmentToBuffer(const char16 *full_filepath,
                                    uint8* destination,
                                    int64 position,
                                    int64 max_bytes) {
  scoped_ptr<File> file(Open(full_filepath, READ, FAIL_IF_NOT_EXISTS));
  if (file.get() && file->Seek(position, SEEK_FROM_START)) {
    return file->Read(destination, max_bytes);
  }
  return -1;
}


bool File::ReadFileToVector(const char16 *full_filepath,
                            std::vector<uint8> *data) {
  scoped_ptr<File> file(Open(full_filepath, READ, FAIL_IF_NOT_EXISTS));
  if (file.get() == NULL) {
    return false;
  }
  int64 size = file->Size();
  if (size > data->max_size()) {
    return false;
  }
  data->resize(static_cast<unsigned int>(size));
  if (size > 0) {
    int64 read = file->Read(&(*data)[0], size);
    if (read != size) {
      data->clear();
      return false;
    }
  }
  return true;
}


bool File::WriteVectorToFile(const char16 *full_filepath,
                             const std::vector<uint8> *data) {
  const uint8 *first_byte = data->size() ? &(*data)[0] : (uint8 *)"";
  return WriteBytesToFile(full_filepath, first_byte, data->size());
}


bool File::WriteBytesToFile(const char16 *full_filepath, const uint8 *buf,
                            int length) {
  scoped_ptr<File> file(Open(full_filepath, WRITE, FAIL_IF_NOT_EXISTS));
  if (!file.get()) {
    return false;
  }
  if (!file->Truncate(0)) {
    return false;
  }
  return (file->Write(buf, length) == length);
}
