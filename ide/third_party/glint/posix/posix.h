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

// Definition of a subset of the Platform interface for Posix platforms

#include "glint/include/platform.h"

namespace glint_posix {

using namespace glint;

class PosixPlatform : public Platform {
 public:
  void CrashWithMessage(const char* format, ...);
  void TraceImplementation(const char* format, va_list args);

  // Timer functions. The Timer should return real64 value that is in seconds.
  // The 0 point does not matter, the only assumption is that the value does
  // not overflow during reasonable life expectancy of the app.
  virtual real64 GetCurrentSeconds();

  // Memory allocation functions
  virtual void *AllocateMemory(size_t byte_count);
  virtual void *ReallocateMemory(void *memory_block, size_t byte_count);
  virtual void FreeMemory(void *memory_block);
  virtual void CopyMemoryBlock(void *to,
                               const void *from,
                               size_t byte_count);
  virtual void MoveMemoryBlock(void *to,
                               const void *from,
                               size_t byte_count);

  // Gets a function pointer given the library name and function name.
  // On Win32, it corresponds to LoadLibrary/GetProcAddress pair.
  virtual void* GetDynamicLibraryFunction(const char* library_name,
                                          const char* function_name);

  virtual bool ReadFileAsUtf8(const std::string& filename,
                              std::string* content);

  // Sprintf (apparently, also platform-specific implementation, due to
  // memory/security issues).
  virtual int Sprintf(char *buffer,
                      int buffer_size,
                      const char *format,
                      va_list args);

 private:
  class AutoCloseFd {
   public:
    AutoCloseFd(int fd) : fd_(fd) {}
    ~AutoCloseFd() { if (fd_ >= 0) { close(fd_); } }
    int get() { return fd_; }
   private:
    int fd_;
  };
};

}  // namespace glint_posix
