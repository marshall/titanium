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
// This file is branched from /google3/webutil/http/httputils.cc

#include <iterator>
#include <vector>
#include "gears/base/common/http_utils.h"

// ----------------------------------------------------------------------
// The top portion of this file contains various google3 utils that the
// header parsing code we're actually interested in re-using depends on.
// ----------------------------------------------------------------------

#if WIN32
// From google3/base/port.h
#pragma warning(disable : 4018)  // signed/unsigned mismatch
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

// From google3/string/memutils.h
// ----------------------------------------------------------------------
// strliterallen, memcasecmp, memcaseis, memis, memcasestr
// ----------------------------------------------------------------------

// The ""'s catch people who don't pass in a literal for "str"
#define strliterallen(str) (sizeof("" str "")-1)

static int memcasecmp(const char *s1, const char *s2, size_t len) {
  const unsigned char *us1 = reinterpret_cast<const unsigned char *>(s1);
  const unsigned char *us2 = reinterpret_cast<const unsigned char *>(s2);

  for ( size_t i = 0; i < len; i++ ) {
    const int diff = tolower(us1[i]) - tolower(us2[i]);
    if (diff != 0) return diff;
  }
  return 0;
}

#define memcaseis(str, len, literal)                            \
   ( (((len) == strliterallen(literal))                         \
      && memcasecmp(str, literal, strliterallen(literal)) == 0) )

#define memis(str, len, literal)                                \
   ( (((len) == strliterallen(literal))                         \
      && memcmp(str, literal, strliterallen(literal)) == 0) )

// From google3/string/strutil.h
// ----------------------------------------------------------------------
// var_strprefix()
// var_strcaseprefix()
//    Give me a string and a putative prefix, and I return a pointer
//    past the prefix if the prefix matches, or NULL else.
//    Just like 'strprefix' and 'strcaseprefix' except that it works
//    on character pointers as well as constant strings.
//    Templates are used to provide both const and non-const versions.
// ----------------------------------------------------------------------

template<class CharStar>
inline CharStar var_strprefix(CharStar str, const char* prefix) {
  const int len = strlen(prefix);
  return strncmp(str, prefix, len) == 0 ?  str + len : NULL;
}

template<class CharStar>
inline CharStar var_strcaseprefix(CharStar str, const char* prefix) {
  const int len = strlen(prefix);
  return strncasecmp(str, prefix, len) == 0 ?  str + len : NULL;
}

// ----------------------------------------------------------------------
// Replacements for logging related macros
// ----------------------------------------------------------------------

class null_stream {
 public:
  null_stream& operator <<(char*) { return *this; }
  null_stream& operator <<(const char*) { return *this; }
};
static null_stream devnull;

#define CHECK_LT(x, y) assert(x < y)
#define CHECK_GE(x, y) assert(x >= y)
#define CHECK(x) assert(x);
#define NOLOG(level) devnull   // was LOG in original sources
#define NOVLOG(level) devnull  // was VLOG in original sources
#define WARNING 1

// Based on google3/base/arena.h
// ----------------------------------------------------------------------
// We implement just enough of the interface to allow this module to compile.
// ----------------------------------------------------------------------

class UnsafeArena {
 public:
  ~UnsafeArena() {
    Reset();
  }
  char* Strdup(const char* s) {
    size_t size = strlen(s) + 1;
    return strncpy(Alloc(size), s, size);
  }
  char* Alloc(const size_t size) {
    char *alloced = new char[size];
    allocations_.push_back(alloced);
    return alloced;
  }
  void Reset() {
    for (size_t i = 0; i < allocations_.size(); ++i) {
      delete [] allocations_[i];
    }
    allocations_.clear();
  }
 private:
  std::vector<char*> allocations_;
};

// The google3 code we've picked up uses the standard namespace
using std::vector;
using std::pair;
using std::string;
using std::make_pair;

