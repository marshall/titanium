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
//
// The strings handled here correspond to the document.cookies
// property in JavaScript, values look like:
//
//        "name=value;nameOnly;name=lessSpecificValueForName"
//
// We tokenize the strings at the semi-colon and then parse each
// token for name and value stripped of leading and trailing whitespace.
//
// When a cookie is listed more than once, only the first occurence is
// extracted. According to RFC2109, cookies are listed in order of 
// specificity in the cookie string.
//
// Nameless (or valueless depending on how you look at it) cookies are
// allowed. These functions treat them as "valueless named cookies".

#ifndef GEARS_LOCALSERVER_COMMON_HTTP_COOKIES_H__
#define GEARS_LOCALSERVER_COMMON_HTTP_COOKIES_H__

#include <map>
#include "gears/base/common/string16.h"

extern const std::string16 kNegatedRequiredCookieValue;

class BrowsingContext;
class CookieMap;

// Retrieves the cookies for the specified URL. Cookies are represented as
// strings of semi-colon delimited "name=value" pairs.
// Returns true if the function succeeds.
//
// FIREFOX NOTE: GetCookieString will function properly regardless of
// what thread of control it is called on. However, when it is not
// called on the main thread of control, this function proxies a
// method call to the main thread and blocks the calling thread until
// that proxied method call returns. It is best to arrange for this
// function to be called on the main thread in Firefox.
bool GetCookieString(const char16 *url, BrowsingContext *context,
                     std::string16 *cookies_out);

// Parses a cookie string, as returned by GetCookieString, populating
// map with an entry for each value. If a cookie in the string does not
// have a values associated with it, the map contains an entry with cookie
// as the key and an empty string as the value.
// Ex. Given "name=value;cookie" the map is populated as follows
//     map["name"] = "value";
//     map["cookie"] = "";
// Cookies are ordered from most specific to least specific,
// our map will only contain the most specific value.
void ParseCookieString(const std::string16 &cookies, CookieMap *map);

// Parses a "name=value" string into its the name and value parts.
// The split occurs at the first occurrence of the '=' character.
// Both the name and value are trimmed of whitespace. If there is no
// '=' delimiter, name will be populated with the entire input string,
// trimmed of whitespace, and value will be an empty string.
void ParseCookieNameAndValue(const std::string16 &name_and_value, 
                             std::string16 *name,
                             std::string16 *value);

// A collection of cookie name and optional value pairs
class CookieMap : public std::map<std::string16, std::string16> {
 public:
  // Retrieves the cookies for the specified URL and populates the
  // map with an entry for each value. Previous values in the map
  // are cleared prior to loading the new values. If the cookie string
  // cannot be retrieved, returns false and the map is not modified.
  bool LoadMapForUrl(const char16 *url, BrowsingContext *context);

  // Retrieves the value of the cookie named 'cookie_name'. If the cookie
  // is present but does not have a value, the value string will be empty.
  // Returns true if the cookie is present.
  bool GetCookie(const std::string16 &cookie_name,
                 std::string16 *cookie_value);

  // Returns true if the map contains a cookie named 'cookie_name'
  bool HasCookie(const std::string16 &cookie_name);

  // Returns true if the map contains a cookie with 'cookie_name' having the
  // value 'cookie_value'
  bool HasSpecificCookie(const std::string16 &cookie_name,
                         const std::string16 &cookie_value);

  // Returns true if the 'requiredCookie' attribute of a resource store
  // is satisfied by the contents of this CookieMap. The 'requiredCookie'
  // can express either a particular cookie name/value pair, or the absence
  // of cookie with a particular name.
  // "foo=bar" --> a cookie name "foo" must have the value "bar"
  // "foo" | "foo=" --> "foo" must be present but have an empty value
  // "foo=;NONE;" --> the collection must not contain a cookie for "foo"
  bool HasLocalServerRequiredCookie(const std::string16 &required_cookie);
};

#ifdef USING_CCTESTS
// For use by internal unit tests cases.
// Sets a fake cookie string for a particular url.
// When GetCookieString is queried about 'url', the fake 'cookies' value
// will be returned. If 'cookies' is NULL, an empty string will be returned.
// There can only be one fake cookie string set at a time. To clear the
// fake data call SetFakeCookieString(NULL, NULL)
void SetFakeCookieString(const char16* url, const char16 *cookies);
#endif

#endif  // GEARS_LOCALSERVER_COMMON_HTTP_COOKIES_H__
