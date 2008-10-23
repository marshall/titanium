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

#include <vector>
#include "gears/base/common/common.h"
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"
#endif
#include "third_party/scoped_ptr/scoped_ptr.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOCATION __FILE__ ", line " TOSTRING(__LINE__)
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("failed at " LOCATION)); \
    assert(error); \
    if (!error->empty()) *error += STRING16(L", "); \
    *error += STRING16(L"failed at "); \
    std::string16 location; \
    UTF8ToString16(LOCATION, &location); \
    *error += location; \
    return false; \
  } \
}

//------------------------------------------------------------------------------
// TestFileUtils
//------------------------------------------------------------------------------
bool TestSplitPath(std::string16 *error); //friend of file.h
static bool TestGetBaseName(std::string16 *error);
static bool TestGetParentDirectory(std::string16 *error);
static bool TestLongPaths(std::string16 *error);
static bool TestFileObject(std::string16 *error);
static bool CheckDirectoryCreation(const char16 *dir);
static bool TestMultipleOpen(std::string16 *error);

bool TestFileUtils(std::string16 *error) {

  //Run tests for individual functions
  TEST_ASSERT(TestGetBaseName(error));
  TEST_ASSERT(TestGetParentDirectory(error));
  TEST_ASSERT(TestSplitPath(error));
  TEST_ASSERT(TestLongPaths(error));
  TEST_ASSERT(TestFileObject(error));
  TEST_ASSERT(TestMultipleOpen(error));

  // Create a new empty directory work with
  std::string16 temp_dir;
  TEST_ASSERT(File::CreateNewTempDirectory(&temp_dir));
  TEST_ASSERT(File::DirectoryExists(temp_dir.c_str()));
  TEST_ASSERT(File::GetDirectoryFileCount(temp_dir.c_str()) == 0);

  const char16 *kFileName = STRING16(L"File.ext");
  std::string16 filepath(temp_dir);
  filepath += kPathSeparator;
  filepath += kFileName;

  // Get the files extension
  std::string16 ext(File::GetFileExtension(filepath.c_str()));
  TEST_ASSERT(ext == STRING16(L".ext"));

  // Create a file, test existence, and make sure we fail if we try to
  // recreate an already existing file
  TEST_ASSERT(!File::Exists(filepath.c_str()));
  TEST_ASSERT(File::CreateNewFile(filepath.c_str()));
  TEST_ASSERT(File::Exists(filepath.c_str()));
  TEST_ASSERT(!File::CreateNewFile(filepath.c_str()));

  // See that the directory contains one item
  TEST_ASSERT(File::GetDirectoryFileCount(temp_dir.c_str()) == 1);

  // Write some data, then read it back and compare
  uint8 stack_fuzz[4096];
  std::vector<uint8> data_orig;
  std::vector<uint8> data_read;
  data_orig.assign(stack_fuzz, stack_fuzz + ARRAYSIZE(stack_fuzz));
  TEST_ASSERT(File::WriteVectorToFile(filepath.c_str(), &data_orig));
  TEST_ASSERT(File::ReadFileToVector(filepath.c_str(), &data_read));
  TEST_ASSERT(data_orig.size() == data_read.size());
  TEST_ASSERT(memcmp(&data_orig[0], &data_read[0], data_orig.size()) == 0);

  // Overwrite what was written above
  data_orig.assign(1024, 0xAA);
  data_read.clear();
  TEST_ASSERT(File::WriteVectorToFile(filepath.c_str(), &data_orig));
  TEST_ASSERT(File::ReadFileToVector(filepath.c_str(), &data_read));
  TEST_ASSERT(data_orig.size() == data_read.size());
  TEST_ASSERT(memcmp(&data_orig[0], &data_read[0], data_orig.size()) == 0);

  // Overwrite again with a zero length file
  data_orig.clear();
  data_read.clear();
  TEST_ASSERT(File::WriteVectorToFile(filepath.c_str(), &data_orig));
  TEST_ASSERT(File::ReadFileToVector(filepath.c_str(), &data_read));
  TEST_ASSERT(data_orig.size() == data_read.size());
  TEST_ASSERT(data_orig.size() == 0);

  // Delete the file, test that it is deleted, and make sure we fail if we try
  // to delete a file that doesn't exist
  TEST_ASSERT(File::Delete(filepath.c_str()));
  TEST_ASSERT(!File::Exists(filepath.c_str()));
  TEST_ASSERT(!File::Delete(filepath.c_str()));

  // See the the directory now contains no items
  TEST_ASSERT(File::GetDirectoryFileCount(temp_dir.c_str()) == 0);

  // We should not be able to read or write a file that doesn't exist
  TEST_ASSERT(!File::WriteVectorToFile(filepath.c_str(), &data_orig));
  TEST_ASSERT(!File::ReadFileToVector(filepath.c_str(), &data_read));

  // Create a sub-directory
  std::string16 sub_dir(temp_dir);
  sub_dir += kPathSeparator;
  sub_dir += STRING16(L"sub");
  TEST_ASSERT(!File::DirectoryExists(sub_dir.c_str()));
  TEST_ASSERT(File::GetDirectoryFileCount(sub_dir.c_str()) == 0);
  TEST_ASSERT(CheckDirectoryCreation(sub_dir.c_str()));
  TEST_ASSERT(File::GetDirectoryFileCount(temp_dir.c_str()) == 1);

  // Remove the entire tmp_dir incuding the sub-dir it contains
  TEST_ASSERT(File::DeleteRecursively(temp_dir.c_str()));
  TEST_ASSERT(!File::DirectoryExists(temp_dir.c_str()));

#ifdef WINCE
   // Test the directory creation on Windows Mobile
  const char16 *kValidDirNames[] = {
      STRING16(L"\\Dir1"),
      STRING16(L"\\Dir1\\Dir2"),
      STRING16(L"\\Dir1\\Dir2\\"),
      STRING16(L"/Dir1"),
      STRING16(L"/Dir1/Dir2/"),
      STRING16(L"/Dir1\\Dir2/"),      
      STRING16(L"\\Dir1/Dir2/"),      
      STRING16(L".\\Dir"),
      STRING16(L"..\\Dir.Dir2.Dir3/Dir4/")
  };
  const char16 *kInvalidDirNames[] = {      
      STRING16(L"//Dir1/////Dir2/"),
      STRING16(L"\\\\Dir1\\Dir2\\"),
      STRING16(L"\\Dir1\\:Dir2"),
      STRING16(L"&.<>:\"/\\|?*"),
      STRING16(L""),      
      STRING16(L"?")
  };

  for (int i = 0; i < ARRAYSIZE(kValidDirNames); ++i) {
    TEST_ASSERT(CheckDirectoryCreation(kValidDirNames[i]));
    TEST_ASSERT(File::DeleteRecursively(kValidDirNames[i]));
  }

  for (int i = 0; i < ARRAYSIZE(kInvalidDirNames); ++i) {
    TEST_ASSERT(!(File::RecursivelyCreateDir(kInvalidDirNames[i])));
  }    
#endif
  LOG(("TestFileUtils - passed\n"));
  return true;
}