// ----------------------------------------------------------------------
// The remainder of this file is largely unmodified from the original
// sources in google3/webutil/http/httputils.cc, with the caveat that
// we've only taken a subset of the original methods. The methods that
// we have taken are nearly verbatim.
// ----------------------------------------------------------------------

const char * const HTTPHeaders::ACCEPT = "Accept";
const char * const HTTPHeaders::ACCEPT_CHARSET = "Accept-Charset";
const char * const HTTPHeaders::ACCEPT_ENCODING = "Accept-Encoding";
const char * const HTTPHeaders::ACCEPT_LANGUAGE = "Accept-Language";
const char * const HTTPHeaders::ACCEPT_RANGES = "Accept-Ranges";
const char * const HTTPHeaders::AGE = "Age";
const char * const HTTPHeaders::AUTHORIZATION = "Authorization";
const char * const HTTPHeaders::CACHE_CONTROL = "Cache-Control";
const char * const HTTPHeaders::CONNECTION = "Connection";
const char * const HTTPHeaders::CONTENT_DISPOSITION = "Content-Disposition";
const char * const HTTPHeaders::CONTENT_ENCODING = "Content-Encoding";
const char * const HTTPHeaders::CONTENT_LANGUAGE = "Content-Language";
const char * const HTTPHeaders::CONTENT_LENGTH = "Content-Length";
const char * const HTTPHeaders::CONTENT_LOCATION = "Content-Location";
const char * const HTTPHeaders::CONTENT_RANGE = "Content-Range";
const char * const HTTPHeaders::CONTENT_TYPE = "Content-Type";
const char * const HTTPHeaders::COOKIE = "Cookie";
const char * const HTTPHeaders::DATE = "Date";
const char * const HTTPHeaders::DAV = "DAV";
const char * const HTTPHeaders::DEPTH = "Depth";
const char * const HTTPHeaders::DESTINATION = "Destination";
const char * const HTTPHeaders::ETAG = "ETag";
const char * const HTTPHeaders::EXPECT  = "Expect";
const char * const HTTPHeaders::EXPIRES = "Expires";
const char * const HTTPHeaders::FROM = "From";
const char * const HTTPHeaders::HOST = "Host";
const char * const HTTPHeaders::IF = "If";
const char * const HTTPHeaders::IF_MATCH = "If-Match";
const char * const HTTPHeaders::IF_MODIFIED_SINCE = "If-Modified-Since";
const char * const HTTPHeaders::IF_NONE_MATCH = "If-None-Match";
const char * const HTTPHeaders::IF_UNMODIFIED_SINCE = "If-Unmodified-Since";
const char * const HTTPHeaders::IF_RANGE = "If-Range";
const char * const HTTPHeaders::KEEP_ALIVE = "Keep-Alive";
const char * const HTTPHeaders::LABEL = "Label";
const char * const HTTPHeaders::LAST_MODIFIED = "Last-Modified";
const char * const HTTPHeaders::LOCATION = "Location";
const char * const HTTPHeaders::LOCK_TOKEN = "Lock-Token";
const char * const HTTPHeaders::MS_AUTHOR_VIA = "MS-Author-Via";
const char * const HTTPHeaders::OVERWRITE_HDR = "Overwrite";
const char * const HTTPHeaders::PRAGMA = "Pragma";
const char * const HTTPHeaders::P3P = "P3P";
const char * const HTTPHeaders::PROXY_CONNECTION = "Proxy-Connection";
const char * const HTTPHeaders::PROXY_AUTHENTICATE = "Proxy-Authenticate";
const char * const HTTPHeaders::PROXY_AUTHORIZATION = "Proxy-Authorization";
const char * const HTTPHeaders::RANGE = "Range";
const char * const HTTPHeaders::REFERER = "Referer";
const char * const HTTPHeaders::SERVER = "Server";
const char * const HTTPHeaders::SET_COOKIE = "Set-Cookie";
const char * const HTTPHeaders::STATUS_URI = "Status-URI";
const char * const HTTPHeaders::TIMEOUT = "Timeout";
const char * const HTTPHeaders::TRAILERS = "Trailers";
const char * const HTTPHeaders::TRANSFER_ENCODING = "Transfer-Encoding";
const char * const HTTPHeaders::TRANSFER_ENCODING_ABBRV = "TE";
const char * const HTTPHeaders::UPGRADE = "Upgrade";
const char * const HTTPHeaders::USER_AGENT = "User-Agent";
const char * const HTTPHeaders::VARY = "Vary";
const char * const HTTPHeaders::VIA = "Via";
const char * const HTTPHeaders::WWW_AUTHENTICATE = "WWW-Authenticate";
const char * const HTTPHeaders::X_FORWARDED_FOR = "X-Forwarded-For";
const char * const HTTPHeaders::X_PROXYUSER_IP = "X-ProxyUser-IP";
const char * const HTTPHeaders::X_UP_SUBNO = "X-Up-Subno";
const char * const HTTPHeaders::XID = "XID";
const char * const HTTPHeaders::X_ROBOTS = "X-Robots-Tag";

