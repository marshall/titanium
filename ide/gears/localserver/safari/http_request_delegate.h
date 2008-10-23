// Copyright 2008, Google Inc.
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

#ifndef GEARS_LOCALSERVER_SAFARI_HTTP_REQUEST_DELEGATE_H__
#define GEARS_LOCALSERVER_SAFARI_HTTP_REQUEST_DELEGATE_H__

// Wrapper for an NSURLconnection - encapsulates as much of the Objective C
// code as possible, delegating everything else to an "owner" C++ object
// through which the rest of the Gears code interfaces
// This class relies on the HttpRequest "owner"  class to enforce data validity
// and invariants.
// Note: This class assumes the above holds and does not perform validation
// of it's arguments.

#import <Cocoa/Cocoa.h>
#import <Foundation/NSStream.h>

#include "gears/base/common/byte_store.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/safari/http_request_sf.h"

@interface HttpRequestDelegate : NSObject {
 @private
  SFHttpRequest *owner_;
  NSMutableURLRequest *request_;     // (strong)
  NSURLConnection *connection_;      // (strong)
  ByteStore *response_body_;         // (strong)
  NSInteger statusCode_;
  NSDictionary *headerDictionary_;   // (strong)
  NSString *mimeType_;               // (strong)
}

#pragma mark Public Instance methods

// Initializes a newly allocated instance.
//
// Arguments: owner - the C++ object that owns this delegate, we make use
// of the owner's SetReadyState() & AllowRedirect() methods to notify it
// of state changes and give it control of request redirection.
- (id)initWithOwner:(SFHttpRequest *)owner;

// Open a new connection.
//
// Arguments: full_url - the url to open the connection to.
//            method - "GET", "POST", "HEAD" or another HTTP method listed in 
//                     the W3C specification.
//
// Returns: true on success.
- (bool)open:(const std::string16 &)full_url
      method:(const std::string16 &)method;

// Send the request
//
// Arguments: post_data_stream - a stream containing the data to send if the
//                               |method| specified in the open call was 'POST'.
//            user_agent - the User Agent string to use when sending the 
//                         request.
//            headers - HTTP headers to send the request with.
//            bypass_browser_cache - whether or not to use the browser's cache
//                                   when processing the request.
//
// Returns: true on success.
- (bool)send:(NSInputStream *)post_data_stream
   userAgent:(const std::string16 &)user_agent
     headers:(const SFHttpRequest::HttpHeaderVector &)headers
     bypassBrowserCache:(bool)bypass_browser_cache
     sendBrowserCookies:(bool)send_browser_cookies;

// Abort a request.
// Behavior is undefined if called multiple times.
- (void)abort;

#pragma mark Public Instance methods -- Access Methods

// These methods should only be called after the connection is closed.

// Get the headers received from the server.
- (void)headers:(SFHttpRequest::HttpHeaderVector *)headers;

// Retrieve a named header.
// If the header specified by name doesn't exist, |value| is cleared.
- (void)headerByName:(const std::string16 &)name 
               value:(std::string16 *)value;

// Get the HTTP status code.
- (int)statusCode;

// Get the MIME Type.
- (void)mimeType:(std::string16 *)mime_type_str;

// Get human readable text associated with the status code.
- (void)statusText:(std::string16 *)status_line;

// Retrieve the response body as a blob.
- (void)responseBody:(scoped_refptr<BlobInterface> *)body;

@end
#endif  // GEARS_LOCALSERVER_SAFARI_HTTP_REQUEST_DELEGATE_H__
