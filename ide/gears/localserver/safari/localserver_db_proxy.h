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
#ifndef GEARS_LOCALSERVER_SAFARI_LOCALSERVER_DB_PROXY_H__
#define GEARS_LOCALSERVER_SAFARI_LOCALSERVER_DB_PROXY_H__

// Provide a Cocoa wrapper to the WebCacheDB functions required to check and
// fetch stored content.  This is used so that the Enabler (InputManager)
// doesn't have to link against all of Gears

#import <Foundation/Foundation.h>

// See description above
@interface GearsWebCacheDB : NSObject

// Return YES if |request|'s url is stored locally.
+ (BOOL)canService:(NSURLRequest *)request;

// Return the data for |url| and |mimeType| (if specified).  If the |url| had
// a redirect, |redirectURL| will be initialized to the final destination.
// |statusCode| indicates the code received when saved into the WebCacheDB.
// |headers| are the request's HTTP Headers, or nil if none are found.
+ (NSData *)service:(NSURL *)url mimeType:(NSString **)mimeType 
      headers:(NSDictionary **)headers statusCode:(int *)statusCode 
      redirectURL:(NSURL **)redirectURL;

@end
#endif  // GEARS_LOCALSERVER_SAFARI_LOCALSERVER_DB_PROXY_H__