// ----------------------------------------------------------------------
// HTTPHeaders::HTTPHeaders()
// ~HTTPHeaders::HTTPHeaders()
//    Basic memory management
// ----------------------------------------------------------------------

HTTPHeaders::HTTPHeaders()
  : headers_(new KeyValueList), arena_(new UnsafeArena) {
  // Could just call ClearHeaders(), but this is faster and has the same effect
  set_http_version(HTTP_ERROR);
  set_response_code(HTTPResponse::RC_UNDEFINED);
}

HTTPHeaders::~HTTPHeaders() {
  delete headers_;
  delete arena_;
}

// ----------------------------------------------------------------------
// HTTPHeaders::ClearHeaders()
//    SetHeader() allocates both the key and the value using
//    arena_->Strdup.  We reset the arena and call clear() on the
//    underlying vector.  You're left with an empty headers vector.
// ----------------------------------------------------------------------

void HTTPHeaders::ClearHeaders() {
  set_http_version(HTTP_ERROR);
  set_response_code(HTTPResponse::RC_UNDEFINED);
  set_reason_phrase("");
  firstline_.clear();
  headers_->clear();
  arena_->Reset();
}

// ----------------------------------------------------------------------
// HTTPHeaders::IsEmpty()
// HTTPHeaders::begin()
// HTTPHeaders::end()
//   We'd put these in the .h file if we could use STL there.
// ----------------------------------------------------------------------

bool HTTPHeaders::IsEmpty() const {
  return headers_->empty();
}
HTTPHeaders::const_iterator HTTPHeaders::begin() const {
  return headers_->begin();
}
HTTPHeaders::const_iterator HTTPHeaders::end() const   {
  return headers_->end();
}
HTTPHeaders::const_reverse_iterator HTTPHeaders::rbegin() const {
  return headers_->rbegin();
}
HTTPHeaders::const_reverse_iterator HTTPHeaders::rend() const   {
  return headers_->rend();
}

// ----------------------------------------------------------------------
// HTTPHeaders::GetHeader()
//    Returns the value associated with header.
//    Returns NULL if one is not found.
//       This function should not be used to process large numbers of
//    command headers (it searches a vector) but it's very handy when
//    there are just a few headers.
//       Also do not use this method to get the special case header
//    'Set-Cookie', because there maybe multiple Set-Cookie headers and this
//    method will only return the first one.  Use GetHeaders() instead.
// ----------------------------------------------------------------------

const char* HTTPHeaders::GetHeader(const char* header) const {
  const_iterator key;
  for (key = headers_->begin(); key != headers_->end(); ++key) {
    if (strcasecmp(key->first, header) == 0) {
      NOVLOG(5) << "Found HTTP header: '" << header << "': '"
	      << (key->second ? key->second : "(null)") << "'";
      return key->second;         // The associated value
    }
  }
  NOVLOG(5) << "Failed to find HTTP header: '" << header << "'";
  return NULL;
}