bool TestFileOpen(std::string16 *error, std::string16 filepath,
                  File::OpenAccessMode access_mode,
                  File::OpenExistsMode exists_mode,
                  bool instantiated, bool existsAfter) {
  scoped_ptr<File> file(File::Open(filepath.c_str(), access_mode, exists_mode));
  TEST_ASSERT(File::Exists(filepath.c_str()) == existsAfter);
  return ((file.get() != NULL) == instantiated);
}


bool TestFileObject(std::string16 *error) {
  // Create a new empty directory work with
  std::string16 temp_dir;
  TEST_ASSERT(File::CreateNewTempDirectory(&temp_dir));
  TEST_ASSERT(File::DirectoryExists(temp_dir.c_str()));
  TEST_ASSERT(File::GetDirectoryFileCount(temp_dir.c_str()) == 0);

  const char16 *kFileName = STRING16(L"File.ext");
  std::string16 filepath(temp_dir);
  filepath += kPathSeparator;
  filepath += kFileName;

  // Read mode with nonexistent file; read should never create the file
  TEST_ASSERT(!File::Exists(filepath.c_str()));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ,
                           File::FAIL_IF_NOT_EXISTS, false, false));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ,
                           File::FAIL_IF_EXISTS, false, false));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ,
                           File::NEVER_FAIL, false, false));

  // Write mode with nonexistent file
  TEST_ASSERT(TestFileOpen(error, filepath, File::WRITE,
                           File::FAIL_IF_NOT_EXISTS, false, false));
  TEST_ASSERT(TestFileOpen(error, filepath, File::WRITE,
                           File::FAIL_IF_EXISTS, true, true));
  TEST_ASSERT(File::Delete(filepath.c_str()));
  TEST_ASSERT(TestFileOpen(error, filepath, File::WRITE,
                           File::NEVER_FAIL, true, true));
  TEST_ASSERT(File::Delete(filepath.c_str()));

  // Read/Write mode with nonexistent file
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ_WRITE,
                           File::FAIL_IF_NOT_EXISTS, false, false));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ_WRITE,
                           File::FAIL_IF_EXISTS, true, true));
  TEST_ASSERT(File::Delete(filepath.c_str()));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ_WRITE,
                           File::NEVER_FAIL, true, true));


  // Read mode with existent file
  TEST_ASSERT(File::Exists(filepath.c_str()));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ,
                           File::FAIL_IF_EXISTS, false, true));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ,
                           File::FAIL_IF_NOT_EXISTS, true, true));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ,
                           File::NEVER_FAIL, true, true));

  // Write mode with existent file
  TEST_ASSERT(TestFileOpen(error, filepath, File::WRITE,
                           File::FAIL_IF_EXISTS, false, true));
  TEST_ASSERT(TestFileOpen(error, filepath, File::WRITE,
                           File::FAIL_IF_NOT_EXISTS, true, true));
  TEST_ASSERT(TestFileOpen(error, filepath, File::WRITE,
                           File::NEVER_FAIL, true, true));

  // Read/Write mode with existent file
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ_WRITE,
                           File::FAIL_IF_EXISTS, false, true));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ_WRITE,
                           File::FAIL_IF_NOT_EXISTS, true, true));
  TEST_ASSERT(TestFileOpen(error, filepath, File::READ_WRITE,
                           File::NEVER_FAIL, true, true));

  const int size = 4096;
  uint8 data_write[size];
  uint8 data_read[size];
  for (int i = 0; i < size; i++) {
    data_write[i] = i;
  }

  // basic operations on 0-length file
  scoped_ptr<File> file(File::Open(filepath.c_str(), File::READ,
                                   File::NEVER_FAIL));
  TEST_ASSERT(file.get() != NULL);
  TEST_ASSERT(file->Size() == 0);
  TEST_ASSERT(file->Tell() == 0);
  TEST_ASSERT(!file->Seek(-1));
  TEST_ASSERT(!file->Truncate(0));
  TEST_ASSERT(file->Read(data_read, 1) == 0);
  TEST_ASSERT(file->Write(data_write, 1) == -1);
  // can seek past the end, but can't read anything
  TEST_ASSERT(file->Seek(1));
  TEST_ASSERT(file->Read(data_read, 1) == 0);

  // write-only mode
  file.reset();
  file.reset(File::Open(filepath.c_str(), File::WRITE, File::NEVER_FAIL));
  TEST_ASSERT(file.get() != NULL);
  TEST_ASSERT(file->Size() == 0);
  TEST_ASSERT(file->Tell() == 0);
  TEST_ASSERT(!file->Seek(-1));
  TEST_ASSERT(file->Truncate(0));
  TEST_ASSERT(file->Read(data_read, 1) == -1);
  TEST_ASSERT(file->Write(data_write, size) == size);
  TEST_ASSERT(file->Flush());
  TEST_ASSERT(file->Size() == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(file->Read(data_read, 1) == -1);

  // read what we just wrote on a different file handle
  file.reset();
  file.reset(File::Open(filepath.c_str(), File::READ, File::NEVER_FAIL));
  TEST_ASSERT(file.get() != NULL);
  TEST_ASSERT(file->Size() == size);
  TEST_ASSERT(file->Tell() == 0);
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size + 1) == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  // EOF is a 0-byte read
  TEST_ASSERT(file->Read(data_read, 1) == 0);
  TEST_ASSERT(file->Tell() == size);

  // read-write mode combined with seek
  file.reset();
  file.reset(File::Open(filepath.c_str(), File::READ_WRITE, File::NEVER_FAIL));
  TEST_ASSERT(file.get() != NULL);
  TEST_ASSERT(file->Size() == size);
  TEST_ASSERT(file->Tell() == 0);
  TEST_ASSERT(!file->Seek(-1));
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size + 1) == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  // write a second copy of the same bytes
  TEST_ASSERT(file->Seek(0, File::SEEK_FROM_CURRENT));
  TEST_ASSERT(file->Write(data_write, size) == size);
  TEST_ASSERT(file->Tell() == size * 2);
  // verify the first copy
  TEST_ASSERT(file->Seek(-1 * size, File::SEEK_FROM_END));
  TEST_ASSERT(file->Tell() == size);
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size + 1) == size);
  TEST_ASSERT(file->Tell() == size * 2);
  TEST_ASSERT(file->Read(data_read, 1) == 0);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  // verify both copies
  TEST_ASSERT(file->Seek(0, File::SEEK_FROM_START));
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size) == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size + 1) == size);
  TEST_ASSERT(file->Tell() == size * 2);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  TEST_ASSERT(file->Read(data_read, 1) == 0);
  TEST_ASSERT(file->Tell() == size * 2);

  // overwrite with new data and truncate
  file.reset();
  file.reset(File::Open(filepath.c_str(), File::READ_WRITE, File::NEVER_FAIL));
  TEST_ASSERT(file.get() != NULL);
  TEST_ASSERT(file->Size() == size * 2);
  // verify both existing copies again
  memset(data_read, 0, size);
  TEST_ASSERT(file->Tell() == 0);
  TEST_ASSERT(file->Read(data_read, size) == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size + 1) == size);
  TEST_ASSERT(file->Tell() == size * 2);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  TEST_ASSERT(file->Read(data_read, 1) == 0);
  TEST_ASSERT(file->Tell() == size * 2);
  // overwrite the first copy with new bytes
  TEST_ASSERT(file->Seek(size * -2, File::SEEK_FROM_END));
  TEST_ASSERT(file->Tell() == 0);
  memset(data_write, 14, size);
  TEST_ASSERT(file->Write(data_write, size) == size);
  TEST_ASSERT(file->Tell() == size);
  // and truncate the second copy
  TEST_ASSERT(file->Seek(0));
  TEST_ASSERT(file->Truncate(size));
  TEST_ASSERT(file->Tell() == 0);
  TEST_ASSERT(file->Size() == size);
  // verify
  TEST_ASSERT(file->Seek(0, File::SEEK_FROM_START));
  TEST_ASSERT(file->Tell() == 0);
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size + 1) == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  TEST_ASSERT(file->Read(data_read, 1) == 0);
  TEST_ASSERT(file->Tell() == size);

  // boundary read/write parameters
  TEST_ASSERT(file->Read(data_read, 0) == 0);
  TEST_ASSERT(file->Read(data_read, -1) == -1);
  TEST_ASSERT(file->Write(data_write, 0) == 0);
  TEST_ASSERT(file->Write(data_write, -1) == -1);
  // NULL read/write parameters
  TEST_ASSERT(file->Read(NULL, 0) == -1);
  TEST_ASSERT(file->Write(NULL, 0) == -1);

  // tmpfile
  file.reset();
  file.reset(File::CreateNewTempFile());
  TEST_ASSERT(file->Size() == 0);
  TEST_ASSERT(file->Tell() == 0);
  TEST_ASSERT(file->Write(data_write, size) == size);
  TEST_ASSERT(file->Flush());
  TEST_ASSERT(file->Size() == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(file->Seek(0, File::SEEK_FROM_START));
  TEST_ASSERT(file->Tell() == 0);
  memset(data_read, 0, size);
  TEST_ASSERT(file->Read(data_read, size + 1) == size);
  TEST_ASSERT(file->Tell() == size);
  TEST_ASSERT(memcmp(data_write, data_read, size) == 0);
  // EOF is a 0-byte read
  TEST_ASSERT(file->Read(data_read, 1) == 0);
  TEST_ASSERT(file->Tell() == size);

  // Remove the entire tmp_dir incuding the sub-dir it contains
  TEST_ASSERT(File::DeleteRecursively(temp_dir.c_str()));
  TEST_ASSERT(!File::DirectoryExists(temp_dir.c_str()));

  return true;
}

