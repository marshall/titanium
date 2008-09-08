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

#include <assert.h>
#include <vector>
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/http_cookies.h"

//------------------------------------------------------------------------------
// Browser-independent code
//------------------------------------------------------------------------------

const std::string16 kNegatedRequiredCookieValue(STRING16(L";NONE;"));

static const std::string16 kCookieDelimiter(STRING16(L";"));

void ParseCookieString(const std::string16 &cookies, CookieMap *map) {
  map->clear();
  std::vector<std::string16> tokens;
  Tokenize(cookies, kCookieDelimiter, &tokens);
  for (int i = 0; i < static_cast<int>(tokens.size()); ++i) {
    std::string16 name, value;
    ParseCookieNameAndValue(tokens[i], &name, &value);
    if (!name.empty()) {
      // If the map already has a more specific entry for this name, don't
      // add this less specific value
      if (!map->HasCookie(name)) {
        (*map)[name] = value;
      }
    }
  }
}

bool CookieMap::LoadMapForUrl(const char16 *url, BrowsingContext *context) {
  std::string16 cookies_string;
  if (!GetCookieString(url, context, &cookies_string))
    return false;
  ParseCookieString(cookies_string, this);
  return true;
}

bool CookieMap::GetCookie(const std::string16 &cookie_name,
                          std::string16 *cookie_value) {
  const_iterator found = find(cookie_name);
  if (found == end())
    return false;
  *cookie_value = found->second;
  return true;
}

bool CookieMap::HasCookie(const std::string16 &cookie_name) {
  const_iterator found = find(cookie_name);
  return  found != end();
}

bool CookieMap::HasSpecificCookie(const std::string16 &cookie_name,
                                  const std::string16 &cookie_value) {
  const_iterator found = find(cookie_name);
  if (found == end())
    return false;
  return cookie_value == found->second;
}

bool CookieMap::HasLocalServerRequiredCookie(
                    const std::string16 &required_cookie) {
  if (required_cookie.empty())
    return true;

  std::string16 name, value;
  ParseCookieNameAndValue(required_cookie, &name, &value);
  if (name.empty())
    return false;

  return (value == kNegatedRequiredCookieValue)
                       ? !HasCookie(name) : HasSpecificCookie(name, value);
}

void ParseCookieNameAndValue(const std::string16 &name_and_value,
                             std::string16 *name,
                             std::string16 *value) {
  // Some observations about cookie names and values
  // - names/values cannot have leading or trailing whitespace
  // - names/values can have embedded whitespace
  // - names/values cannot contain the ';' character
  // - values can contain embedded '=' and ',' charaters
  size_t equal_pos = name_and_value.find('=');
  if (equal_pos == std::string16::npos) {
    const char16 *start = name_and_value.c_str();
    int len = name_and_value.length();
    StripWhiteSpace(&start, &len);
    name->assign(start, len);
    value->clear();
  } else {
    const char16 *start = name_and_value.c_str();
    int len = static_cast<int>(equal_pos);
    StripWhiteSpace(&start, &len);
    name->assign(start, len);
    size_t value_pos = equal_pos + 1;
    if (value_pos < name_and_value.length() - 1) {
      start = name_and_value.c_str() + value_pos;
      len = name_and_value.length() - value_pos;
      StripWhiteSpace(&start, &len);
      value->assign(start, len);
    } else {
      value->clear();
    }
  }
}


#ifdef USING_CCTESTS
#include "gears/base/common/mutex.h"
static Mutex g_fake_lock;
static std::string16 g_fake_url;
static std::string16 g_fake_cookies;

void SetFakeCookieString(const char16* url, const char16 *cookies) {
  MutexLock locker(&g_fake_lock);
  const char16 *kEmptyString = STRING16(L"");
  g_fake_url = url ? url : kEmptyString;
  g_fake_cookies = cookies ? cookies : kEmptyString;
}
#endif

#ifdef USING_CCTESTS
static bool GetFakeCookieString(const char16 *url, std::string16 *cookies) {
  MutexLock locker(&g_fake_lock);
  if (url == g_fake_url) {
    *cookies = g_fake_cookies;
    return true;
  } else {
    return false;
  }
}
#endif

//------------------------------------------------------------------------------
// Browser-dependent code
//------------------------------------------------------------------------------
#if BROWSER_IE
#include <windows.h>
#include <wininet.h>
#include "gears/base/ie/atl_headers.h"

