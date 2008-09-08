// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// macho_id.h: Functions to gather identifying information from a macho file
//
// Author: Dan Waylonis

#ifndef COMMON_MAC_MACHO_ID_H__
#define COMMON_MAC_MACHO_ID_H__

#include <limits.h>
#include <mach-o/loader.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace MacFileUtilities {

class MachoWalker;

class MachoID {
 public:
  MachoID(const char *path);
  ~MachoID();

  // For the given |cpu_type|, return a UUID from the LC_UUID command.
  // Return false if there isn't a LC_UUID command.
  bool UUIDCommand(int cpu_type, unsigned char identifier[16]);

  // For the given |cpu_type|, return a UUID from the LC_ID_DYLIB command.
  // Return false if there isn't a LC_ID_DYLIB command.
  bool IDCommand(int cpu_type, unsigned char identifier[16]);

  // For the given |cpu_type|, return the Adler32 CRC for the mach-o data
  // segment(s).
  // Return 0 on error (e.g., if the file is not a mach-o file)
  uint32_t Adler32(int cpu_type);

  // For the given |cpu_type|, return the MD5 for the mach-o data segment(s).
  // Return true on success, false otherwise
  bool MD5(int cpu_type, unsigned char identifier[16]);

  // For the given |cpu_type|, return the SHA1 for the mach-o data segment(s).
  // Return true on success, false otherwise
  bool SHA1(int cpu_type, unsigned char identifier[16]);

 private:
  // Signature of class member function to be called with data read from file
  typedef void (MachoID::*UpdateFunction)(unsigned char *bytes, size_t size);

  // Update the CRC value by examining |size| |bytes| and applying the algorithm
  // to each byte.
  void UpdateCRC(unsigned char *bytes, size_t size);

  // Update the MD5 value by examining |size| |bytes| and applying the algorithm
  // to each byte.
  void UpdateMD5(unsigned char *bytes, size_t size);

  // Update the SHA1 value by examining |size| |bytes| and applying the
  // algorithm to each byte.
  void UpdateSHA1(unsigned char *bytes, size_t size);

  // Bottleneck for update routines
  void Update(MachoWalker *walker, unsigned long offset, size_t size);

  // The callback from the MachoWalker for CRC, MD5, and SHA1
  static bool WalkerCB(MachoWalker *walker, load_command *cmd, off_t offset,
                       bool swap, void *context);

  // The callback from the MachoWalker for LC_UUID
  static bool UUIDWalkerCB(MachoWalker *walker, load_command *cmd, off_t offset,
                           bool swap, void *context);

  // The callback from the MachoWalker for LC_ID_DYLIB
  static bool IDWalkerCB(MachoWalker *walker, load_command *cmd, off_t offset,
                         bool swap, void *context);

  // File path
  char path_[PATH_MAX];

  // File descriptor
  int file_;

  // The current crc value
  uint32_t crc_;

  // The MD5 context
  MD5_CTX md5_context_;

  // The SHA1 context
  SHA_CTX sha1_context_;

  // The current update to call from the Update callback
  UpdateFunction update_function_;
};

}  // namespace MacFileUtilities

#endif  // COMMON_MAC_MACHO_ID_H__
