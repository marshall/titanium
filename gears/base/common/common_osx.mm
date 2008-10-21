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

#import <Cocoa/Cocoa.h>
#import "gears/base/common/common_osx.h"
#import <string>

@interface PathFinder : NSObject
+ (NSString *)moduleResourcesDirectory;
@end

@implementation PathFinder
+ (NSString *)moduleResourcesDirectory {
  NSBundle *pluginBundle = [NSBundle bundleForClass:[PathFinder class]];
  return [pluginBundle resourcePath];
}
@end

void *InitAutoReleasePool() {
  return [[NSAutoreleasePool alloc] init];
}

void DestroyAutoReleasePool(void *pool) {
  assert(pool);
  [(NSAutoreleasePool *)pool release];
}

std::string ModuleResourcesDirectory() {
  NSString * ns_path = [PathFinder moduleResourcesDirectory];
  return std::string([ns_path fileSystemRepresentation]);
}

std::string TempDirectoryForCurrentUser() {
  return std::string([NSTemporaryDirectory() fileSystemRepresentation]);
}

WindowRef GetWindowPtrFromNSWindow(void* ns_window) {
  assert(ns_window);
  return (WindowRef)[(NSWindow*)ns_window windowRef];
}

WindowRef GetMainWindow() {
  return (WindowRef)[[NSApp mainWindow] windowRef];

}

WindowRef GetKeyWindow() {
  return (WindowRef)[[NSApp keyWindow] windowRef];
}

void SystemLog(const char *msg, ...) {
  // Logging is called from C++ worker threads, so need to have autorelease pool.
  NSAutoreleasePool *autoreleasePool = [[NSAutoreleasePool alloc] init];
  va_list args;
  va_start(args, msg);
  NSLogv([NSString stringWithCString:msg], args);
  va_end(args);
  [autoreleasePool release];
}

void SystemLog16(const char16 *msg_utf16, ...) {
  // Logging is called from C++ worker threads, so need to have autorelease pool.
  NSAutoreleasePool *autoreleasePool = [[NSAutoreleasePool alloc] init];
  va_list args;
  va_start(args, msg_utf16);

  CFStringRef format =
      CFStringCreateWithCharacters(NULL,
                                   msg_utf16,
                                   std::char_traits<char16>::length(msg_utf16));

  CFIndex length = CFStringGetLength(format);

  CFMutableStringRef format_mutable =
      CFStringCreateMutableCopy(NULL, length, format);

  CFStringFindAndReplace(format_mutable,
                         CFSTR("%s"),
                         CFSTR("%S"),
                         CFRangeMake(0, length),
                         0);

  NSLogv((NSString *)format_mutable, args);
  va_end(args);

  CFRelease(format);
  CFRelease(format_mutable);
  [autoreleasePool release];
}

