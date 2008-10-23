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

#ifndef GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_SCOPED_RELEASER_H__
#define GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_SCOPED_RELEASER_H__

#include <new>
#include <Foundation/Foundation.h>

///  Some stack based classes for dealing with Reference counted Apple types in an exceptional C++/ObjC++ world.

///  A stack based class for RAII type handling of NS based types that need
///  release called on them.
template <typename T> class GMNSScopedReleaser {
 public:
  explicit GMNSScopedReleaser(T* value) : value_(value) { }
  // Even in a GC world, this release should probably stay here because
  // you never know when it is going to be run under non-gc.
  ~GMNSScopedReleaser() { [value_ release]; }
  
  operator T*() { return value_; }
  T* get() { return value_; }
  bool isValid() { return value_ != nil; }
 private:
  T* value_;
  
  //  Keep anybody from ever creating one of these things not on the stack.
  GMNSScopedReleaser() { }
  GMNSScopedReleaser(const GMNSScopedReleaser&);
  GMNSScopedReleaser & operator=(GMNSScopedReleaser&);
};

///  Stack based class for dealing with NSAutoreleasepools which is one
///  of our most common uses for the GMNSScopedReleaser.
class GMAutoreleasePool {
 public:
  GMAutoreleasePool() : pool_([[NSAutoreleasePool alloc] init]) { }
  
 private:
  //  Keep anybody from ever creating one of these things not on the stack.
  GMAutoreleasePool(const GMAutoreleasePool&);
  GMAutoreleasePool & operator=(GMAutoreleasePool&);
  GMNSScopedReleaser<NSAutoreleasePool> pool_; //  STRONG our pool  
};

///  A stack based class for RAII type handling of CF based types that need
///  CFRelease called on them. Checks for NULL before calling release.
///  Allows you to use other release funcs as necessary (i.e. CGReleaseContext) 
///  by passing them in.
template <typename T> class GMCFScopedReleaser {
 public:
  typedef void (*ReleaseFunc)(T cf);
  
  explicit GMCFScopedReleaser(T value, ReleaseFunc func=LocalRelease) 
    : value_(value), release_func(func) { }
  explicit GMCFScopedReleaser(CFTypeRef value, ReleaseFunc func=LocalRelease) 
    : value_(reinterpret_cast<T>(value)), release_func(func) { }
  ~GMCFScopedReleaser() { 
    if (value_ && release_func) {
      release_func(value_); 
    }
  }
  T get() { return value_; }
  operator T() { return value_; }
  bool isValid() { return value_ != nil; }
  
 private:
  T value_;
  ReleaseFunc release_func;
  
  inline static void LocalRelease(T cf) { CFRelease(cf); }
  
  //  Keep anybody from ever creating one of these things not on the stack.
  GMCFScopedReleaser() { }
  GMCFScopedReleaser(const GMCFScopedReleaser&);
  GMCFScopedReleaser & operator=(GMCFScopedReleaser&);
};
#endif  // GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_SCOPED_RELEASER_H__
