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
//
// TODO(cprince): move this shared file to /base

#ifndef GEARS_LOCALSERVER_COMMON_HTTP_CONSTANTS_H__
#define GEARS_LOCALSERVER_COMMON_HTTP_CONSTANTS_H__

#include "gears/base/common/string16.h"

class HttpConstants {
 public:
  enum {
    HTTP_OK = 200,
    HTTP_NOT_MODIFIED = 304,
    HTTP_MOVED = 301,
    HTTP_FOUND = 302,
    HTTP_INTERNAL_ERROR = 500,
    HTTP_SERVICE_UNAVAILABLE = 503
  };

  static const int kHttpStatusCodeMaxDigits = 3;
  static const int kMaxRedirects = 10;

  static const char16 *kCacheControlHeader;
  static const char16 *kContentDispositionHeader;
  static const char16 *kContentEncodingHeader;
  static const char16 *kContentLengthHeader;
  static const char16 *kContentTypeHeader;
  static const char16 *kCookieHeader;
  static const char16 *kCrLf;
  static const char   *kCrLfAscii;
  static const char16 *kExpiresHeader;
  static const char16 *kHttpScheme;
  static const char   *kHttpSchemeAscii;
  static const int     kHttpDefaultPort = 80;
  static const char16 *kHttpsScheme;
  static const char   *kHttpsSchemeAscii;
  static const int     kHttpsDefaultPort = 443;
  static const char16 *kFileScheme;
  static const char   *kFileSchemeAscii;
  static const int     kFileDefaultPort = 0;
  static const char16 *kHttpGET;
  static const char   *kHttpGETAscii;
  static const char16 *kHttpHEAD;
  static const char16 *kHttpPOST;
  static const char16 *kHttpPUT;
  static const char16 *kIfModifiedSinceHeader;
  static const char16 *kLastModifiedHeader;
  static const char16 *kLocationHeader;
  static const char16 *kMimeTextPlain;
  static const char16 *kMimeApplicationOctetStream;
  static const char16 *kNoCache;
  static const char16 *kOKStatusLine;
  static const char16 *kPragmaHeader;
  static const char16 *kRetryAfterHeader;
  static const char16 *kSetCookieHeader;
  static const char16 *kUriHeader;
  static const char16 *kUserAgentHeader;
  static const char16 *kXCapturedFilenameHeader;
  static const char16 *kXGoogleGearsBypassLocalServer;
  static const char16 *kXGearsSafariCapturedMimeType;
  static const char16 *kXGearsReasonHeader;
  static const char16 *kXGearsReason_ValidateManifest;
  static const char   *kXGearsDecodedContentLengthAscii;
  static const char16 *kXGoogleGearsHeader;
};

// Returns true if status_line appears to be a valid HTTP response status
// line and optionally returns the version, code, and reason substrings.
// The 'status_line' argument does not need to be terminated by a CRLF
// delimiter. The out arguments may be NULL.
// Ex. Given "HTTP/1.0 200 OK"
//     version_out = "HTTP/1.0", code_out = 200, reason_out = "OK"
bool ParseHttpStatusLine(const std::string16 &status_line,
                         std::string16 *version_out,
                         int *code_out,
                         std::string16 *reason_out);

// Returns true if 'code' is a valid HTTP response code
bool IsValidResponseCode(int code);

// Returns true if the port number is the default for the given scheme.
// The scheme should be either "http", "https", or "file", all lower case.
bool IsDefaultPort(const std::string16 &scheme, int port);

#endif  // GEARS_LOCALSERVER_COMMON_HTTP_CONSTANTS_H__
