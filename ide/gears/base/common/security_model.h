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

#ifndef GEARS_BASE_COMMON_SECURITY_MODEL_H__
#define GEARS_BASE_COMMON_SECURITY_MODEL_H__

#include <assert.h>
#include "gears/base/common/string16.h"

// Value to use when the source domain cannot be determined.
extern const char16* kUnknownDomain;
extern const char*   kUnknownDomainAscii;

//------------------------------------------------------------------------------
// Class that represents the origin of a URL. The origin includes the scheme,
// host, and port.
//------------------------------------------------------------------------------
class SecurityOrigin {
 public:
  SecurityOrigin() : initialized_(false), port_(0) {}

  // Makes a copy of the given security origin.
  void CopyFrom(const SecurityOrigin &security_origin);

  // Extacts the scheme, host, and port of the 'full_url'.
  // Returns true if successful. Only "file","http", and "https"
  // urls are supported. In the case of a file url, the host will
  // be set to kUnknownDomain.
  bool InitFromUrl(const char16 *full_url);

  // Has the security origin been initialized yet?
  bool initialized() const
      { return initialized_; }

  // A url that contains the information representative of the security
  // origin and nothing more. The path is always empty. The port number is
  // not included for for the default port case.
  // Ex. http://host:99, http://host
  const std::string16 &url() const
      { assert(initialized_); return url_; }

  // The full url the origin was initialized with.
  // TODO(michaeln): Remove this convenience. Callers that need it should
  // pass it around rather than having it piggyback on this class.
  const std::string16 &full_url() const
      { assert(initialized_); return full_url_; }

  const std::string16 &scheme() const
      { assert(initialized_); return scheme_; }

  const std::string16 &host() const
      { assert(initialized_); return host_; }

  int port() const
      { assert(initialized_); return port_; }

  const std::string16 &port_string() const
      { assert(initialized_); return port_string_; }

  // Returns true if 'other' and 'this' represent the same origin. If
  // either origin is not initalized, returns false.
  bool IsSameOrigin(const SecurityOrigin &other) const {
    assert(initialized_);
    assert(other.initialized_);
    return (port_ == other.port_) &&
           (scheme_ == other.scheme_) &&
           (host_ == other.host_);
  }

  // Returns true if 'full_url' is form the same origin as 'this'
  bool IsSameOriginAsUrl(const char16 *full_url) const {
    SecurityOrigin origin;
    return origin.InitFromUrl(full_url) &&
           IsSameOrigin(origin);
  }

 private:
  friend bool TestSecurityModel(std::string16 *error);

  bool Init(const char16 *full_url, const char16 *scheme,
            const char16 *host, int port);

  bool initialized_;
  std::string16 url_;
  std::string16 full_url_;
  std::string16 scheme_;
  std::string16 host_;
  int port_;
  std::string16 port_string_;
};

#endif // GEARS_BASE_COMMON_SECURITY_MODEL_H__
