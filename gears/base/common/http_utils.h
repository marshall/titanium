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
// Code lifted from google3 to parse HTTP headers.
// This file is branched from /google3/webutil/http/httputils.h
//
// Utilities that are useful for HTTP.  In particular, we
// can parse http headers.  This includes the "firstline"
// ("HTTP/1.0") in addition to the "Key: value" lines.


#ifndef GEARS_BASE_COMMON_HTTP_UTILS_H__
#define GEARS_BASE_COMMON_HTTP_UTILS_H__

#include <assert.h>
#include <string.h>
#include <iterator>
#include <string>
#include <vector>
#include "gears/base/common/basictypes.h"

class UnsafeArena;

// From google3/webutil/http/httpresponse
// These are the HTTP and RTSP status codes.
class HTTPResponse {
 public:
  // These are the status codes we can give back to the client
  // be sure to update here and in httpresponse.cc's response_codes[]
  // array definition, and in HTTPResponse::GetReasonPhrase
  enum ResponseCode {              // these are HTTP/1.0 responses, mostly
    RC_UNDEFINED                 = 0,    // illegal value for initialization
    RC_FIRST_CODE                = 100,

    // Informational
    RC_CONTINUE                  = 100,  // Continue
    RC_SWITCHING                 = 101,  // Switching Protocols
    RC_PROCESSING                = 102,  // Processing (rfc 2518, sec 10.1)

    // Success
    RC_REQUEST_OK                = 200,  // OK
    RC_CREATED                   = 201,  // Created
    RC_ACCEPTED                  = 202,  // Accepted
    RC_PROVISIONAL               = 203,  // Non-Authoritative Information
    RC_NO_CONTENT                = 204,  // No Content
    RC_RESET_CONTENT             = 205,  // Reset Content
    RC_PART_CONTENT              = 206,  // Partial Content
    RC_MULTI_STATUS              = 207,  // Multi-Status (rfc 2518, sec 10.2)
    RC_RTSP_LOW_STORAGE          = 250,  // Low On Storage Space (RTSP)

    // Redirect
    RC_MULTIPLE                  = 300,  // Multiple Choices
    RC_MOVED_PERM                = 301,  // Moved Permanently
    RC_MOVED_TEMP                = 302,  // Found
    RC_SEE_OTHER                 = 303,  // See Other
    RC_NOT_MODIFIED              = 304,  // Not Modified
    RC_USE_PROXY                 = 305,  // Use Proxy
    RC_TEMP_REDIRECT             = 307,  // More or less like 302

