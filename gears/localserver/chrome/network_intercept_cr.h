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

#ifndef GEARS_LOCALSERVER_CHROME_NETWORK_INTERCEPT_CR_H__
#define GEARS_LOCALSERVER_CHROME_NETWORK_INTERCEPT_CR_H__

#include "gears/base/common/string16.h"
#include "gears/localserver/common/localserver_db.h"
#include "third_party/chrome/chrome_plugin_api.h"

// This class handles browser network interception.
class NetworkIntercept {
public:
  static NetworkIntercept* FromCPRequest(CPRequest* request) {
    return reinterpret_cast<NetworkIntercept*>(request->pdata);
  }

  // Returns a new intercept object for the given URL, which will provide
  // the response metadata and body, or NULL of we have no response.
  static NetworkIntercept *Intercept(CPRequest *request,
                                     const std::string16 &url,
                                     BrowsingContext *context);

  // Queries a specific piece of metadata, and stores the result in 'buf'.
  // Satisfies the Chrome plugin API CPP_GetResponseInfo.
  int GetResponseInfo(CPResponseInfoType type, void* buf, uint32 buf_size);

  // Reads up to 'buf_size' more data into 'buf'.
  // Satisfies the Chrome plugin API CPP_Read.
  int Read(void* buf, uint32 buf_size);

private:
  NetworkIntercept(WebCacheDB::PayloadInfo* payload);

  // The response payload.
  scoped_ptr<WebCacheDB::PayloadInfo> payload_;

  // Our current offset into the response data, used by Read.
  uint64 offset_;
};

#endif // GEARS_LOCALSERVER_CHROME_NETWORK_INTERCEPT_CR_H__
