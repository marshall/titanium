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

// Implementation of the PathUtils.h common utilities

#import "gears/base/common/paths.h"
#import "gears/base/common/paths_sf_more.h"
#import "gears/base/common/string_utils_osx.h"
#import "gears/base/safari/nsstring_utils.h"
  
bool GetInstallDirectory(std::string16 *path) {
  // TODO(aa): Implement me when needed.
  return false;
}

bool GetBaseDataDirectory(std::string16 *path) {
  NSString *dir = [GearsPathUtilities gearsDataDirectory];
  return CFStringRefToString16((CFStringRef)dir, path);
}

bool GetBaseResourcesDirectory(std::string16 *path) {
  NSString *dir = [GearsPathUtilities gearsResourcesDirectory];
  return CFStringRefToString16((CFStringRef)dir, path);
}

bool GetUserHomeDirectory(std::string16 *home_dir) {
  return [[@"~" stringByExpandingTildeInPath] string16:home_dir];
}

bool GetUserTempDirectory(std::string16 *tmp_dir) {
  // As of OSX 10.4+ this is a per-user temp directory.
  return [NSTemporaryDirectory() string16:tmp_dir];
}

bool GetComponentDirectory(std::string16 *path) {
  return GetBaseResourcesDirectory(path);
}
