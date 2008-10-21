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

#ifndef GEARS_LOCALSERVER_SAFARI_HTTP_HANDLER_H__
#define GEARS_LOCALSERVER_SAFARI_HTTP_HANDLER_H__

#if defined(__OBJC__)
#import <Foundation/Foundation.h>

// Protocol handler to intercept requests for pages/data that are in the Gears
// cache.  If Gears cannot handle the URL, canInitWithRequest will return NO.
// Otherwise, it will return YES and the Cocoa URL loading will then call the
// startLoading method to fetch the Gears pages.  Redirects are handled via
// the NSURLProcotoclClient delegate method 
// URLProtocol:wasRedirectedToRequest:redirectResponse:

@interface GearsHTTPHandler : NSURLProtocol
//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------
+ (BOOL)registerHandler;

//------------------------------------------------------------------------------
// NSURLProtocol
//------------------------------------------------------------------------------
+ (NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request;
+ (BOOL)canInitWithRequest:(NSURLRequest *)request;

- (void)startLoading;
- (void)stopLoading;

@end
#endif  // __OBJC__

#endif  // GEARS_LOCALSERVER_SAFARI_HTTP_HANDLER_H__