// ----------------------------------------------------------------------
// HTTPHeaders::GetHeaders()
//    Returns all values associated with a header.
//    Use this method for the 'Set-Cookie' header since there may
//    be multiple instances.
// ----------------------------------------------------------------------
void HTTPHeaders::GetHeaders(const char* header,
                             vector<const char*>* values) const {
  const_iterator key;
  for (key = headers_->begin(); key != headers_->end(); ++key) {
    if (strcasecmp(key->first, header) == 0) {
      NOVLOG(5) << "Found HTTP header: '" << header << "': '"
	      << (key->second ? key->second : "(null)") << "'";
      values->push_back(key->second);
    }
  }
}

// ----------------------------------------------------------------------
// HTTPHeaders::HeaderIs()
// HTTPHeaders::HeaderStartsWith()
//    Convenience methods which tell you if a header is present, and
//    if so whether it has a certain value, or starts with a certain
//    value.  All comparisons are case-insensitive.
// ----------------------------------------------------------------------

bool HTTPHeaders::HeaderIs(const char* key, const char* value) const {
  const char* real_value = GetHeader(key);
  return real_value && !strcasecmp(real_value, value);
}

bool HTTPHeaders::HeaderStartsWith(const char* key, const char* value) const {
  const char* real_value = GetHeader(key);
  return real_value && var_strcaseprefix(real_value, value);
}

// ----------------------------------------------------------------------
// HTTPHeaders::SetHeader()
// HTTPHeaders::AddNewHeader()
//    Sets a header in the vector of headers.  If a header with the
//    given name already exists and the overwrite parameter is set to
//    OVERWRITE, then we overwrite it.  If a header with the given name
//    already exists and the overwrite parameter is set to APPEND, then
//    we append the new value on to the old value (separating them with
//    a ',').  When setting the new vector entry, it makes copies
//    of both key and value.
//       In the case of AddNewHeader(), we always tack on the given
//    key and value to the end of our list of headers. You probably only
//    want to use this low-level interface if you're writing test code
//    that needs to generate duplicate HTTP headers. Well-behaved
//    programs should use SetHeader().
//       Value can be NULL; this means "erase this header."  We actually
//    store with the NULL, so the erase can apply "automatic"
//    headers the connection class would otherwise emit by itself.
//       RETURNS a pointer to the new value for the key we just inserted
//    (which, in the overwrite == APPEND case, may not be the value that
//    we just set), where it lives in the array.
//
//    If we are over the threshold for header size, we return null. We
//    don't even bother to allow header erasing in this case.
// ----------------------------------------------------------------------

const char* HTTPHeaders::SetHeader(const char* key, const char* value,
                                   int overwrite) {

  // We want to limit the size the headers can grow to, to avoid
  // denial-of-service where clients keep sending us lots of data.
  // Ignore any beyond the max.
  if ( IsUsingTooMuchMemory() )
    return NULL;

  // Header values cannot contain \r\n because everything following the \r\n
  // will be interpreted as a new header line. This can cause security problems
  // if an uncareful server accepts user input (e.g., a URL to redirect to) and
  // writes it into response headers without proper sanitization. So scan for
  // CR or LF, and if we find either, replace it with "_". Common case is
  // none is found, so all we've done is scan the string.
  if (value) {
    char* copy = NULL;
    for (int i=0; value[i]; i++)
      if (value[i] == '\r' || value[i] == '\n') {
        if (!copy) {                                  // only dup once
          copy = arena_->Strdup(value);
        }
        copy[i] = '_';
      }

    if (copy)
      value = copy;
  }

  // Special case Set-Cookie/WWW-Authenticate can be a multi-line header
  // (like a multi-map)
  if ( !strcasecmp(key, SET_COOKIE) || !strcasecmp(key, WWW_AUTHENTICATE) ) {
    if ( overwrite == APPEND ) {
      return AddNewHeader( key, value);
    } else if ( overwrite == OVERWRITE ) {
      ClearHeader(key);
      return AddNewHeader(key, value);
    }
    // If overwrite is NO_OVERWRITE, then the non-Set-Cookie-specific code
    // below will handle it correctly.
  }

  for ( KeyValueList::iterator it = headers_->begin(); it != headers_->end();
        ++it ) {
    if ( !strcasecmp(it->first, key) ) {
      if ( overwrite == NO_OVERWRITE ) {
        NOVLOG(5) << "Didn't replace HTTP header '" << it->first << "':'"
                << it->second << "' with value '" << value << "' "
                << "because the overwrite flag was not set";
      } else if ( overwrite == OVERWRITE || it->second == NULL ) {
        // We temporarily leak the memory for the old key and value.
        // Note: we could avoid copying key here but I am worried about
        // case differences between old key and new key.
        it->first = arena_->Strdup(key);
        NOVLOG(5) << "Replaced HTTP header '"
                << it->first << "':'" << it->second
                << "' with value '" << value << "'";
        it->second = value ? arena_->Strdup(value) : NULL;
      } else {
        assert(overwrite == APPEND);
        if ( value ) {
          // We temporarily leak the memory for the old key and value.
          // Note: we could avoid copying key here but I am worried about
          // case differences between old key and new key.
          it->first = arena_->Strdup(key);
          AppendValueToHeader(&(*it), ",", value);
        } else {
          NOVLOG(5) << "Appended nothing to HTTP header '"
                  << it->first << "':'" << it->second << "'";
        }
      }
      return it->second;
    }
  }
  // Nothing matched, so we can insert
  return AddNewHeader(key, value);
}

