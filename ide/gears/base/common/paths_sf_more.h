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

// Utilities operating on paths (files and directories)

#ifndef GEARS_BASE_COMMON_PATHS_SF_MORE_H__
#define GEARS_BASE_COMMON_PATHS_SF_MORE_H__

#if defined(__OBJC__)
#import <Foundation/Foundation.h>
@interface GearsPathUtilities : NSObject
// Ensures all directories along the specified |dirPath| exist.  Any directories
// that do not exist will be created.
// Returns YES on success, NO otherwise
// The method will typically fail if a component of |dirPath| is either not
// writable by this user or that a file exists with the same name.
+ (BOOL)ensureDirectoryPathExists:(NSString *)dirPath;

// Locate a particular folder and provide the option to create it if missing
// Valid values for |folderType| and |domain| are the same as those passed to
// FSFindFolder().
+ (NSString *)pathForFolder:(OSType)folderType domain:(short)domain 
                     create:(BOOL)create;

// Returns the directory for resources (Resource path within plugin)
+ (NSString *)gearsResourcesDirectory;

// Returns the base directory for all Gears user data for the current user.
// Does not try to create the directory.
+ (NSString *)gearsDataDirectory;

// Return a full path with a unique (UUID) name in /tmp.
// Can be used for files or directories.
+ (NSString *)gearsUniqueTempPath;

@end

#endif  // __OBJC__
#endif  // GEARS_BASE_COMMON_PATHS_SF_MORE_H__
