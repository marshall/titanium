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

#import "gears/base/common/string_utils.h"
#import "gears/base/common/http_utils.h"
#import "gears/base/safari/nsstring_utils.h"
#import "gears/localserver/common/http_constants.h"
#import "gears/localserver/common/localserver_db.h"
#import "gears/localserver/safari/localserver_db_proxy.h"

@implementation GearsWebCacheDB
//------------------------------------------------------------------------------
+ (BOOL)canService:(NSURLRequest *)request {
  static NSString *bypass_localserver_header_name =  
      (NSString *)CFStringCreateWithString16(
                      HttpConstants::kXGoogleGearsBypassLocalServer);

  // Check if the bypasslocalserver header field exists.
  if ([request respondsToSelector:@selector(valueForHTTPHeaderField:)]) {
    if ([request valueForHTTPHeaderField:bypass_localserver_header_name] 
        != nil) {
      return NO;
    }
  }

  NSURL *url = [request URL];
  NSString *urlStr = [url absoluteString];
  std::string16 url16;
  
  if ([urlStr string16:&url16]) {
    WebCacheDB *db = WebCacheDB::GetDB();
    
    if (db && db->CanService(url16.c_str(), NULL))
      return YES;
  }
  
  return NO;
}

//------------------------------------------------------------------------------
+ (NSData *)service:(NSURL *)url mimeType:(NSString **)mimeType
      headers:(NSDictionary **)headers statusCode:(int *)statusCode 
      redirectURL:(NSURL **)redirectURL {
  std::string16 url16;
  
  assert(url);
  assert(mimeType);
  assert(headers);
  assert(statusCode);
  assert(redirectURL);
  
  if (![[url absoluteString] string16:&url16])
    return nil;
  
  WebCacheDB *db = WebCacheDB::GetDB();
  WebCacheDB::PayloadInfo payload;
  
  if (!db->Service(url16.c_str(), NULL, false, &payload))
    return nil;

  // The requested url may redirect to another location
  std::string16 redirect_url;
  if (payload.IsHttpRedirect()) {
    // We collapse a chain of redirects and hop directly to the final 
    // location for which we have a cache entry.
    int num_redirects = 0;
    while (payload.IsHttpRedirect()) {
      if (!payload.GetHeader(HttpConstants::kLocationHeader, &redirect_url))
        return nil;

      // Fetch a response for redirect_url from our DB
      if (!db->Service(redirect_url.c_str(), NULL, false, &payload))
        return nil;
        
      // Make sure we don't get stuck in an infinite redirect loop.
      ++num_redirects;
      if (num_redirects > HttpConstants::kMaxRedirects) {
        LOG16((STRING16(L"Redirect chain too long %s -> %s"),
               url16.c_str(), redirect_url.c_str()));
        return nil;
      }
    }
    
    *redirectURL = [NSURL URLWithString:
      [NSString stringWithString16:redirect_url.c_str()]];
  } else {
    *redirectURL = nil;
  }
  
  std::string16 header16;
  
  if (payload.GetHeader(HttpConstants::kXGearsSafariCapturedMimeType, 
      &header16))
    *mimeType = [NSString stringWithString16:header16.c_str()];
  else
    *mimeType = nil;
 
  *statusCode = payload.status_code;
  NSMutableDictionary *ret_headers = nil;
  
  // Crack header string and convert to NSDictionary.
  // TODO(playmobil): If this turns out to be a performance bottleneck, we may
  // want to just store the cracked header in the payload rather than re-parsing
  // them on every request.
  const std::string16 &headers_utf16 = payload.headers;
  if (!headers_utf16.empty()) {
    std::string headers_utf8;
    if (!String16ToUTF8(headers_utf16.c_str(), 
                        headers_utf16.length(),
                        &headers_utf8)) {
      return nil;
    }
    
    HTTPHeaders parsed_headers;
    const char *body = headers_utf8.c_str();
    uint32 bodylen = headers_utf8.length();
    if (!HTTPUtils::ParseHTTPHeaders(&body, &bodylen, &parsed_headers, true)) {
      return nil;
    }
    ret_headers = [[[NSMutableDictionary alloc] init] autorelease];
    for (HTTPHeaders::const_iterator it = parsed_headers.begin();
         it != parsed_headers.end();
         ++it) {
      NSString *header_name = [NSString stringWithUTF8String:it->first];
      NSString *header_value = [NSString stringWithUTF8String:it->second];
      [ret_headers setValue:header_value forKey:header_name];
    }
  }
  *headers = ret_headers;
  
  // Copy the data
  std::vector<unsigned char> *dataVec = payload.data.get();
  const unsigned char *bytes = dataVec ? &(*dataVec)[0] : NULL;
  unsigned int length = dataVec ? dataVec->size() : 0;

  return [NSData dataWithBytes:bytes length:length];
}

@end