bool GetCookieString(const char16 *url, BrowsingContext *context,
                     std::string16 *cookies_out) {
  assert(url);
  assert(cookies_out);
#ifdef USING_CCTESTS
  if (GetFakeCookieString(url, cookies_out)) {
    return true;
  }
#endif

  cookies_out->clear();

  // To defend against the cookie string changing between the first and second
  // calls to InternetGetCookieW, we loop if we need to
  DWORD last_len = 0xffffffff;
  while (true) {
    DWORD len = 0;
    if (!InternetGetCookieW(url, NULL, NULL, &len)) {
      DWORD last_error = GetLastError();
      // FALSE is returned when the cookie string is empty (error code is
      // ERROR_NO_MORE_ITEMS), but this is not an error condition. Return true
      // and an empty string.
      //
      // On WinCE, FALSE is also returned when we pass NULL for the data string.
      // The error code is ERROR_INSUFFICIENT_BUFFER, but the length parameter
      // is set correctly. In this case, we continue getting the cookie.
      //
      // If the error is anything else, return false.
      if (ERROR_NO_MORE_ITEMS == last_error) {
        return true;
      } else if (ERROR_INSUFFICIENT_BUFFER != last_error) {
        return false;
      }
    }

    if (len == last_len)
      return false;  // don't loop forever
    else
      last_len = len;

    CStringW cookies_str;
    BOOL ok = InternetGetCookieW(url, NULL, cookies_str.GetBuffer(len + 1),
                                 &len);
    DWORD last_error = GetLastError();
    cookies_str.ReleaseBuffer(len);
    if (ok) {
      cookies_out->assign(cookies_str.GetString());
      return true;
    } else if (last_error == ERROR_NO_MORE_ITEMS) {
      return true;
    } else if (last_error == ERROR_INSUFFICIENT_BUFFER) {
      continue;  // loop and try again with a larger buffer
    } else {
      return false;
    }
  }

  // GetBuffer(len + 1) followied ReleaseBuffer(len) looks odd, here's
  // what's going on with that ATL::CString wart.
  // For GetBuffer, len is the minimum size of the character buffer in
  // characters. This value does not include space for a null terminator.
  // For ReleaseBuffer, len is the new length of the string in characters,
  // not counting a null terminator
  // So you have to leave room for the NULL when calling GetBuffer yourself.
}

//------------------------------------------------------------------------------
#elif BROWSER_FF
#ifdef WIN32
#include <windows.h>  // must manually #include before nsIEventQueueService.h
#endif
#include <gecko_sdk/include/nsMemory.h>
#include <gecko_sdk/include/nsIURI.h>
#include <gecko_sdk/include/nsIIOService.h>
#include <gecko_internal/nsICookieService.h>
#if BROWSER_FF3
#include <gecko_internal/nsXPCOMCIDInternal.h>
#include <gecko_sdk/include/nsIHttpChannel.h>
#include "gears/localserver/firefox/http_request_ff.h"
#else
#include <gecko_internal/nsIEventQueueService.h>
#endif
#include <gecko_internal/nsIProxyObjectManager.h>
#include "gears/base/common/common.h"