const char* HTTPHeaders::AddNewHeader(const char* key, const char* value) {

  // We want to limit the size the headers can grow to, to avoid
  // denial-of-service where clients keep sending us lots of data.
  // Ignore any beyond the max.
  if ( IsUsingTooMuchMemory() )
    return NULL;

  headers_->push_back(make_pair(arena_->Strdup(key),
                                value ? arena_->Strdup(value) : NULL));
  NOVLOG(5) << "Added HTTP header '" << key << "': '"
	  << (value ? value : "(null)") << "'";
  return headers_->back().second;       // pointer to value we just inserted
}

// ----------------------------------------------------------------------
// HTTPHeaders::AppendValueToHeader()
//    Append the given value to the given header, separating it from
//    the current value with the given separator.
//       RETURNS a pointer to the new value for the key we just inserted,
//    where it lives in the array.
//
//    As the previous value is not freed, there are degenerate documents
//    that require O(N^2) memory to process.  E.g. A single header,
//    with the rest of the document being header continuation lines.
// ----------------------------------------------------------------------

const char* HTTPHeaders::AppendValueToHeader(pair<char*,char*>* header,
                                             const char* separator,
                                             const char* value) {
  // We want to limit the size the headers can grow to, to avoid
  // denial-of-service where clients keep sending us lots of data.
  // Ignore any beyond the max.
  if ( IsUsingTooMuchMemory() )
    return NULL;

  const size_t len1 = strlen(header->second);
  const size_t len2 = strlen(separator);
  const size_t len3 = strlen(value);
  // The + 1 is for the \0 terminator.
  char* const new_second = arena_->Alloc(len1 + len2 + len3 + 1);
  strncpy(new_second, header->second, len1 + len2 + len3 + 1);
  strncpy(new_second + len1, separator, len2 + len3 + 1);
  strncpy(new_second + len1 + len2, value, len3 + 1);
  NOVLOG(5) << "Appended to replace HTTP header '"
          << header->first << "':'" << header->second
          << "' with value '" << new_second << "'";
  header->second = new_second;   // temporarily leaks old value in arena
  return header->second;
}

// ----------------------------------------------------------------------
// HTTPHeaders::ClearHeader()
//    Makes it as if the header never was.  This is like
//    SetHeader(hdr, NULL) in that, in both cases, the header won't
//    ever be emitted to the user.  They differ only in how they
//    react to SetHeader(hdr, value, NO_OVERWRITE).  After ClearHeader,
//    this SetHeader will succeed, but after the NULL-SetHeader,
//    it will fail.  RETURNS true if we found the header, false else.
// ----------------------------------------------------------------------

