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

#import "gears/base/common/common_sf.h"
#import "gears/base/safari/loader.h"
#import "gears/base/safari/safari_workarounds.h"
#import "gears/localserver/safari/http_handler.h"

// The NSHTTPURLResponse doesn't have a way to set the status code or headers, 
// so we'll subclass and override the statusCode & allHeaderFields methods.
@interface GearsNSHTTPURLResponse : NSHTTPURLResponse {
  int statusCode_;
  NSDictionary *headers_;  // strong
}
@end

@implementation GearsNSHTTPURLResponse
- (void)dealloc {
  [headers_ release];
  [super dealloc];
}

- (void)setStatusCode:(int)code {
  statusCode_ = code;
}

- (int)statusCode {
  return statusCode_ ? statusCode_ : [super statusCode];
}

- (void)setHeaders:(NSDictionary *)headers {
  [headers_ autorelease];
  headers_ = [headers retain];
}

- (NSDictionary *)allHeaderFields {
  return headers_ ? [[headers_ copy] autorelease] : [super allHeaderFields];
}

@end

// The actual GearsWebCacheDB is not linked in with this file.  It is looked up
// at runtime.  This category on NSObject keeps the compiler happy.
@interface NSObject(GearsWebCacheDBInterface)
+ (BOOL)canService:(NSURLRequest *)request;
+ (NSData *)service:(NSURL *)url mimeType:(NSString **)mimeType
      headers:(NSDictionary **)headers statusCode:(int *)statusCode 
      redirectURL:(NSURL **)redirectURL;
@end

@implementation GearsHTTPHandler
//------------------------------------------------------------------------------
+ (BOOL)registerHandler {
  return [NSURLProtocol registerClass:[self class]];
}

//------------------------------------------------------------------------------
+ (NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request {
  // Must be implemented, but return default request.  This method will only 
  // be called after canInitWithRequest: has returned YES, indicating that 
  // Gears will be processing the request.
  return request;
}

//------------------------------------------------------------------------------
+ (BOOL)canInitWithRequest:(NSURLRequest *)request {
  Class webCacheClass = NSClassFromString(@"GearsWebCacheDB");  
  return [webCacheClass canService:request];
 }

//------------------------------------------------------------------------------
- (void)startLoading {
  NSString *mimeType = nil;
  NSURL *url = [[self request] URL];
  int statusCode = 0;
  Class webCacheClass = NSClassFromString(@"GearsWebCacheDB");
  NSURL *redirectURL = nil;
  NSDictionary *headers = nil;
  NSData *data = [webCacheClass service:url 
                               mimeType:&mimeType 
                               headers:&headers
                             statusCode:&statusCode 
                            redirectURL:&redirectURL];
  
  if (!data) {
    // Send a failed response
    NSDictionary *userInfo = 
      [NSDictionary dictionaryWithObject:url forKey:NSURLErrorKey];
    NSError *error = [NSError errorWithDomain:NSCocoaErrorDomain code:404 
                                     userInfo:userInfo];
    [[self client] URLProtocol:self didFailWithError:error];
    [[self client] URLProtocolDidFinishLoading:self];
    return;
  }
  
  if (!mimeType)
    mimeType = @"text/html";
  // Create a HTTP response with the cached status code
  NSURLResponse *response;
  
  // Workaround for rdar://problem/5817126 see base/safari/safari_workaround.m
  // for more details.
  if (IsLeopardOrGreater()) {
    NSMutableDictionary *new_headers = [[headers mutableCopy] autorelease];
    [new_headers setObject:mimeType forKey:@"Content-Type"];
    response = LeopardCreateNSURLResponse(url, statusCode, new_headers);
  } else {
    GearsNSHTTPURLResponse *g_response;
    g_response = [[GearsNSHTTPURLResponse alloc] initWithURL:url 
                                                  MIMEType:mimeType
                                     expectedContentLength:[data length]
                                          textEncodingName:nil];
    
    [g_response setStatusCode:statusCode];
    
    // WebCache should always contain some content for the headers.
    assert(headers);
    [g_response setHeaders:headers];
    response = g_response;
  }
  
  // Notify the client if there was a redirect or just a normal response
  if (redirectURL) {
    [[self client] URLProtocol:self 
        wasRedirectedToRequest:[NSURLRequest requestWithURL:redirectURL]
              redirectResponse:response];
  } else {
    [[self client] URLProtocol:self didReceiveResponse:response
            cacheStoragePolicy:NSURLCacheStorageNotAllowed];    
    // Notify the client of the data
    [[self client] URLProtocol:self didLoadData:data];
  }
  
  [[self client] URLProtocolDidFinishLoading:self];

  [response release];
}

//------------------------------------------------------------------------------
- (void)stopLoading {
  // Implementation of this method is required, but in our case it does 
  // nothing.
}

@end