bool GetCookieString(const char16 *url, BrowsingContext *context,
                     std::string16 *cookies_out) {
  assert(url);
  assert(cookies_out);
#ifdef USING_CCTESTS
  if (GetFakeCookieString(url, cookies_out)) {
    return true;
  }
#endif

  cookies_out->clear();

  // Get the ioservice which we need to create nsIURIs
  nsCOMPtr<nsIIOService> ios =
      do_GetService("@mozilla.org/network/io-service;1");
  if (!ios) { return false; }

  // Convert url to utf8 nsCString, so we can create an nsIURI
  nsString url_str;
  nsCString url_utf8;
  url_str.Assign(url);
  nsresult nr = NS_UTF16ToCString(url_str, NS_CSTRING_ENCODING_UTF8, url_utf8);
  NS_ENSURE_SUCCESS(nr, false);

  // Manufacture an nsIURI, so we can get the cookie string
  nsCOMPtr<nsIURI> url_obj;
  nr = ios->NewURI(url_utf8, nsnull, nsnull, getter_AddRefs(url_obj));
  NS_ENSURE_SUCCESS(nr, false);

  // Get the cookie service
  nsCOMPtr<nsICookieService> cookie_service =
      do_GetService("@mozilla.org/cookieService;1");
  if (!cookie_service) return false;

  // Make a proxy for it in case we're being called form a worker thread.
  // If we're on the main thread, we'll call straight through.
  // from nsProxyEvent.h
  #define PROXY_SYNC    0x0001  // acts just like a function call
  #define PROXY_ASYNC   0x0002  // fire and forget
  #define PROXY_ALWAYS  0x0004  // ignore check to see if the eventQ
                                // is on the same thread as the caller
#if BROWSER_FF3
  #define PROXY_TO_MAIN_THREAD NS_PROXY_TO_MAIN_THREAD
#else
  #define PROXY_TO_MAIN_THREAD NS_UI_THREAD_EVENTQ
#endif
  nsCOMPtr<nsIProxyObjectManager> proxy_manager =
      do_GetService(NS_XPCOMPROXY_CONTRACTID);
  if (!proxy_manager) return false;
  nsCOMPtr<nsICookieService> cookie_service_proxy;
  nr = proxy_manager->GetProxyForObject(PROXY_TO_MAIN_THREAD,
                                        NS_GET_IID(nsICookieService),
                                        cookie_service,
                                        PROXY_SYNC,
                                        getter_AddRefs(cookie_service_proxy));
  if (!cookie_service_proxy || NS_FAILED(nr)) return false;
  if (cookie_service_proxy != cookie_service) {
    LOG(("WARNING: GetCookieString not called on the UI thread"));
  }
#if BROWSER_FF3
  nsCOMPtr<nsIChannel> channel;
  scoped_refptr<GearsLoadGroup> load_group(new GearsLoadGroup);
  nr = ios->NewChannel(url_utf8, nsnull, nsnull, getter_AddRefs(channel));
  NS_ENSURE_SUCCESS(nr, false);
  NS_ENSURE_TRUE(channel, false);

  channel->SetLoadGroup(load_group.get());
  channel->SetLoadFlags(nsIChannel::LOAD_DOCUMENT_URI);
#else
  nsIChannel *channel = NULL;
#endif

  // Finally, call the cookie service
  char *cookies_str = NULL;
  nr = cookie_service_proxy->GetCookieString(url_obj, channel, &cookies_str);
  NS_ENSURE_SUCCESS(nr, false);

  // A return value of NS_OK and a NULL cookie_str means an empty string
  if (!cookies_str) {
    assert(cookies_out->empty());  // was cleared on function entry
    return true;
  }

  // One last hoop, convert to string16
  bool rv = UTF8ToString16(cookies_str, cookies_out);
  nsMemory::Free(cookies_str);

  return rv;
}

//------------------------------------------------------------------------------

#elif BROWSER_SAFARI
#include "gears/base/common/string_utils_osx.h"
#include "gears/base/safari/scoped_cf.h"
#include "gears/localserver/safari/http_cookies_sf.h"

bool GetCookieString(const char16 *url, BrowsingContext *context,
                     std::string16 *cookies_out) {
  assert(url);
  assert(cookies_out);
#ifdef USING_CCTESTS
  if (GetFakeCookieString(url, cookies_out)) {
    return true;
  }
#endif

  scoped_CFString url_cfstr(CFStringCreateWithString16(url));
  scoped_CFString cookie_cfstr(GetHTTPCookieString(url_cfstr.get()));

  return CFStringRefToString16(cookie_cfstr.get(), cookies_out);
}

#elif defined(OS_ANDROID)
#include "gears/localserver/android/http_request_android.h"

bool GetCookieString(const char16 *url, BrowsingContext *context,
                     std::string16 *cookies_out) {
  assert(url);
  assert(cookies_out);
#ifdef USING_CCTESTS
  if (GetFakeCookieString(url, cookies_out)) {
    return true;
  }
#endif
  // Just call the handler in HttpRequestAndroid.
  *cookies_out = HttpRequestAndroid::GetCookieForUrl(url);
  return true;
}

#elif BROWSER_NPAPI

bool GetCookieString(const char16 *url, BrowsingContext *context,
                     std::string16 *cookies_out) {
  // TODO(mpcomplete): Uh oh... how do we get cookies in NPAPI?
  return false;
}

#endif
