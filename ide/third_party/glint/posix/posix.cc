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

// Implementation of a subset of the Platform interface for Posix platforms

#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "glint/posix/posix.h"

namespace glint_posix {

static uint64 kMicrosecsTo100ns = static_cast<uint64>(10);
static uint64 kMillisecsTo100ns = static_cast<uint64>(10000);
static uint64 kSecsTo100ns      = kMillisecsTo100ns * 1000;

static uint64 GetCurrent100NSTime() {
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  uint64 retval = tv.tv_sec * kSecsTo100ns;
  retval += tv.tv_usec * kMicrosecsTo100ns;
  return retval;
}

void PosixPlatform::CrashWithMessage(const char* format, ...) {
  va_list args;
  va_start(args, format);
  TraceImplementation(format, args);
  va_end(args);
  // crash now
  abort();
}

void PosixPlatform::TraceImplementation(const char* format, va_list args) {
  vfprintf(stderr, format, args);
}

// Timer functions. The Timer should return real64 value that is in seconds.
// The 0 point does not matter, the only assumption is that the value does
// not overflow during reasonable life expectancy of the app.
real64 PosixPlatform::GetCurrentSeconds() {
  uint64 hundred_ns = GetCurrent100NSTime();
  return (static_cast<real64>(hundred_ns) / kSecsTo100ns);
}

// Memory allocation functions
void* PosixPlatform::AllocateMemory(size_t byte_count) {
  return malloc(byte_count);
}

void* PosixPlatform::ReallocateMemory(void *memory_block,
                                      size_t byte_count) {
  return realloc(memory_block, byte_count);
}

void PosixPlatform::FreeMemory(void *memory_block) {
  free(memory_block);
}

void PosixPlatform::CopyMemoryBlock(void *to,
                                    const void *from,
                                    size_t byte_count) {
  memcpy(to, from, byte_count);
}

void PosixPlatform::MoveMemoryBlock(void *to,
                                    const void *from,
                                    size_t byte_count) {
  memmove(to, from, byte_count);
}

// Gets a function pointer given the library name and function name.
// On Win32, it corresponds to LoadLibrary/GetProcAddress pair.
void* PosixPlatform::GetDynamicLibraryFunction(const char* library_name,
                                               const char* function_name) {
  void* lib = dlopen(library_name, RTLD_LOCAL);
  if (lib == NULL) {
    Trace("Couldn't load library %s", library_name);
    return NULL;
  }

  return dlsym(lib, function_name);
}

bool PosixPlatform::ReadFileAsUtf8(const std::string& filename,
                                   std::string* content) {
  if (!content)
    return false;

  AutoCloseFd fd(open(filename.c_str(), O_RDONLY, 0));

  if (fd.get() == -1) {
    Trace("Open failed on %s", filename.c_str());
    return false;
  }

  struct stat st;
  if (fstat(fd.get(), &st) != 0) {
    return false;
  }

  off_t file_size = st.st_size;
  content->resize(file_size);

  // note not "+ 1" since STL string is an STL container, "resize" is in
  // terms of actual elements and trailing \0 is accounted for.
  content->resize(file_size);
  char* buffer = &content->at(0);
  if (!buffer || read(fd.get(), buffer, file_size) < 0) {
    return false;
  }

  buffer[file_size] = '\0';
  return true;
}

// Sprintf (apparently, also platform-specific implementation, due to
// memory/security issues).
int PosixPlatform::Sprintf(char *buffer, int buffer_size, const char *format,
                           va_list args) {
  return vsnprintf(buffer, buffer_size, format, args);
}

}  // namespace glint_posix
