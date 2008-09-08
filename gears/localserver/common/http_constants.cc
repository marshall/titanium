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

#include "gears/base/common/http_utils.h"
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/http_constants.h"

const char16 *HttpConstants::kCacheControlHeader = STRING16(L"Cache-Control");
const char16 *HttpConstants::kContentDispositionHeader
                                            = STRING16(L"Content-Disposition");
const char16 *HttpConstants::kContentEncodingHeader
                                            = STRING16(L"Content-Encoding");
const char16 *HttpConstants::kContentLengthHeader = STRING16(L"Content-Length");
const char16 *HttpConstants::kContentTypeHeader = STRING16(L"Content-Type");
const char16 *HttpConstants::kCookieHeader = STRING16(L"Cookie");
const char16 *HttpConstants::kCrLf = STRING16(L"\r\n");
const char   *HttpConstants::kCrLfAscii = "\r\n";
const char16 *HttpConstants::kExpiresHeader = STRING16(L"Expires");
const char16 *HttpConstants::kFileScheme = STRING16(L"file");
const char   *HttpConstants::kFileSchemeAscii =      "file";
const char16 *HttpConstants::kHttpGET = STRING16(L"GET");
const char   *HttpConstants::kHttpGETAscii = "GET";
const char16 *HttpConstants::kHttpHEAD = STRING16(L"HEAD");
const char16 *HttpConstants::kHttpPOST = STRING16(L"POST");
const char16 *HttpConstants::kHttpPUT = STRING16(L"PUT");
const char16 *HttpConstants::kHttpScheme = STRING16(L"http");
const char   *HttpConstants::kHttpSchemeAscii =      "http";
const char16 *HttpConstants::kHttpsScheme = STRING16(L"https");
const char   *HttpConstants::kHttpsSchemeAscii =      "https";
const char16 *HttpConstants::kIfModifiedSinceHeader =
                                 STRING16(L"If-Modified-Since");
const char16 *HttpConstants::kLastModifiedHeader = STRING16(L"Last-Modified");
const char16 *HttpConstants::kLocationHeader = STRING16(L"Location");
const char16 *HttpConstants::kMimeTextPlain = STRING16(L"text/plain");
const char16 *HttpConstants::kMimeApplicationOctetStream =
                                 STRING16(L"application/octet-stream");
const char16 *HttpConstants::kNoCache = STRING16(L"no-cache");
const char16 *HttpConstants::kOKStatusLine = STRING16(L"HTTP/1.1 200 OK");
const char16 *HttpConstants::kPragmaHeader = STRING16(L"Pragma");
const char16 *HttpConstants::kRetryAfterHeader = STRING16(L"Retry-After");
const char16 *HttpConstants::kSetCookieHeader = STRING16(L"Set-Cookie");
const char16 *HttpConstants::kUriHeader = STRING16(L"URI");
const char16 *HttpConstants::kUserAgentHeader = STRING16(L"User-Agent");
const char16 *HttpConstants::kXCapturedFilenameHeader =
                                 STRING16(L"X-Captured-Filename");
// These header values should not be prefixed with "X-Google" as those values
// are special cased and stripped by GFEs.
// [naming] Note the "X-Gears-" prefixes here.
const char16 *HttpConstants::kXGoogleGearsBypassLocalServer =
                                 STRING16(L"X-Gears-Bypass-LocalServer");
const char16 *HttpConstants::kXGearsSafariCapturedMimeType =
                                 STRING16(L"X-Gears-Safari-MimeType");
const char16 *HttpConstants::kXGearsReasonHeader =
                                 STRING16(L"X-Gears-Reason");
const char16 *HttpConstants::kXGearsReason_ValidateManifest =
                                 STRING16(L"validate-manifest");
const char   *HttpConstants::kXGearsDecodedContentLengthAscii =
                                 "X-Gears-Decoded-Content-Length";
const char16 *HttpConstants::kXGoogleGearsHeader =
                                 STRING16(L"X-Gears-Google");

bool ParseHttpStatusLine(const std::string16 &status_line,
                         std::string16 *version_out,
                         int *code_out,
                         std::string16 *reason_out) {
  /* From RFC2616

  6.1 Status-Line

  The first line of a Response message is the Status-Line, consisting
  of the protocol version followed by a numeric status code and its
  associated textual phrase, with each element separated by SP
  characters. No CR or LF is allowed except in the final CRLF sequence.

      Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
      HTTP-Version = "HTTP" "/" 1*DIGIT "." 1*DIGIT
  
  Ex. "HTTP/1.0 200 OK"
  */

  // We have to compose a message body in order to use HTTPUtils,
  // in particular, the 'body' must contain a blank line which
  // delimits the response headers from the response data.
  std::string status_line_ascii;
  String16ToUTF8(status_line.c_str(), status_line.length(), &status_line_ascii);
  status_line_ascii += HttpConstants::kCrLfAscii;
  status_line_ascii += HttpConstants::kCrLfAscii;
  // Also workaround a bug in ParseHTTPHeaders when the reason phrase
  // contains a ':'
  const std::string kColon(":");
  const std::string kDash("-");
  ReplaceAll(status_line_ascii, kColon, kDash);
  const char *body = status_line_ascii.c_str();
  uint32 body_len = status_line_ascii.length();
  HTTPHeaders headers;
  if (!HTTPUtils::ParseHTTPHeaders(&body, &body_len,
                                   &headers, true /* allow_const_cast */)) {
    return false;
  }

  // ParseHTTPHeaders returns true even if there is no status line,
  // here we ensure that a valid status line was seen and parsed.
  if (headers.http_version() == HTTPHeaders::HTTP_ERROR ||
      headers.response_code() == HTTPResponse::RC_UNDEFINED) {
    return false;
  }

  if (version_out) {
    UTF8ToString16(headers.http_version_str(), version_out);
  }
  if (code_out) {
    *code_out = static_cast<int>(headers.response_code());
  }
  if (reason_out) {
    UTF8ToString16(headers.reason_phrase(), reason_out);
  }

  return true;
}


bool IsValidResponseCode(int code) {
  return code >= HTTPResponse::RC_FIRST_CODE &&
         code <= HTTPResponse::RC_LAST_CODE;
}

bool IsDefaultPort(const std::string16 &scheme, int port) {
  assert((scheme == HttpConstants::kHttpScheme) ||
         (scheme == HttpConstants::kHttpsScheme) ||
         (scheme == HttpConstants::kFileScheme));

  return ((scheme == HttpConstants::kHttpScheme) &&
          (port == HttpConstants::kHttpDefaultPort)) ||
         ((scheme == HttpConstants::kHttpsScheme) &&
          (port == HttpConstants::kHttpsDefaultPort)) ||
         ((scheme == HttpConstants::kFileScheme) &&
          (port == HttpConstants::kFileDefaultPort));
}
