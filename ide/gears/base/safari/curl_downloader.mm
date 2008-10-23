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

#import <Cocoa/Cocoa.h>
#import </usr/include/curl/curl.h>

#import "gears/base/common/mutex.h"
#import "gears/base/common/scoped_token.h"
#import "gears/base/npapi/browser_utils.h"
#import "gears/base/safari/curl_downloader.h"
#import "gears/localserver/common/http_cookies.h"

static bool curl_was_initialised = false;
static Mutex curl_init_mutex;

// Called by CURL when new data is available.
static size_t write_data_callback(void *buffer, size_t size, size_t nmemb, 
                                  void *closure) {
  NSMutableData *response_bytes = (NSMutableData *)closure;
  [response_bytes appendBytes:buffer length:nmemb*size];
  return nmemb;
}

typedef DECLARE_SCOPED_TRAITS(CURL*, curl_easy_cleanup, NULL) CURLTraits;
typedef scoped_token<CURL*, CURLTraits> ScopedCURLHandle;

bool GetURLDataAsVector(const std::string16 &url, 
                         const std::string16 &user_agent,
                         std::vector<uint8> *data) {
  assert(data);
  assert(!url.empty());
  
  NSMutableData *response_bytes = [[[NSMutableData alloc] init] autorelease];
  
  if (!GetURLDataAsNSData(url, user_agent, response_bytes)) return false;
  
  // Copy out data.
  data->resize([response_bytes length]);  
  [response_bytes getBytes:&((*data)[0])];
  return true;
}

bool GetURLDataAsNSData(const std::string16 &url,
                        const std::string16 &user_agent,
                        NSMutableData *data) {
  assert(data);
  assert(!url.empty());
  
  std::string url_utf8;
  if (!String16ToUTF8(url.c_str(), url.length(), &url_utf8)) {
    return false;
  }
  
  // Set user_agent, cookies, and headers.
  NSMutableData *response_bytes = data;

  CURLcode ret = CURLE_OK;
  // Initialize CURL on first run, this code may be called from HTMLDialog
  // which means it must be threadsafe.
  {
    MutexLock lock(&curl_init_mutex);
    if (!curl_was_initialised) {
        ret = curl_global_init(CURL_GLOBAL_SSL);
        if (ret != CURLE_OK) return false;
        curl_was_initialised = true;
    }
  }
  
  // Create a CURL Handle for this request.
  ScopedCURLHandle handle(curl_easy_init());
  if (!handle.get()) {
    return false;
  }
  
  // Set global options.
  ret = curl_easy_setopt(handle.get(), CURLOPT_URL, url_utf8.c_str());
  if (ret != CURLE_OK) return false;
  ret = curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, 
                         write_data_callback);
  if (ret != CURLE_OK) return false;
  ret = curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, response_bytes);
  if (ret != CURLE_OK) return false;
  
  std::string user_agent_utf8;
  if (!String16ToUTF8(user_agent.c_str(), user_agent.length(), 
                      &user_agent_utf8)) {
    return false;
  }
  
  ret = curl_easy_setopt(handle.get(), CURLOPT_USERAGENT, 
                         user_agent_utf8.c_str());
  if (ret != CURLE_OK) return false;
  
  // Cookies.
  std::string16 cookie_utf16;
  if (!GetCookieString(url.c_str(), NULL, &cookie_utf16)) {
    return false;
  }
  
  // Fixup cookie formatting from ';' delimited to '; ' delimited.
  ReplaceAll(cookie_utf16,
             std::string16(STRING16(L";")),
             std::string16(STRING16(L"; ")));
  
  std::string cookie_utf8;
  if (!String16ToUTF8(cookie_utf16.c_str(), cookie_utf16.length(), 
                      &cookie_utf8)) {
    return false;
  }
  
  ret = curl_easy_setopt(handle.get(), CURLOPT_COOKIE, cookie_utf8.c_str());
  if (ret != CURLE_OK) return false;
  
  // Perform request.
  ret = curl_easy_perform(handle.get());
  if (ret != CURLE_OK) return false;

  return true;
}