bool HTTPHeaders::ClearHeader(const char* header) {
  bool did_delete = false;
  KeyValueList::iterator it = headers_->begin();
  while ( it != headers_->end() ) {
    if ( !strcasecmp(it->first, header) ) {
      it = headers_->erase(it);
      did_delete = true;
    } else {
      ++it;
    }
  }
  return did_delete;
}

// ----------------------------------------------------------------------
// HTTPHeaders::AddRequestFirstline()
// HTTPHeaders::CheckResponseProtocol()
//
//    AddRequestFirstline() and AddResponseFirstline() know how to parse one
//    line of input: the firstline (HTTP command or response).
//    CheckResponseProtocol() is used by AddResponseFirstline() to parse the
//    protocol (e.g., "HTTP/1.1" part of the first line). It can be
//    overridden to support other protocols (such as Gnutella) that use
//    HTTP-like headers.
//    It's safest if the line you pass in includes a trailing \n
//    (should only be necessary, though, if you use "H 200" shortcut).
// ----------------------------------------------------------------------

#define FAIL(msg) do {                                                      \
  NOLOG(WARNING) << "Request line error ("                                  \
                 << HTTPHeaders::firstline() << "): "  << (msg);            \
  return false;                           /* indicate we're done parsing */ \
} while (0)

bool HTTPHeaders::AddResponseFirstline(const char* firstline, int linelen) {
  assert(firstline);
  firstline_.assign(firstline, linelen);
  const char* const line = firstline_.c_str(); // so that we're NUL terminated
  // One-letter requests can't be valid HTTP.  But we do see (and expect) them
  // in google1-compatibility mode: so while we still fail, we do so silently.
  if ( linelen == 1 )  return false; // FAIL without logging
  const char* const space = strchr(line, ' ');
  if ( !space )  FAIL("no space char");
  const int protolen = space - line;

  if (!CheckResponseProtocol(line, protolen))
    return false;

  char* error;
  const int response_int = strtol(space, &error, 10);
  if ( NULL == error || space == error ||
       (!isspace(*error) && *error != '\0') )
    FAIL("Response code not an integer");
  if ( response_int < HTTPResponse::RC_FIRST_CODE ||
       response_int > HTTPResponse::RC_LAST_CODE ) {
    FAIL("Illegal response code");
  }
  set_response_code(static_cast<HTTPResponse::ResponseCode>(response_int));
  while (isspace(*error))  error++;
  reason_phrase_.assign(error);

  return true;
}

bool HTTPHeaders::CheckResponseProtocol(const char* proto, int protolen) {
  // TODO(michaeln): Gears, do we care to future proof this, HTTP/x.x?
  if ( memcaseis(proto, protolen, "HTTP/1.0") ) {
    set_http_version(HTTP_10);
  } else if ( memcaseis(proto, protolen, "HTTP/1.1") ) {
    set_http_version(HTTP_11);
  } else if ( memcaseis(proto, protolen, "HTTP") ) {
    // If the server didn't provide a version number, then we assume
    // 1.0. This helps with odd servers like the NCSA/1.5.2 server,
    // which does not send a version number if the request is version
    // 1.1. Mozilla does the same - it tolerates this and assumes
    // version 1.0.
    set_http_version(HTTP_10);
/* Gears deletions
  } else if ( memcaseis(proto, protolen, "ICY") ) {
    // For icecast/shoutcast responsed used by say iTune radio stations.
    set_http_version(HTTP_ICY);
  } else if ( memis(proto, protolen, "H") ) {
    // our internal "H<space> ..." form.
    set_http_version(HTTP_OTHER);       // a minimalist response
*/
  } else {
    // We don't LOG here because it's easy to get here by accident:
    // maybe they called AddResponse when they meant to call AddRequest.
    return false; // FAIL without logging
  }
  return true;
}