bool TestLongPaths(std::string16 *error) {
// Only Win32 has issues with long pathnames, ignore WinCE for now.
#if defined(WIN32) && !defined(WINCE)

  // Constants
  const std::string16 kDrv(STRING16(L"c:"));
  const std::string16 kSep(&kPathSeparator, 1);
  const std::string16 kFoo(STRING16(L"kFoo"));

  // Create a new kEmpty directory to work with
  std::string16 temp_dir;
  TEST_ASSERT(File::CreateNewTempDirectory(&temp_dir));
  TEST_ASSERT(File::DirectoryExists(temp_dir.c_str()));
  TEST_ASSERT(File::GetDirectoryFileCount(temp_dir.c_str()) == 0);

  std::string16 long_file_name;  // longest legal filename
  for (size_t i = 0; i < File::kMaxPathComponentChars; ++i) {
    long_file_name += L'A';
  }

  //------------ RecursivelyCreateDir ------------
  // Creating c: & c:\ should succeed
  TEST_ASSERT(File::RecursivelyCreateDir(kDrv.c_str()));

  // The previous implementation of RecursivelyCreateDir fails this test, as 
  // SHCreateDirectoryEx() returns an error when trying to create c:\. We
  // do not emulate this behavior.
  TEST_ASSERT(File::RecursivelyCreateDir((kDrv + kSep).c_str()));

  // Check that creating a directory with the same name as a file fails.
  {
    std::string16 file_path = temp_dir + kSep + kFoo;
    TEST_ASSERT(File::CreateNewFile(file_path.c_str()));
    TEST_ASSERT(File::Exists(file_path.c_str()));
    
    TEST_ASSERT(!File::RecursivelyCreateDir(file_path.c_str()));

    // Make sure file wasn't touched...
    TEST_ASSERT(File::Exists(file_path.c_str()));
    
    // then delete is.
    TEST_ASSERT(File::Delete(file_path.c_str()));
    TEST_ASSERT(!File::Exists(file_path.c_str()));
  }

  // Check Reading & Writing data from a file with a long path
  {
    std::string16 parent_dir = temp_dir + kSep;

    // Make sure that the directory this file is in is already longer than
    // MAX_PATH.
    while (parent_dir.length() < MAX_PATH) {
      parent_dir += long_file_name + kSep;
    }

    //long filename
    const std::string16 kFilenameSuffix = STRING16(L"_data_test.txt");
    const std::string16 kFilePath  = parent_dir + 
        long_file_name.substr(0, File::kMaxPathComponentChars - 
                           kFilenameSuffix.length()) + 
         kFilenameSuffix;

    uint8 write_data = 0xAA;
    std::vector<uint8> write_vector;
    std::vector<uint8> read_data;
    
    TEST_ASSERT(!File::DirectoryExists(parent_dir.c_str()));
    TEST_ASSERT(File::RecursivelyCreateDir(parent_dir.c_str()));
    TEST_ASSERT(File::DirectoryExists(parent_dir.c_str()));

    TEST_ASSERT(!File::Exists(kFilePath.c_str()));
    TEST_ASSERT(!File::ReadFileToVector(kFilePath.c_str(), &read_data));

    TEST_ASSERT(File::CreateNewFile(kFilePath.c_str()));
    TEST_ASSERT(File::WriteBytesToFile(kFilePath.c_str(), &write_data, 1));
    TEST_ASSERT(File::ReadFileToVector(kFilePath.c_str(), &read_data));
    TEST_ASSERT(read_data.size() == 1 && read_data[0] == write_data);

    TEST_ASSERT(File::Delete(kFilePath.c_str()));
    TEST_ASSERT(!File::Exists(kFilePath.c_str()));

    write_vector.push_back(write_data);
    TEST_ASSERT(File::CreateNewFile(kFilePath.c_str()));
    TEST_ASSERT(File::Exists(kFilePath.c_str()));
    TEST_ASSERT(File::WriteVectorToFile(kFilePath.c_str(), &write_vector));
    TEST_ASSERT(File::ReadFileToVector(kFilePath.c_str(), &read_data));
    TEST_ASSERT(read_data.size() == 1 && read_data[0] == write_data);

    TEST_ASSERT(File::Delete(kFilePath.c_str()));
    TEST_ASSERT(!File::Exists(kFilePath.c_str()));

    TEST_ASSERT(File::DeleteRecursively(parent_dir.c_str()));
    TEST_ASSERT(!File::DirectoryExists(parent_dir.c_str()));
  }

  // Check that creating a long directory path works
  // DeleteRecursively
  {
    const std::string16 kTopOfTestDir = temp_dir + kSep + long_file_name + kSep;
    std::string16 file_path = kTopOfTestDir;

    while (file_path.length() < MAX_PATH) {
      file_path += long_file_name + kSep;
    }

    TEST_ASSERT(!File::DirectoryExists(file_path.c_str()));

    TEST_ASSERT(File::RecursivelyCreateDir(file_path.c_str()));
    TEST_ASSERT(File::RecursivelyCreateDir(file_path.c_str()));

    TEST_ASSERT(File::DirectoryExists(file_path.c_str()));
    TEST_ASSERT(File::DeleteRecursively(kTopOfTestDir.c_str()));
    TEST_ASSERT(!File::DirectoryExists(kTopOfTestDir.c_str()));
  }
  
  // DirectoryExists, Exists, CreateNewFile, Delete, GetDirectoryFileCount
  {
    std::string16 parent_dir = temp_dir + kSep;

    // Make sure that the directory this file is in is already longer than
    // MAX_PATH.
    while (parent_dir.length() < MAX_PATH) {
      parent_dir += long_file_name + kSep;
    }

    std::string16 file_path = parent_dir + long_file_name;
    std::string16 other_file_path = parent_dir + STRING16(L"a.txt");


    TEST_ASSERT(!File::DirectoryExists(file_path.c_str()));
    TEST_ASSERT(!File::Exists(file_path.c_str()));
    
    TEST_ASSERT(File::RecursivelyCreateDir(parent_dir.c_str()));
    TEST_ASSERT(File::CreateNewFile(file_path.c_str()));
    TEST_ASSERT(File::Exists(file_path.c_str()));

    TEST_ASSERT(!File::CreateNewFile(file_path.c_str()));
    TEST_ASSERT(File::Exists(file_path.c_str()));

    TEST_ASSERT(File::GetDirectoryFileCount(parent_dir.c_str()) == 1);

    TEST_ASSERT(File::CreateNewFile(other_file_path.c_str()));
    TEST_ASSERT(File::Exists(other_file_path.c_str()));

    TEST_ASSERT(File::GetDirectoryFileCount(parent_dir.c_str()) == 2);
    
    TEST_ASSERT(File::Delete(file_path.c_str()));
    TEST_ASSERT(!File::Delete(file_path.c_str()));

    TEST_ASSERT(File::Delete(other_file_path.c_str()));

    TEST_ASSERT(File::GetDirectoryFileCount(parent_dir.c_str()) == 0);

    TEST_ASSERT(!File::DirectoryExists(file_path.c_str()));
    TEST_ASSERT(!File::Exists(file_path.c_str()));

    TEST_ASSERT(File::DeleteRecursively(parent_dir.c_str()));

  }
#endif  // defined(WIN32) && !defined(WINCE)
  return true;
}

