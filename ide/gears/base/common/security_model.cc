// Copyright 2006, Google Inc.
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

#include <algorithm>
#include <cctype>
#include <string>
#include "gears/base/common/security_model.h"
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/http_constants.h"

#include "third_party/googleurl/src/url_parse.h"


const char16* kUnknownDomain      = STRING16(L"_null_.localdomain");
const char*   kUnknownDomainAscii =           "_null_.localdomain";


//------------------------------------------------------------------------------
// CopyFrom
//------------------------------------------------------------------------------
void SecurityOrigin::CopyFrom(const SecurityOrigin &security_origin) {
  initialized_ = security_origin.initialized_;
  url_ = security_origin.url_;
  full_url_ = security_origin.full_url_;
  scheme_ = security_origin.scheme_;
  host_ = security_origin.host_;
  port_ = security_origin.port_;
  port_string_ = security_origin.port_string_;
}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
bool SecurityOrigin::Init(const char16 *full_url, const char16 *scheme,
                          const char16 *host, int port) {
  assert(full_url && scheme && host); // file URLs pass 0 for 'port'
  if (!full_url[0] || !scheme[0] || !host[0])
    return false;

  full_url_ = full_url;
  scheme_ = scheme;
  host_ = host;
  port_ = port;

  port_string_ = IntegerToString16(port_);
  LowerString(scheme_);
  LowerString(host_);

  url_ = scheme_;
  url_ += STRING16(L"://");
  url_ += host;
  if (!IsDefaultPort(scheme_, port_)) {
    url_ += STRING16(L":");
    url_ += port_string_;
  }

  initialized_ = true;
  return true;
}


//------------------------------------------------------------------------------
// InitFromUrl
//------------------------------------------------------------------------------
bool SecurityOrigin::InitFromUrl(const char16 *full_url) {
  initialized_ = false;

  int url_len = char16_wcslen(full_url);

  url_parse::Component scheme_comp;
  if (!url_parse::ExtractScheme(full_url, url_len, &scheme_comp)) {
    return false;
  }

  std::string16 scheme(full_url + scheme_comp.begin, scheme_comp.len);
  LowerString(scheme);
  if (scheme == STRING16(L"http") || scheme == STRING16(L"https")) {
    url_parse::Parsed parsed;
    url_parse::ParseStandardURL(full_url, url_len, &parsed);

    // Disallow urls with embedded username:passwords. These are disabled by
    // default in IE such that InternetCrackUrl fails. To have consistent
    // behavior, do the same for all browsers.
    if (parsed.username.len != -1 || parsed.password.len != -1) {
      return false;
    }

    if (parsed.host.len == -1) {
      return false;
    }

    int port;
    if (parsed.port.len > 0) {
      std::string16 port_str(full_url + parsed.port.begin, parsed.port.len);
      port = ParseLeadingInteger(port_str.c_str(), NULL);
    } else if (scheme == HttpConstants::kHttpsScheme) {
      port = HttpConstants::kHttpsDefaultPort;
    } else {
      port = HttpConstants::kHttpDefaultPort;
    }

    std::string16 host(full_url + parsed.host.begin, parsed.host.len);
    return Init(full_url, scheme.c_str(), host.c_str(), port);
  } else if (scheme == STRING16(L"file")) {
    return Init(full_url, HttpConstants::kFileScheme, kUnknownDomain, 0);
  }

  return false;
}