// ----------------------------------------------------------------------
// HTTPHeaders::SetHeaderFromLine()
//    Like SetHeader(), but we actually parse the response line from
//    a webserver ("Header: value") for you.
//       While the input is a char*, because we munge it as we go,
//    we do not modify it: it's the same after our call as before.
//       RETURNS a pointer to the new value for the key we just inserted,
//    where it lives in the array.
//
//    We handle header continuations. To quote from RFC 2616, section
//    2.2:
//
//     HTTP/1.1 header field values can be folded onto multiple lines
//     if the continuation line begins with a space or horizontal
//     tab. All linear white space, including folding, has the same
//     semantics as SP. A recipient MAY replace any linear white space
//     with a single SP before interpreting the field value or
//     forwarding the message downstream.
// ----------------------------------------------------------------------

const char* HTTPHeaders::SetHeaderFromLine(char* line, int overwrite) {
  // First check for continuation lines. Continuation lines
  // start with some number of space and/or tab characters,
  // which are equivalent to a single space character.
  const char* start = line;
  while ( (*start == ' ') || (*start == '\t') )  start++;
  if ( start > line ) {
    KeyValueList::reverse_iterator last_header = headers_->rbegin();
    if ( last_header == headers_->rend() ) {
      NOLOG(WARNING) << "Ignoring continuation line with no previous header";
      return NULL;
    } else {
      // If the previous header value is non-empty, and this
      // continuation is non-empty, then we need a separator: we
      // collapse the header continuation whitespace into a single
      // space separator (as the RFC says we "MAY" do; see above).
      const char* const separator =
        ((*(last_header->second) != '\0') && (*start != '\0'))  ? " " : "";
      return AppendValueToHeader(&(*last_header), separator, start);
    }
  }

  // This is not a continuation line; hopefully this is a regular "Key: Value".
  char* colon = strchr(line, ':');
  if ( !colon ) {
    NOLOG(WARNING) << "Ignoring invalid header '" << line << "'";
    return NULL;
  } else {
    char *key, *value;            // space used to ingore spaces around :
    key = line;
    value = colon+1;

    while ( colon > key && colon[-1] == ' ' )  colon--;    // spaces before :
    const char oldval = *colon;   // we'll restore this later
    *colon = '\0';                // \0 the end of the key
    while ( *value == ' ' ) value++;                       // spaces after :

    if (!strcasecmp(key, "host")) {
      // a host might already have been set (if the incoming header
      // had an absolute request url as in GET http://host/path HTTP/1.0)
      // the spec says to ignore the "Host: host.com" header in this case
      overwrite = NO_OVERWRITE;
    }
    const char* retval = SetHeader(key, value, overwrite);
    *colon = oldval;              // restore input to original state
    return retval;
  }
}

// ----------------------------------------------------------------------
// HTTPUtils::ParseHTTPHeaders()
//    On entrance to this function, body and bodylen point to a document.
//    At the end, body points past the http headers for this document;
//    it stays unchanged if there don't seem to be any headers.  bodylen
//    is adjusted accordingly.  Body need not be nul-terminated.
//    If we have an error parsing, body is set to NULL and len to 0.
//    If headers is not NULL, the headers will be parsed, and if
//    allow_const_cast is true, the body will be temporarily modified
//    while parsing the headers.  Otherwise header lines are copied to
//    a local string.
//
//    Returns true only when a blank line ('\r\n' only) has been seen.
//    Returns false if an error occurs before then.
//
//    Factored out from both HTTPParser::Parse and HTTPUtils::ExtractBody
// ----------------------------------------------------------------------

