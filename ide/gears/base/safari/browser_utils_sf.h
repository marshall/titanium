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

// Utility functions / methods operating on URLs

#ifndef GEARS_BASE_SAFARI_BROWSER_UTILS_SF_H__
#define GEARS_BASE_SAFARI_BROWSER_UTILS_SF_H__

#if defined(__OBJC__)
#import <Cocoa/Cocoa.h>
@interface GearsURLUtilities : NSObject
// Return URL absoluteString for |relativeURLStr| on the current page
+ (NSString *)resolveURLString:(NSString *)relativeURLStr
               usingPluginArgs:(NSDictionary *)args;

+ (NSString *)resolveURLString:(NSString *)relativeURLStr
                    baseURLStr:(NSString *)baseURLStr;
@end
#endif  // __OBJC__

#if defined(__cplusplus)
#include <CoreFoundation/CoreFoundation.h>
#include "gears/base/common/string_utils.h"

class SecurityOrigin;

// Convert a CFURLRef into a String16 representation, returning true if 
// successful, false otherwise
bool CFURLRefToString16(CFURLRef url, std::string16 *out16);

// Convert a String16 string into a CFURLRef.  Caller is responsible for
// releasing CFURLRef.
CFURLRef CFURLCreateWithString16(const char16 *url_str);

namespace SafariURLUtilities {
  bool GetPageOrigin(const char *url_str, SecurityOrigin *security_origin);  
  bool GetPageOriginFromArguments(CFDictionaryRef dict, 
                                  SecurityOrigin *security_origin);
}

#endif  // __cplusplus
#endif  // GEARS_BASE_SAFARI_BROWSER_UTILS_SF_H__