// friend of File class, so can't be static
bool TestSplitPath(std::string16 *error) {
  
  const std::string16 kSep(&kPathSeparator, 1);
  
  const std::string16 kFoo(STRING16(L"foo"));
  const std::string16 kBar(STRING16(L"bar"));
  const std::string16 kEmpty;
  File::PathComponents components;
  
  // '/foo' -> ['foo']
  File::SplitPath(kSep + kFoo, &components);
  TEST_ASSERT(components.size() == 1 && components[0] == kFoo);

  // '/foo/bar/' -> ['foo', 'bar']
  File::SplitPath(kSep + kFoo + kSep + kBar + kSep, &components);
  TEST_ASSERT(components.size() == 2 && 
      components[0] == kFoo &&
      components[1] == kBar);

#ifdef WIN32
  const std::string16 kDrv(STRING16(L"c:"));

  // 'c:/foo' -> ['c:\','foo']
  File::SplitPath(kDrv + kSep + kFoo, &components);
  TEST_ASSERT(components.size() == 2 && components[0] == kDrv &&
      components[1] == kFoo);
#endif  // WIN32

  return true;

}

static bool TestGetParentDirectory(std::string16 *error) {
  const std::string16 kSep(&kPathSeparator, 1);
  const std::string16 kBackslash(STRING16(L"\\"));
  const std::string16 kA(STRING16(L"a"));
  const std::string16 kDrvSep(STRING16(L"c:"));
  const std::string16 kFoo(STRING16(L"foo"));
  const std::string16 kBar(STRING16(L"bar"));
  const std::string16 kEmpty;
  std::string16 out;

  // Fail on empty string.
  TEST_ASSERT(!File::GetParentDirectory(kEmpty, &out));
  
  // Fail on path with no parent specified.
  TEST_ASSERT(!File::GetParentDirectory(kFoo, &out));
  
  // 'a/foo' -> 'a'
  TEST_ASSERT(File::GetParentDirectory(kA + kSep + kFoo, &out));
  TEST_ASSERT(out == kA);
  
  // 'a//foo' -> 'a'
  TEST_ASSERT(File::GetParentDirectory(kA + kSep + kSep + kFoo, &out));
  TEST_ASSERT(out == kA);
  
  // 'a///foo' -> 'a'
  TEST_ASSERT(File::GetParentDirectory(kA + kSep + kSep + kSep + kFoo, &out));
  TEST_ASSERT(out == kA);
  
  // '/a/foo' -> '/a'
  TEST_ASSERT(File::GetParentDirectory(kSep + kA + kSep + kFoo, &out));
  TEST_ASSERT(out == kSep + kA);
  
  // 'c:/bar/a/foo' -> 'c:/bar/a'
  std::string16 parent_dir = kDrvSep + kSep + kBar + kSep + kA;
  TEST_ASSERT(File::GetParentDirectory(parent_dir + kSep + kFoo, &out));
  TEST_ASSERT(out == parent_dir);
  
  return true;
}