    // Client Error
    RC_BAD_REQUEST               = 400,  // Bad Request
    RC_UNAUTHORIZED              = 401,  // Unauthorized
    RC_PAYMENT                   = 402,  // Payment Required
    RC_FORBIDDEN                 = 403,  // Forbidden
    RC_NOT_FOUND                 = 404,  // Not Found
    RC_METHOD_NA                 = 405,  // Method Not Allowed
    RC_NONE_ACC                  = 406,  // Not Acceptable
    RC_PROXY                     = 407,  // Proxy Authentication Required
    RC_REQUEST_TO                = 408,  // Request Time-out
    RC_CONFLICT                  = 409,  // Conflict
    RC_GONE                      = 410,  // Gone
    RC_LEN_REQUIRED              = 411,  // Length Required
    RC_PRECOND_FAILED            = 412,  // Precondition Failed
    RC_ENTITY_TOO_BIG            = 413,  // Request Entity Too Large
    RC_URI_TOO_BIG               = 414,  // Request-URI Too Large
    RC_UNKNOWN_MEDIA             = 415,  // Unsupported Media Type
    RC_BAD_RANGE                 = 416,  // Requested range not satisfiable
    RC_BAD_EXPECTATION           = 417,  // Expectation Failed
    RC_UNPROC_ENTITY             = 422,  // Unprocessable Entity
                                         // (rfc 2518, sec 10.3)
    RC_LOCKED                    = 423,  // Locked (rfc 2518, sec 10.4)
    RC_FAILED_DEP                = 424,  // Failed Dependency
                                         // (rfc 2518, sec 10.5)
    RC_RTSP_INVALID_PARAM        = 451,  // Parameter Not Understood (RTSP)
    RC_RTSP_ILLEGAL_CONF         = 452,  // Unknown Conference (RTSP)
    RC_RTSP_INSUF_BANDWIDTH      = 453,  // Not Enough Bandwidth (RTSP)
    RC_RTSP_UNKNOWN_SESSION      = 454,  // Session Not Found (RTSP)
    RC_RTSP_BAD_METHOD           = 455,  // Method Not Valid In This State(RTSP)
    RC_RTSP_BAD_HEADER           = 456,  // Header Field Not Valid (RTSP)
    RC_RTSP_INVALID_RANGE        = 457,  // Invalid Range (RTSP)
    RC_RTSP_READONLY_PARAM       = 458,  // Parameter Is Read-Only (RTSP)
    RC_RTSP_BAD_AGGREGATE        = 459,  // Aggregate Opereration
                                         // Not Allowed (RTSP)
    RC_RTSP_AGGREGATE_ONLY       = 460,  // Only Aggregate Operation
                                         // Allowed (RTSP)
    RC_RTSP_BAD_TRANSPORT        = 461,  // Unsupported Transport (RTSP)
    RC_RTSP_BAD_DESTINATION      = 462,  // Destination Unreachable (RTSP)

    // Server Error
    RC_ERROR                     = 500,  // Internal Server Error
    RC_NOT_IMP                   = 501,  // Not Implemented
    RC_BAD_GATEWAY               = 502,  // Bad Gateway
    RC_SERVICE_UNAV              = 503,  // Service Unavailable
    RC_GATEWAY_TO                = 504,  // Gateway Time-out
    RC_BAD_VERSION               = 505,  // HTTP Version not supported
    RC_INSUF_STORAGE             = 507,  // Insufficient Storage
                                         // (rfc 2518, sec 10.6)
    RC_RTSP_BAD_OPTION           = 551,  // Option Not Supported (RTSP)

    RC_LAST_CODE                 = 599,
  };  // [sic]
};


// Only a subset of this class has been brought over
class HTTPHeaders {
 public:
  // Convenience typedefs
  typedef std::vector< std::pair<char*,char*> > KeyValueList;
  typedef KeyValueList::iterator iterator;
  typedef KeyValueList::const_iterator const_iterator;
  typedef KeyValueList::reverse_iterator const_reverse_iterator;

