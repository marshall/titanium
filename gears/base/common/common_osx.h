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


#ifndef GEARS_BASE_COMMON_COMMON_OSX_H__
#define GEARS_BASE_COMMON_COMMON_OSX_H__

#if defined(OS_MACOSX)

#import <string>
#import <Carbon/Carbon.h>
#include "gears/base/common/string16.h"

// TODO: Move all generally applicable OS X code from common_sf.h here.

// Initialize an NSAutoReleasePool.
void *InitAutoReleasePool();

// Destroys an autorelease pool, passing in NULL is legal and is a no-op.
void DestroyAutoReleasePool(void *pool);

// Returns a resource directory of a currently loaded browser plugin (not
// a browser app resource directory).
// If called from out-of-browser executable, returns the resource directory
// of that executable.
std::string ModuleResourcesDirectory();

// Returns path to the tempfile directory for the current user.
std::string TempDirectoryForCurrentUser();

// Get a Carbon window handle from a Cocoa one.
WindowRef GetWindowPtrFromNSWindow(void* ns_window);

// Get a window handle to the application main window.
WindowRef GetMainWindow();

// Get a window handle to the application window that has the keyboard focus.
WindowRef GetKeyWindow();

// Wrap NSLog so it can be called from C++ with C strings.
void SystemLog(const char *fn, ...);
void SystemLog16(const char16 *msg_utf16, ...);

#ifndef LOG
#ifdef DEBUG
#define LOG(a) do { SystemLog a; } while (0)
#else
#define LOG(a) do { } while (0)
#endif
#endif

#ifndef LOG16
#ifdef DEBUG
#define LOG16(a) do { SystemLog16 a; } while (0)
#else
#define LOG16(a) do { } while (0)
#endif
#endif

#endif  // OS_MACOSX
#endif  // GEARS_BASE_COMMON_COMMON_OSX_H__