static bool TestGetBaseName(std::string16 *error) {
  // GetBaseName
  const std::string16 kSep(&kPathSeparator, 1);
  const std::string16 kDrvSep(STRING16(L"c:"));
  const std::string16 kA(STRING16(L"a"));
  const std::string16 kADotTxt(STRING16(L"a.txt"));
  const std::string16 kFoo(STRING16(L"foo"));
  const std::string16 kEmpty;
  std::string16 out;

  // '' -> ''
  File::GetBaseName(kEmpty, &out);
  TEST_ASSERT(out == kEmpty);
  
  // 'a'->'a'
  File::GetBaseName(kA, &out);
  TEST_ASSERT(out == kA);

  // 'a.txt'->'a.txt'
  File::GetBaseName(kADotTxt, &out);
  TEST_ASSERT(out == kADotTxt);

  // '\a'->'a'
  File::GetBaseName(kSep + kA, &out);
  TEST_ASSERT(out == kA);

  // '\\a'->'a'
  File::GetBaseName(kSep + kSep + kA, &out);
  TEST_ASSERT(out == kA);

  // '\foo\a.txt'->'a.txt'
  File::GetBaseName(kSep + kFoo + kSep + kADotTxt, &out);
  TEST_ASSERT(out == kADotTxt);

  // ------ Directories -------

  // '\' -> '\'
  File::GetBaseName(kSep, &out);
  TEST_ASSERT(out == kSep);

  // '\\' -> '\'
  File::GetBaseName(kSep + kSep, &out);
  TEST_ASSERT(out == kSep);

  // '\a\' -> 'a'
  File::GetBaseName(kSep + kA + kSep, &out);
  TEST_ASSERT(out == kA);

  // -------- Windows Only Tests -------
#ifdef WIN32
  // 'c:\a'->'a'
  File::GetBaseName(kDrvSep + kSep + kA, &out);
  TEST_ASSERT(out == kA);

  // 'c:\foo\a.txt'->'a.txt'
  File::GetBaseName(kDrvSep  + kSep + kFoo + kSep + kADotTxt, &out);
  TEST_ASSERT(out == kADotTxt);

  // 'c:\\foo\\a.txt'->'a.txt'
  File::GetBaseName(kDrvSep + kSep + kSep+ kFoo + kSep + kSep + kADotTxt, &out);
  TEST_ASSERT(out == kADotTxt);

  // 'c:\' -> 'c:'
  File::GetBaseName(kDrvSep + kSep, &out);
  TEST_ASSERT(out == kDrvSep);

  // 'c:\\' -> 'c:'
  File::GetBaseName(kDrvSep + kSep + kSep, &out);
  TEST_ASSERT(out == kDrvSep);

  // Paths starting with '\\\' are illegal in win32.
  TEST_ASSERT(!File::GetBaseName(kSep + kSep + kSep, &out));

#else
  // '\\\' -> '\'
  File::GetBaseName(kSep + kSep + kSep, &out);
  TEST_ASSERT(out == kSep);
#endif  // WIN32

  return true;
}