 public:
  // HTTP Header Names
  static const char * const ACCEPT;
  static const char * const ACCEPT_CHARSET;
  static const char * const ACCEPT_ENCODING;
  static const char * const ACCEPT_LANGUAGE;
  static const char * const ACCEPT_RANGES;
  static const char * const AGE;
  static const char * const AUTHORIZATION;
  static const char * const CACHE_CONTROL;
  static const char * const CONNECTION;
  static const char * const CONTENT_DISPOSITION;
  static const char * const CONTENT_ENCODING;
  static const char * const CONTENT_LANGUAGE;
  static const char * const CONTENT_LENGTH;
  static const char * const CONTENT_LOCATION;
  static const char * const CONTENT_RANGE;
  static const char * const CONTENT_TYPE;
  static const char * const COOKIE;
  static const char * const DATE;
  static const char * const DAV;
  static const char * const DEPTH;
  static const char * const DESTINATION;
  static const char * const ETAG;
  static const char * const EXPECT;
  static const char * const EXPIRES;
  static const char * const FROM;
  static const char * const HOST;
  static const char * const IF;
  static const char * const IF_MATCH;
  static const char * const IF_MODIFIED_SINCE;
  static const char * const IF_NONE_MATCH;
  static const char * const IF_RANGE;
  static const char * const IF_UNMODIFIED_SINCE;
  static const char * const KEEP_ALIVE;
  static const char * const LABEL;
  static const char * const LAST_MODIFIED;
  static const char * const LOCATION;
  static const char * const LOCK_TOKEN;
  static const char * const MS_AUTHOR_VIA;
  static const char * const OVERWRITE_HDR;
  static const char * const P3P;
  static const char * const PRAGMA;
  static const char * const PROXY_CONNECTION;
  static const char * const PROXY_AUTHENTICATE;
  static const char * const PROXY_AUTHORIZATION;
  static const char * const RANGE;
  static const char * const REFERER;
  static const char * const SERVER;
  static const char * const SET_COOKIE;
  static const char * const STATUS_URI;
  static const char * const TIMEOUT;
  static const char * const TRAILERS;
  static const char * const TRANSFER_ENCODING;
  static const char * const TRANSFER_ENCODING_ABBRV;
  static const char * const UPGRADE;
  static const char * const USER_AGENT;
  static const char * const VARY;
  static const char * const VIA;
  static const char * const WWW_AUTHENTICATE;
  static const char * const X_FORWARDED_FOR;
  // For users behind a proxy, this indicates to the GFEs the IP of
  // the end client the request originated from.  The GFEs will only
  // consider this the originating IP for trusted proxies.
  static const char * const X_PROXYUSER_IP;
  // X_UP_SUBNO and XID are used in mobile for device id's
  static const char * const X_UP_SUBNO;
  static const char * const XID;
  // Our extension to robots to allow non-html content providers (eg. pdf docs)
  // to specify robots tags similar to the robots meta tags (noindex, nofollow
  // etc.) via the http headers.
  static const char * const X_ROBOTS;

 public:
  // Constructor/Destructor
  HTTPHeaders();
  virtual ~HTTPHeaders();
  void ClearHeaders();

  bool IsEmpty() const;  // Are there any key-value pairs at all?

  // Allow efficient iteration over the key-value pairs; ignores firstline
  const_iterator begin() const;
  const_iterator end() const;

  // Allow reverse iteration over the key-value pairs; ignores firstline
  const_reverse_iterator rbegin() const;
  const_reverse_iterator rend() const;

  // --- ACCESSORS and MUTATORS

  // Note all key/value lookups below are case-insensitive, since HTTP/1.1
  // spec says headers are case-insensitive (with rare and obscure exceptions)
  enum { UNUSED, NO_OVERWRITE, APPEND, OVERWRITE };
  const char* GetHeader(const char* key) const;  // returns NULL if not found
  void GetHeaders(const char* header,
                  std::vector<const char*>* values) const;
  const char* GetHeaderOr(const char* key, const char* deflt) const {
    // returns default if not found
    if (const char* found = GetHeader(key)) return found; else return deflt;
  }
  bool HeaderIs(const char* key, const char* value) const;  // is key==value?
  bool HeaderStartsWith(const char* key, const char* value) const;
  const char* SetHeader(const char* key, const char* value,
                        int overwrite);
  const char* AddNewHeader(const char* key, const char* value);
  bool ClearHeader(const char* key);     // obliterates key/val pair from hdrs

  // This RETURNs "" if you didn't call ReadMoreInfo() or AddFirstLine()
  const char* firstline() const              { return firstline_.c_str(); }

