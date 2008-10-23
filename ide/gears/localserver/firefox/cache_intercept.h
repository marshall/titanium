// Copyright 2005, Google Inc.
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

#ifndef GEARS_LOCALSERVER_FIREFOX_CACHE_INTERCEPT_H__
#define GEARS_LOCALSERVER_FIREFOX_CACHE_INTERCEPT_H__

#include <gecko_sdk/include/nsIObserver.h>
#include <gecko_sdk/include/nsIChannel.h>
#include <gecko_internal/nsICacheService.h>
#include "gears/base/common/common_ff.h"

class FFHttpRequest;

// Object identifiers
extern const char *kCacheInterceptContractId;
extern const char *kCacheInterceptClassName;
extern const nsCID kCacheInterceptClassId;


//-----------------------------------------------------------------------------
// CacheIntercept
//-----------------------------------------------------------------------------
class CacheIntercept : public nsICacheService, public nsIObserver {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICACHESERVICE
  NS_DECL_NSIOBSERVER

  // Only intercept HTTP cache usage.
  PRBool MayIntercept(const char *clientID) {
    return clientID && (!strcmp(clientID, "HTTP") ||
                        !strcmp(clientID, "HTTP-memory-only"));
  }

 private:
  nsCOMPtr<nsICacheService> default_cache_;  // The actual necko cache

  // Initialization. Called at XPCOM startup time. We replace the default
  // cache service and start listening for notifications from the observer
  // service.
  void Init();

  // Force requests which are set to bypass the cache, but which Gears has a
  // response for, to go to Gears's cache.
  void MaybeForceToCache(nsISupports *request);

  // Helper to determine if a given request was initiated by Gears and
  // to return a pointer to the C++ instance.
  static FFHttpRequest *GetGearsHttpRequest(nsIChannel *channel);
};

#endif // GEARS_LOCALSERVER_FIREFOX_CACHE_INTERCEPT_H__
