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
#import "gears/base/safari/loader.h"

@implementation GearsLoader
//------------------------------------------------------------------------------
+ (BOOL)canLoadGears {
  Class webViewClass = NSClassFromString(@"WebView");
  NSBundle *webkitBundle = [NSBundle bundleForClass:webViewClass];
  NSString *key = (NSString *)kCFBundleVersionKey;
  NSString *vers = [webkitBundle objectForInfoDictionaryKey:key];
  
  // Check the version of WebKit that the current application is using and
  // see if it's new enough to load with.
  
  // A (somewhat dated list) of Safari Version strings and equivalent
  // CFBundleVersions can be found here: 
  // http://developer.apple.com/internet/safari/uamatrix.html
  // It seems that the most significant digit is the OS Version, and the
  // rest denote the real safari version number.
  // Some values we've seen empirically:
  // 5523.15.1 - Safari 3.0.4 on 10.5.2
  // 4525.18   - Safari 3.1.1 on 10.4
  // 5525.18   - Safari 3.1.1 on 10.5
  // 4525.22   - Safari 3.1.2 on 10.4
  // 5525.18   - Safari 3.1.2 on 10.5
  int version = floor([vers floatValue]);
  
  // Strip off MSD, apparently corresponding to OS Version.
  version = version % 1000;
  
  // 525 is Safari 3.1.1
  if (version >= 525)
    return YES;
  
  NSLog(@"%s requires WebKit 4525 or later (Current: %d)",
        PRODUCT_FRIENDLY_NAME_ASCII, [vers intValue]);
  return NO;
}

//------------------------------------------------------------------------------
+ (NSString *)gearsBundlePath {
  NSArray *paths = 
    NSSearchPathForDirectoriesInDomains(
        NSLibraryDirectory, 
        (NSSearchPathDomainMask)(NSUserDomainMask | NSLocalDomainMask),
        YES);
  
  // Find the first one that exists
  unsigned int i, count = [paths count];
  NSFileManager *fileManager = [NSFileManager defaultManager];
  for (i = 0; i < count; ++i) {
    NSString *path = [paths objectAtIndex:i];
    NSString *internetPlugins = @"Internet Plug-Ins";
    
    path = [path stringByAppendingPathComponent:internetPlugins];
    
    BOOL isDir;
    NSString *ident = [NSString stringWithFormat:@"%s.plugin",
      PRODUCT_SHORT_NAME_ASCII];
    NSString *tmp_path = [path stringByAppendingPathComponent:ident];
    if (![fileManager fileExistsAtPath:tmp_path isDirectory:&isDir] || !isDir)
      continue;
      
    // Work around a really bizarre bug where if the case of a filename
    // is different, DYLD will load the bundle twice into a process.
    // This causes all kinds of mayhem, e.g. pthread_once will fire twice...
    NSArray *plugin_dir_contents = [fileManager directoryContentsAtPath:path];
    NSEnumerator *contents = [plugin_dir_contents objectEnumerator];
    while (id filename = [contents nextObject]) {
      if ([ident caseInsensitiveCompare:filename] == NSOrderedSame) {
        path = [path stringByAppendingPathComponent:filename];
        return path;
      }
    }
  }
  
  return nil;
}

//------------------------------------------------------------------------------
+ (BOOL)loadGearsBundle {
  NSBundle *bundle = [NSBundle bundleWithPath:[GearsLoader gearsBundlePath]];
  
  return [bundle load];
}

//------------------------------------------------------------------------------

@end
