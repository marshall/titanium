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

#import <Cocoa/Cocoa.h>
#import "gears/localserver/safari/http_cookies_sf.h"

CFStringRef GetHTTPCookieString(CFStringRef urlStr) {
  NSURL *url = [NSURL URLWithString:(NSString *)urlStr];
  NSHTTPCookieStorage *cs = [NSHTTPCookieStorage sharedHTTPCookieStorage];
  NSArray *cookies = [cs cookiesForURL:url];
  unsigned count = [cookies count];

  // The NSHTTPCookie class method requestHeaderFieldsWithCookies: will generate
  // a dictionary with the NSString value of the "Cookie" key made by
  // of the concatenation of the name/value pairs from each cookie, with
  // semicolon delimiter between pairs.
  // However, constructing the string manually gives us control over the format
  // in case Apple's implementation or Gears's expected format changes.
  if (!count)
    return CFSTR("");

  NSMutableString *cookieStr = [NSMutableString string];

  for (unsigned i = 0; i < count; ++i) {
    NSHTTPCookie *cookie = [cookies objectAtIndex:i];
    NSString *name = [cookie name];
    NSString *value = [cookie value];
    const char *delim = (i < (count - 1)) ? ";" : ""; // No trailing semicolon

    if ([value length])
      [cookieStr appendFormat:@"%@=%@%s", name, value, delim];
    else
      [cookieStr appendFormat:@"%@%s", name, delim];
  }

  return CFStringCreateCopy(NULL, (CFStringRef)cookieStr);
}
