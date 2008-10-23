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

#import "genfiles/product_constants.h"
#import "gears/base/common/common.h"
#import "gears/base/safari/scoped_cf.h"
#import "gears/base/common/string_utils.h"
#import "gears/base/common/paths_sf_more.h"

@implementation GearsPathUtilities
//------------------------------------------------------------------------------
+ (BOOL)ensureDirectoryPathExists:(NSString *)dirPath {
  NSFileManager *mgr = [NSFileManager defaultManager];
  BOOL isDir;
  
  // Standardize the path
  dirPath = [dirPath stringByStandardizingPath];
  
  // If we got a relative path, prepend the current directory
  if (![dirPath isAbsolutePath]) {
    NSString *currentPath = [mgr currentDirectoryPath];
    dirPath = [currentPath stringByAppendingPathComponent:dirPath];
  }
  
  NSString *path = dirPath;
  
  // Ensure that no file exists within the path which would block creation
  while (1) {
    if ([mgr fileExistsAtPath:path isDirectory:&isDir]) {
      if (isDir)
        break;
      
      LOG(("File exists at %s", 
           [path cStringUsingEncoding:NSUTF8StringEncoding]));
      return NO;
    }
    
    NSString *parentPath = [path stringByDeletingLastPathComponent];
    
    if (([parentPath length] == [path length]) || (![parentPath length]))
      break;
    
    path = parentPath;
  }
  
  // Path now contains the first valid directory (or is empty)
  if (![path length])
    return NO;
  
  // If dirPath already exists, we can exit now
  if ([path isEqualToString:dirPath])
    return YES;
  
  // Break up the difference into components
  NSString *diff = [dirPath substringFromIndex:[path length] + 1];
  NSArray *components = [diff pathComponents];
  unsigned count = [components count];
  NSDictionary *attrs = [NSDictionary dictionaryWithObject:
    [NSNumber numberWithUnsignedLong:0700] forKey:NSFilePosixPermissions];

  // Rebuild the path one component at a time
  for (unsigned i = 0; i < count; ++i) {
    NSString *subPath = [components objectAtIndex:i];
    
    // We can skip trailing "/"
    if ([subPath isEqualToString:@"/"])
      continue;
    
    path = [path stringByAppendingPathComponent:subPath];
    
    if (![mgr createDirectoryAtPath:path attributes:attrs]) {
      LOG(("Unable to create directory: %s", 
             [path cStringUsingEncoding:NSUTF8StringEncoding]));
      return NO;
    }
  }
  
  return YES;
}

//------------------------------------------------------------------------------
+ (NSString *)pathForFolder:(OSType)folderType domain:(short)domain 
                     create:(BOOL)create {
  NSString *dir = nil;
  FSRef fileRef;
  
  if (FSFindFolder(domain, folderType, create, &fileRef) == noErr) {
    CFURLRef url = CFURLCreateFromFSRef(NULL, &fileRef);
    
    if (url) {
      dir = (NSString *)CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
      [dir autorelease];
      CFRelease(url);
    }
  }
  
  return dir;
}

//------------------------------------------------------------------------------
+ (NSString *)gearsResourcesDirectory {
  NSBundle *pluginBundle = [NSBundle bundleForClass:[self class]];
  
  return [pluginBundle resourcePath];
}

//------------------------------------------------------------------------------
+ (NSString *)gearsDataDirectory {
  NSString *appDir = 
    [GearsPathUtilities pathForFolder:kApplicationSupportFolderType 
                               domain:kUserDomain create:YES]; 
  NSString *subdir = [NSString stringWithFormat:@"Google/%s for Safari",
    PRODUCT_FRIENDLY_NAME_ASCII];

  return [appDir stringByAppendingPathComponent:subdir];
}

//------------------------------------------------------------------------------
+ (NSString *)gearsUniqueTempPath {
  NSString *tempDir = NSTemporaryDirectory();
  scoped_CFUUID uuid(CFUUIDCreate(NULL));
  NSString *uuidStr = (NSString *)CFUUIDCreateString(NULL, uuid.get());
  [uuidStr release];

  return [tempDir stringByAppendingPathComponent:uuidStr];
}

@end