static bool CheckDirectoryCreation(const char16 *dir) {
  return File::RecursivelyCreateDir(dir) && File::DirectoryExists(dir);  
}

static bool TestMultipleOpen(std::string16 *error) {
  // Create a new empty directory in which to work.
  std::string16 temp_dir;
  TEST_ASSERT(File::CreateNewTempDirectory(&temp_dir));
  TEST_ASSERT(File::DirectoryExists(temp_dir.c_str()));
  TEST_ASSERT(File::GetDirectoryFileCount(temp_dir.c_str()) == 0);

  const char16 *kFileName = STRING16(L"File.ext");
  std::string16 filepath(temp_dir);
  filepath += kPathSeparator;
  filepath += kFileName;

  // Open file twice for write. This will also create the files,
  // so that they can later be successfully opened for READ.
  {
    scoped_ptr<File> file1(File::Open(filepath.c_str(), File::WRITE,
                                      File::NEVER_FAIL));
    TEST_ASSERT(file1.get());
    scoped_ptr<File> file2(File::Open(filepath.c_str(), File::WRITE,
                                      File::NEVER_FAIL));
    TEST_ASSERT(file2.get());
  }

  // Open file twice for read.
  {
    scoped_ptr<File> file1(File::Open(filepath.c_str(), File::READ,
                                      File::NEVER_FAIL));
    TEST_ASSERT(file1.get());
    scoped_ptr<File> file2(File::Open(filepath.c_str(), File::READ,
                                      File::NEVER_FAIL));
    TEST_ASSERT(file2.get());
  }

  // Open file once for read, once for write.
  {
    scoped_ptr<File> file1(File::Open(filepath.c_str(), File::READ,
                                      File::NEVER_FAIL));
    TEST_ASSERT(file1.get());
    scoped_ptr<File> file2(File::Open(filepath.c_str(), File::WRITE,
                                      File::NEVER_FAIL));
    TEST_ASSERT(file2.get());
  }

  // Remove the entire tmp_dir.
  TEST_ASSERT(File::DeleteRecursively(temp_dir.c_str()));
  TEST_ASSERT(!File::DirectoryExists(temp_dir.c_str()));
  return true;
}