bool HTTPUtils::ParseHTTPHeaders(const char** body, uint32* bodylen,
                                 HTTPHeaders* headers, bool allow_const_cast) {
  int num_headers = 0;             // number we've seen

  const char* content = *body;
  uint32 contentlen = *bodylen;
  const char* contentend = content + contentlen;

  *body = NULL;                    // init to impossible values
  *bodylen = 0;

  // doc bodies can be empty
  if (content == NULL || contentlen == 0) return false;

  const char* line = content;

  while (true) {                            // done when we notice header end
    // After this loop runs, line_end will point to the \r of a
    // line-ending \r\n pair or be NULL if there isn't one.
    const char* line_end;
    if (line + 1 >= contentend) {           // not enough space for \r\n
      line_end = NULL;
    } else {
      for (const char* start = line;; start = line_end + 1) {
        line_end = (char *) memchr(start, '\r', contentend - start - 1);
        if (line_end == NULL || line_end[1] == '\n')
          break;
      }
    }

    // doc ends while still in headers?
    if (line_end == NULL) {
      if (num_headers == 0) {              // means there are no headers
        *body = content;                   // so everything is the body
        *bodylen = contentlen;
      } else {                             // assume document was only header
        // this statement was different in HTTPParser::Parse(), so we
        // use the return value to set body = content within Parse()
        *body = content + contentlen;
        *bodylen = 0;
      }
     return false;                         // done due to error
    }

    CHECK_LT(line_end + 1, contentend);

    // If we haven't seen any headers and there's a \n before the \r,
    // the first \r must not have ended a header line.
    if (num_headers == 0 &&
        memchr(line, '\n', line_end - line)) {
      *body = content;
      *bodylen = contentlen;
      return false;                        // done due to error
    }

    if (line_end == line) {                // blank line, means end of headers
      *body = line_end + 2;                // skip over last \r\n
      // If the checks in this block fail, do not try to fix them here.
      // Something upstream almost certainly went past the end of the
      // headers and we're confused now.
      CHECK_GE(*body, content);
      const int headerlen = *body - content;
      CHECK_GE(static_cast<int>(contentlen), headerlen);
      *bodylen = contentlen - headerlen;
      return true;                         // done due to end of headers
    }

    if (num_headers == 0
        && ! memchr(line, ':', line_end - line)) {
      // First header line AND no colon means that it's (hopefully!) a status
      // response line.
      bool good_first_line = false;
      const char *space = (char *) memchr(line, ' ', line_end - line);
      if (space) {
        char *errpos;
        strtol(space + 1, &errpos, 10);
        if (isspace(*errpos) && errpos > space + 1) {
          good_first_line = true;
          num_headers++;
        }
      }
      if (good_first_line) {
        // If we are actually recording the first line, there are a few
        // more validity checks in this AddResponseFirstLine
        if (headers && !headers->AddResponseFirstline(line, line_end - line)) {
          num_headers--;
        }
      } else {
        *body = content;
        *bodylen = contentlen;
        return false;                        // Done -- error parsing first line
      }
    } else {                                 // Not the first parsed header line
      num_headers++;                         //   or we have a line with a colon
      if (headers) {                         // We have to actually parse it
        if (allow_const_cast) {
          // This line should be a header line.
          // Temporarily terminate the line and use SetHeaderFromLine.
          char oldval = *line_end;
          *(const_cast<char *>(line_end)) = '\0';
          headers->SetHeaderFromLine(const_cast<char *>(line),
                                     HTTPHeaders::APPEND);
          *(const_cast<char *>(line_end)) = oldval; // Restore the old value.
        } else {
          // Can't use const_cast so we copy the line to a temporary string
          string header_line(line, line_end);
          header_line.c_str();
          headers->SetHeaderFromLine(&header_line[0], HTTPHeaders::APPEND);
        }
      }
    }

    line = line_end + 2;                   // read past \r and \n
  }
}

// ----------------------------------------------------------------------
// HTTPUtils::ExtractBody()
//    On entrance to this function, body and bodylen point to a document.
//    At the end, body points past the http headers for this document;
//    it stays unchanged if there don't seem to be any headers.  bodylen
//    is adjusted accordingly.  Body need not be nul-terminated.
//       If we have an error parsing, body is set to NULL and len to 0.
//    TODO: make sure this function supports header continuation lines.
// ----------------------------------------------------------------------

void HTTPUtils::ExtractBody(const char **body, uint32* bodylen) {
  ParseHTTPHeaders(body, bodylen, NULL, false);
}