  // We assign these constants numberic values that are mnemonic
  // HTTP_OTHER is meant for Google's internal, short, HTTP protocol, in
  // which we accept responses that look like "H 200 O".
  // HTTP_ICY is for icecast/shoutcast (http://www.icecast.org/). A typical
  // response is "ICY 200 OK" followed by a bunch of icy-specific response
  // headers.
  /* Gears deletions
  enum HTTPVersion { HTTP_ERROR = 0, HTTP_OTHER = 1, HTTP_ICY = 8,
                     HTTP_09 = 9, HTTP_10 = 10, HTTP_11 = 11 };
  */
  enum HTTPVersion { HTTP_ERROR = 0, HTTP_10 = 10, HTTP_11 = 11 };
  HTTPVersion http_version() const           { return http_version_; }
  const char* http_version_str() const {
    switch (http_version_) {
      case HTTP_ERROR: return "";
      /* Gears deletions
      case HTTP_OTHER: return "H";       // compact response from google servers
      case HTTP_ICY: return "ICY";       // For icecast/shoutcast.
      case HTTP_09: return "";           // HTTP/0.9 predates version naming
      */
      case HTTP_10: return "HTTP/1.0";
      case HTTP_11: return "HTTP/1.1";
      default: assert(NULL == "Unknown HTTPVersion in http_version_str()");
               return "";
    }
  }
  void set_http_version(HTTPVersion version) { http_version_ = version; }

  // These vars are useful only for HTTP responses (what servers give back)
  HTTPResponse::ResponseCode response_code() const  { return response_code_; }
  void set_response_code(HTTPResponse::ResponseCode r) { response_code_ = r; }
  // A "reason phrase" is, eg, "OK" or "Moved Permanently"
  const char* reason_phrase() const         { return reason_phrase_.c_str(); }
  void set_reason_phrase(const char* rp)     { reason_phrase_.assign(rp); }

  bool IsUsingTooMuchMemory() const { return false; }  // Gears change

  // --- CONVENIENCE MUTATORS -- let you set from a line or lines of text

  // These RETURN true if we parse OK, false if we see an error
  bool AddResponseFirstline(const char* firstline, int linelen); // "OK"

  // Sets the value of a header based on a response-line from a server,
  // eg "Header: Value".  Do not include the trailing \r\n.
  const char* SetHeaderFromLine(char* line, int overwrite);

 protected:
  // Checks the protocol and version in a response. Called by
  // AddResponseFirstline when the first line is parsed.
  // Can be overridden to support other protocols (such as Gnutella) that
  // use HTTP-like headers.
  // Returns true if the protocol was recognized, false otherwise.
  // Run "g4 describe -c 324620" for the long story on why this particular
  // design choice was made.
  // TODO: Revisit this design choice...
  virtual bool CheckResponseProtocol(const char* proto, int protolen);

  // Append the given value to the given header, with the given separator.
  const char* AppendValueToHeader(std::pair<char*,char*>* header,
                                  const char* separator,
                                  const char* value);
 private:
  std::string firstline_;
  HTTPVersion http_version_;
  HTTPResponse::ResponseCode response_code_;
  std::string reason_phrase_;
  KeyValueList * headers_;
  UnsafeArena* arena_;          // For allocating header keys/values
  DISALLOW_EVIL_CONSTRUCTORS(HTTPHeaders);
};

// This class doesn't hold any data; it's used merely as a namespace.
class HTTPUtils {
 public:
  // Pass in a pointer to the full http document text, including
  // headers, and a pointer to the full document length.  Modifies
  // these values: body points to the body only (beyond headers),
  // and bodylen is set accordingly.  If extraction fails, (e.g.
  // empty document) sets body = NULL and bodylen = 0.
  static void ExtractBody(const char **body, uint32* bodylen);

  // This contains code that was refactored from ExtractBody and
  // HTTPParser::Parse.  body and bodylen are modified exactly like they are
  // in ExtractBody.  If headers is not NULL, the headers are parsed, and
  // if allow_const_cast is true, the body is modified temporarily when
  // parsing the headers.  Otherwise, each header line is copied into a
  // temporary string.
  //
  // Returns true only when a blank line ('\r\n' only) has been seen.
  // Returns false if an error occurs before then.
  static bool ParseHTTPHeaders(const char** body, uint32* bodylen,
                               HTTPHeaders* headers, bool allow_const_cast);
};


#endif /* GEARS_BASE_COMMON_HTTP_UTILS_H__ */
