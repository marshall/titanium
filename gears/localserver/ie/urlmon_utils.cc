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

#include "gears/base/common/common.h"  // for ARRAYSIZE
#include "gears/localserver/common/http_constants.h"
#include "gears/localserver/ie/urlmon_utils.h"
#include "third_party/passthru_app/urlmon_ie7_extras.h"


#ifdef DEBUG
void TraceBindFlags(DWORD flags) {
  LOG16((L"  "));
  if (flags & BINDF_ASYNCHRONOUS)
    LOG16((L"BINDF_ASYNCHRONOUS, "));
  if (flags & BINDF_ASYNCSTORAGE)
    LOG16((L"BINDF_ASYNCSTORAGE, "));
  if (flags & BINDF_NOPROGRESSIVERENDERING)
    LOG16((L"BINDF_NOPROGRESSIVERENDERING, "));
  if (flags & BINDF_OFFLINEOPERATION)
    LOG16((L"BINDF_OFFLINEOPERATION, "));
  if (flags & BINDF_GETNEWESTVERSION)
    LOG16((L"BINDF_GETNEWESTVERSION, "));
  if (flags & BINDF_NOWRITECACHE)
    LOG16((L"BINDF_NOWRITECACHE, "));
  if (flags & BINDF_NEEDFILE)
    LOG16((L"BINDF_NEEDFILE, "));
  if (flags & BINDF_PULLDATA)
    LOG16((L"BINDF_PULLDATA, "));
  if (flags & BINDF_IGNORESECURITYPROBLEM)
    LOG16((L"BINDF_IGNORESECURITYPROBLEM, "));
  if (flags & BINDF_RESYNCHRONIZE)
    LOG16((L"BINDF_RESYNCHRONIZE, "));
  if (flags & BINDF_HYPERLINK)
    LOG16((L"BINDF_HYPERLINK, "));
  if (flags & BINDF_NO_UI)
    LOG16((L"BINDF_NO_UI, "));
  if (flags & BINDF_SILENTOPERATION)
    LOG16((L"BINDF_SILENTOPERATION, "));
  if (flags & BINDF_PRAGMA_NO_CACHE)
    LOG16((L"BINDF_PRAGMA_NO_CACHE, "));
  if (flags & BINDF_GETCLASSOBJECT)
    LOG16((L"BINDF_GETCLASSOBJECT, "));
  if (flags & BINDF_RESERVED_1)
    LOG16((L"BINDF_RESERVED_1, "));
  if (flags & BINDF_FREE_THREADED)
    LOG16((L"BINDF_FREE_THREADED, "));
  if (flags & BINDF_DIRECT_READ)
    LOG16((L"BINDF_DIRECT_READ, "));
  if (flags & BINDF_FORMS_SUBMIT)
    LOG16((L"BINDF_FORMS_SUBMIT, "));
  if (flags & BINDF_GETFROMCACHE_IF_NET_FAIL)
    LOG16((L"BINDF_GETFROMCACHE_IF_NET_FAIL, "));
  if (flags & BINDF_FROMURLMON)
    LOG16((L"BINDF_FROMURLMON, "));
  if (flags & BINDF_FWD_BACK)
    LOG16((L"BINDF_FWD_BACK, "));
  if (flags & BINDF_PREFERDEFAULTHANDLER)
    LOG16((L"BINDF_PREFERDEFAULTHANDLER, "));
  if (flags & BINDF_ENFORCERESTRICTED)
    LOG16((L"BINDF_ENFORCERESTRICTED"));
  LOG16((L"\n"));
}
#endif

#ifdef DEBUG
const char16 *GetBindStatusLabel(DWORD status) {
  const char16 *kUnknownStatus = L"Unknown status";
  const char16 *kBindStatusLabels[] = {
      kUnknownStatus,
      L"BINDSTATUS_FINDINGRESOURCE", //",  // 1,
      L"BINDSTATUS_CONNECTING",  // BINDSTATUS_FINDINGRESOURCE + 1,
      L"BINDSTATUS_REDIRECTING",  // BINDSTATUS_CONNECTING + 1,
      L"BINDSTATUS_BEGINDOWNLOADDATA",  // BINDSTATUS_REDIRECTING + 1,
      L"BINDSTATUS_DOWNLOADINGDATA",  // BINDSTATUS_BEGINDOWNLOADDATA + 1,
      L"BINDSTATUS_ENDDOWNLOADDATA",  // BINDSTATUS_DOWNLOADINGDATA + 1,
      L"BINDSTATUS_BEGINDOWNLOADCOMPONENTS",  // BINDSTATUS_ENDDOWNLOADDATA + 1,
      L"BINDSTATUS_INSTALLINGCOMPONENTS",  // BINDSTATUS_BEGINDOWNLOADCOMPONENTS + 1,
      L"BINDSTATUS_ENDDOWNLOADCOMPONENTS",  // BINDSTATUS_INSTALLINGCOMPONENTS + 1,
      L"BINDSTATUS_USINGCACHEDCOPY",  // BINDSTATUS_ENDDOWNLOADCOMPONENTS + 1,
      L"BINDSTATUS_SENDINGREQUEST",  // BINDSTATUS_USINGCACHEDCOPY + 1,
      L"BINDSTATUS_CLASSIDAVAILABLE",  // BINDSTATUS_SENDINGREQUEST + 1,
      L"BINDSTATUS_MIMETYPEAVAILABLE",  // BINDSTATUS_CLASSIDAVAILABLE + 1,
      L"BINDSTATUS_CACHEFILENAMEAVAILABLE",  // BINDSTATUS_MIMETYPEAVAILABLE + 1,
      L"BINDSTATUS_BEGINSYNCOPERATION",  // BINDSTATUS_CACHEFILENAMEAVAILABLE + 1,
      L"BINDSTATUS_ENDSYNCOPERATION",  // BINDSTATUS_BEGINSYNCOPERATION + 1,
      L"BINDSTATUS_BEGINUPLOADDATA",  // BINDSTATUS_ENDSYNCOPERATION + 1,
      L"BINDSTATUS_UPLOADINGDATA",  // BINDSTATUS_BEGINUPLOADDATA + 1,
      L"BINDSTATUS_ENDUPLOADDATA",  // BINDSTATUS_UPLOADINGDATA + 1,
      L"BINDSTATUS_PROTOCOLCLASSID",  // BINDSTATUS_ENDUPLOADDATA + 1,
      L"BINDSTATUS_ENCODING",  // BINDSTATUS_PROTOCOLCLASSID + 1,
      L"BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE",  // BINDSTATUS_ENCODING + 1,
      L"BINDSTATUS_CLASSINSTALLLOCATION",  // BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE + 1,
      L"BINDSTATUS_DECODING",  // BINDSTATUS_CLASSINSTALLLOCATION + 1,
      L"BINDSTATUS_LOADINGMIMEHANDLER",  // BINDSTATUS_DECODING + 1,
      L"BINDSTATUS_CONTENTDISPOSITIONATTACH",  // BINDSTATUS_LOADINGMIMEHANDLER + 1,
      L"BINDSTATUS_FILTERREPORTMIMETYPE",  // BINDSTATUS_CONTENTDISPOSITIONATTACH + 1,
      L"BINDSTATUS_CLSIDCANINSTANTIATE",  // BINDSTATUS_FILTERREPORTMIMETYPE + 1,
      L"BINDSTATUS_IUNKNOWNAVAILABLE",  // BINDSTATUS_CLSIDCANINSTANTIATE + 1,
      L"BINDSTATUS_DIRECTBIND",  // BINDSTATUS_IUNKNOWNAVAILABLE + 1,
      L"BINDSTATUS_RAWMIMETYPE",  // BINDSTATUS_DIRECTBIND + 1,
      L"BINDSTATUS_PROXYDETECTING",  // BINDSTATUS_RAWMIMETYPE + 1,
      L"BINDSTATUS_ACCEPTRANGES",  // BINDSTATUS_PROXYDETECTING + 1,
      L"BINDSTATUS_COOKIE_SENT",  // BINDSTATUS_ACCEPTRANGES + 1,
      L"BINDSTATUS_COMPACT_POLICY_RECEIVED",  // BINDSTATUS_COOKIE_SENT + 1,
      L"BINDSTATUS_COOKIE_SUPPRESSED",  // BINDSTATUS_COMPACT_POLICY_RECEIVED + 1,
      L"BINDSTATUS_COOKIE_STATE_UNKNOWN",  // BINDSTATUS_COOKIE_SUPPRESSED + 1,
      L"BINDSTATUS_COOKIE_STATE_ACCEPT",  // BINDSTATUS_COOKIE_STATE_UNKNOWN + 1,
      L"BINDSTATUS_COOKIE_STATE_REJECT",  // BINDSTATUS_COOKIE_STATE_ACCEPT + 1,
      L"BINDSTATUS_COOKIE_STATE_PROMPT",  // BINDSTATUS_COOKIE_STATE_REJECT + 1,
      L"BINDSTATUS_COOKIE_STATE_LEASH",  // BINDSTATUS_COOKIE_STATE_PROMPT + 1,
      L"BINDSTATUS_COOKIE_STATE_DOWNGRADE",  // BINDSTATUS_COOKIE_STATE_LEASH + 1,
      L"BINDSTATUS_POLICY_HREF",  // BINDSTATUS_COOKIE_STATE_DOWNGRADE + 1,
      L"BINDSTATUS_P3P_HEADER",  // BINDSTATUS_POLICY_HREF + 1,
      L"BINDSTATUS_SESSION_COOKIE_RECEIVED",  // BINDSTATUS_P3P_HEADER + 1,
      L"BINDSTATUS_PERSISTENT_COOKIE_RECEIVED",  // BINDSTATUS_SESSION_COOKIE_RECEIVED + 1,
      L"BINDSTATUS_SESSION_COOKIES_ALLOWED",  // BINDSTATUS_PERSISTENT_COOKIE_RECEIVED + 1
    };

  if (status > ARRAYSIZE(kBindStatusLabels) - 1) {
    return kUnknownStatus;
  } else {
    return kBindStatusLabels[status];
  }
}

static const char16 *kUnknownOption = L"Unknown option";
static const char16 *kUndefined = L"Undefined";
static const char16 *kWinInetLockHandle = L"WININETINFO_OPTION_LOCK_HANDLE";

// String constants for various options passed to IWinInetInfo::QueryOption,
// Used for logging
static const char16 *kWinInetInfoOptionLabels[] = {
    //
    // options manifests for Internet{Query|Set}Option
    //
    kUndefined,
    L"INTERNET_OPTION_CALLBACK",  //                1
    L"INTERNET_OPTION_CONNECT_TIMEOUT",  //         2
    L"INTERNET_OPTION_CONNECT_RETRIES",  //         3
    L"INTERNET_OPTION_CONNECT_BACKOFF",  //         4
    L"INTERNET_OPTION_SEND_TIMEOUT",  //            5
    L"INTERNET_OPTION_RECEIVE_TIMEOUT",  //         6
    L"INTERNET_OPTION_DATA_SEND_TIMEOUT",  //       7
    L"INTERNET_OPTION_DATA_RECEIVE_TIMEOUT",  //    8
    L"INTERNET_OPTION_HANDLE_TYPE",  //             9
    kUndefined,
    L"INTERNET_OPTION_LISTEN_TIMEOUT",  //          11
    L"INTERNET_OPTION_READ_BUFFER_SIZE",  //        12
    L"INTERNET_OPTION_WRITE_BUFFER_SIZE",  //       13
    kUndefined,
    L"INTERNET_OPTION_ASYNC_ID",  //                15
    L"INTERNET_OPTION_ASYNC_PRIORITY",  //          16
    kUndefined,
    kUndefined,
    kUndefined,
    kUndefined,
    L"INTERNET_OPTION_PARENT_HANDLE",  //           21
    L"INTERNET_OPTION_KEEP_CONNECTION",  //         22
    L"INTERNET_OPTION_REQUEST_FLAGS",  //           23
    L"INTERNET_OPTION_EXTENDED_ERROR",  //          24
    kUndefined,
    L"INTERNET_OPTION_OFFLINE_MODE",  //            26
    L"INTERNET_OPTION_CACHE_STREAM_HANDLE",  //     27
    L"INTERNET_OPTION_USERNAME",  //                28
    L"INTERNET_OPTION_PASSWORD",  //                29
    L"INTERNET_OPTION_ASYNC",  //                   30
    L"INTERNET_OPTION_SECURITY_FLAGS",  //          31
    L"INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT",  // 32
    L"INTERNET_OPTION_DATAFILE_NAME",  //           33
    L"INTERNET_OPTION_URL",  //                     34
    L"INTERNET_OPTION_SECURITY_CERTIFICATE",  //    35
    L"INTERNET_OPTION_SECURITY_KEY_BITNESS",  //    36
    L"INTERNET_OPTION_REFRESH",  //                 37
    L"INTERNET_OPTION_PROXY",  //                   38
    L"INTERNET_OPTION_SETTINGS_CHANGED",  //        39
    L"INTERNET_OPTION_VERSION",  //                 40
    L"INTERNET_OPTION_USER_AGENT",  //              41
    L"INTERNET_OPTION_END_BROWSER_SESSION",  //     42
    L"INTERNET_OPTION_PROXY_USERNAME",  //          43
    L"INTERNET_OPTION_PROXY_PASSWORD",  //          44
    L"INTERNET_OPTION_CONTEXT_VALUE",  //           45
    L"INTERNET_OPTION_CONNECT_LIMIT",  //           46
    L"INTERNET_OPTION_SECURITY_SELECT_CLIENT_CERT",  // 47
    L"INTERNET_OPTION_POLICY",  //                  48
    L"INTERNET_OPTION_DISCONNECTED_TIMEOUT",  //    49
    L"INTERNET_OPTION_CONNECTED_STATE",  //         50
    L"INTERNET_OPTION_IDLE_STATE",  //              51
    L"INTERNET_OPTION_OFFLINE_SEMANTICS",  //       52
    L"INTERNET_OPTION_SECONDARY_CACHE_KEY",  //     53
    L"INTERNET_OPTION_CALLBACK_FILTER",  //         54
    L"INTERNET_OPTION_CONNECT_TIME",  //            55
    L"INTERNET_OPTION_SEND_THROUGHPUT",  //         56
    L"INTERNET_OPTION_RECEIVE_THROUGHPUT",  //      57
    L"INTERNET_OPTION_REQUEST_PRIORITY",  //        58
    L"INTERNET_OPTION_HTTP_VERSION",  //            59
    L"INTERNET_OPTION_RESET_URLCACHE_SESSION",  //  60
    kUndefined,
    L"INTERNET_OPTION_ERROR_MASK",  //              62
    L"INTERNET_OPTION_FROM_CACHE_TIMEOUT",  //      63
    L"INTERNET_OPTION_BYPASS_EDITED_ENTRY",  //     64
    kUndefined,
    kUndefined,
    L"INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO",  //  67
    L"INTERNET_OPTION_CODEPAGE",  //                68
    L"INTERNET_OPTION_CACHE_TIMESTAMPS",  //        69
    L"INTERNET_OPTION_DISABLE_AUTODIAL",  //        70
    kUndefined,
    kUndefined,
    L"INTERNET_OPTION_MAX_CONNS_PER_SERVER",  //     73
    L"INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER",  // 74
    L"INTERNET_OPTION_PER_CONNECTION_OPTION",  //    75
    L"INTERNET_OPTION_DIGEST_AUTH_UNLOAD",  //       76
    L"INTERNET_OPTION_IGNORE_OFFLINE",  //           77
    L"INTERNET_OPTION_IDENTITY",  //                 78
    L"INTERNET_OPTION_REMOVE_IDENTITY",  //          79
    L"INTERNET_OPTION_ALTER_IDENTITY",  //           80
    L"INTERNET_OPTION_SUPPRESS_BEHAVIOR",  //        81
    L"INTERNET_OPTION_AUTODIAL_MODE",  //            82
    L"INTERNET_OPTION_AUTODIAL_CONNECTION",  //      83
    L"INTERNET_OPTION_CLIENT_CERT_CONTEXT",  //      84
    L"INTERNET_OPTION_AUTH_FLAGS",  //               85
    L"INTERNET_OPTION_COOKIES_3RD_PARTY",  //        86
    L"INTERNET_OPTION_DISABLE_PASSPORT_AUTH",  //    87
    L"INTERNET_OPTION_SEND_UTF8_SERVERNAME_TO_PROXY",  //         88
    L"INTERNET_OPTION_EXEMPT_CONNECTION_LIMIT",  //  89
    L"INTERNET_OPTION_ENABLE_PASSPORT_AUTH",  //     90
    L"INTERNET_OPTION_HIBERNATE_INACTIVE_WORKER_THREADS",  //       91
    L"INTERNET_OPTION_ACTIVATE_WORKER_THREADS",  //                 92
    L"INTERNET_OPTION_RESTORE_WORKER_THREAD_DEFAULTS",  //          93
    L"INTERNET_OPTION_SOCKET_SEND_BUFFER_LENGTH",  //               94
    L"INTERNET_OPTION_PROXY_SETTINGS_CHANGED",  //                  95
  };
#endif  // DEBUG

// String constants for various options passed to IWinInetHttpInfo::QueryInfo
// Used for logging and to map from option code to an HTTP header name
// TODO(michaeln): fill in more header_names here
static const struct {
  const char16 *label;        // string representation of the option param
  const char16 *header_name;  // a corresponding http header
} kHttpQueryInfoOptions[] = {
  //
  // HttpQueryInfo info levels. Generally, there is one info level
  // for each potential RFC822/HTTP/MIME header that an HTTP server
  // may send as part of a request response.
  //
  // The HTTP_QUERY_RAW_HEADERS info level is provided for clients
  // that choose to perform their own header parsing.
  //
  // See http://www.ietf.org/rfc/rfc2616.txt for http header infomation.
  //
  {L"HTTP_QUERY_MIME_VERSION",                // 0
   NULL},
  {L"HTTP_QUERY_CONTENT_TYPE",                // 1
   HttpConstants::kContentTypeHeader},
  {L"HTTP_QUERY_CONTENT_TRANSFER_ENCODING",   // 2
   NULL},
  {L"HTTP_QUERY_CONTENT_ID",                  // 3
   NULL},
  {L"HTTP_QUERY_CONTENT_DESCRIPTION",         // 4
   NULL},
  {L"HTTP_QUERY_CONTENT_LENGTH",              // 5
   HttpConstants::kContentLengthHeader},
  {L"HTTP_QUERY_CONTENT_LANGUAGE",            // 6
   NULL},
  {L"HTTP_QUERY_ALLOW",                       // 7
   NULL},
  {L"HTTP_QUERY_PUBLIC",                      // 8
   L"Public"},
  {L"HTTP_QUERY_DATE",                        // 9
   L"Date"},
  {L"HTTP_QUERY_EXPIRES",                     // 10
   L"Expires"},
  {L"HTTP_QUERY_LAST_MODIFIED",               // 11
   HttpConstants::kLastModifiedHeader},   
  {L"HTTP_QUERY_MESSAGE_ID",                  // 12
   NULL},
  {L"HTTP_QUERY_URI",                         // 13
   HttpConstants::kUriHeader},
  {L"HTTP_QUERY_DERIVED_FROM",                // 14
   NULL},
  {L"HTTP_QUERY_COST",                        // 15
   NULL},
  {L"HTTP_QUERY_LINK",                        // 16
   NULL},
  {L"HTTP_QUERY_PRAGMA",                      // 17
   HttpConstants::kPragmaHeader},
  {L"HTTP_QUERY_VERSION",                     // 18 part of status line
   NULL},
  {L"HTTP_QUERY_STATUS_CODE",                 // 19  part of status line
   NULL},
  {L"HTTP_QUERY_STATUS_TEXT",                 // 20  part of status line
   NULL},
  {L"HTTP_QUERY_RAW_HEADERS",                 // 21  all headers as ASCIIZ
   NULL},
  {L"HTTP_QUERY_RAW_HEADERS_CRLF",            // 22  all headers
   NULL},
  {L"HTTP_QUERY_CONNECTION",                  // 23
   NULL},
  {L"HTTP_QUERY_ACCEPT",                      // 24
   NULL},
  {L"HTTP_QUERY_ACCEPT_CHARSET",              // 25
   NULL},
  {L"HTTP_QUERY_ACCEPT_ENCODING",             // 26
   NULL},
  {L"HTTP_QUERY_ACCEPT_LANGUAGE",             // 27
   NULL},
  {L"HTTP_QUERY_AUTHORIZATION",               // 28
   NULL},
  {L"HTTP_QUERY_CONTENT_ENCODING",            // 29
   NULL},
  {L"HTTP_QUERY_FORWARDED",                   // 30
   NULL},
  {L"HTTP_QUERY_FROM",                        // 31
   NULL},   
  {L"HTTP_QUERY_IF_MODIFIED_SINCE",           // 32
   NULL},
  {L"HTTP_QUERY_LOCATION",                    // 33
   NULL},
  {L"HTTP_QUERY_ORIG_URI",                    // 34
   NULL},
  {L"HTTP_QUERY_REFERER",                     // 35
   L"Referer"},
  {L"HTTP_QUERY_RETRY_AFTER",                 // 36
   NULL},
  {L"HTTP_QUERY_SERVER",                      // 37
   L"Server"},
  {L"HTTP_QUERY_TITLE",                       // 38
   NULL},
  {L"HTTP_QUERY_USER_AGENT",                  // 39
   L"User-Agent"},
  {L"HTTP_QUERY_WWW_AUTHENTICATE",            // 40
   NULL},
  {L"HTTP_QUERY_PROXY_AUTHENTICATE",          // 41
   NULL},
  {L"HTTP_QUERY_ACCEPT_RANGES",               // 42
   NULL},
  {L"HTTP_QUERY_SET_COOKIE",                  // 43
   L"Set-Cookie"},
  {L"HTTP_QUERY_COOKIE",                      // 44
   L"Cookie"},
  {L"HTTP_QUERY_REQUEST_METHOD",              // 45  GET/POST etc.
   NULL},
  {L"HTTP_QUERY_REFRESH",                     // 46
   NULL},
  {L"HTTP_QUERY_CONTENT_DISPOSITION",         // 47
   HttpConstants::kContentDispositionHeader},
  //
  // HTTP 1.1 defined headers
  //
  {L"HTTP_QUERY_AGE",                         // 48
   NULL},
  {L"HTTP_QUERY_CACHE_CONTROL",               // 49
   HttpConstants::kCacheControlHeader},
  {L"HTTP_QUERY_CONTENT_BASE",   //                 50
   NULL},
  {L"HTTP_QUERY_CONTENT_LOCATION",   //             51
   NULL},
  {L"HTTP_QUERY_CONTENT_MD5",   //                  52
   NULL},
  {L"HTTP_QUERY_CONTENT_RANGE",   //                53
   NULL},
  {L"HTTP_QUERY_ETAG",   //                      54
   L"ETag"},
  {L"HTTP_QUERY_HOST",   //                      55
   L"Host"},
  {L"HTTP_QUERY_IF_MATCH",   //                     56
   NULL},
  {L"HTTP_QUERY_IF_NONE_MATCH",   //                57
   NULL},
  {L"HTTP_QUERY_IF_RANGE",   //                     58
   NULL},
  {L"HTTP_QUERY_IF_UNMODIFIED_SINCE",   //          59
   NULL},
  {L"HTTP_QUERY_MAX_FORWARDS",   //                 60
   NULL},
  {L"HTTP_QUERY_PROXY_AUTHORIZATION",   //          61
   NULL},
  {L"HTTP_QUERY_RANGE",   //                        62
   NULL},
  {L"HTTP_QUERY_TRANSFER_ENCODING",   //            63
   NULL},
  {L"HTTP_QUERY_UPGRADE",   //                      64
   NULL},
  {L"HTTP_QUERY_VARY",   //                      65
   L"Vary"},
  {L"HTTP_QUERY_VIA",   //                        66
   L"Via"},
  {L"HTTP_QUERY_WARNING",   //                      67
   NULL},
  {L"HTTP_QUERY_EXPECT",   //                       68
   NULL},
  {L"HTTP_QUERY_PROXY_CONNECTION",   //             69
   NULL},
  {L"HTTP_QUERY_UNLESS_MODIFIED_SINCE",   //        70
   NULL},
  {L"HTTP_QUERY_ECHO_REQUEST",   //                 71
   NULL},
  {L"HTTP_QUERY_ECHO_REPLY",  //                   72
   NULL}, 
  // These are the set of headers that should be added back to a request when
  // re-doing a request after a RETRY_WITH response.
  {L"HTTP_QUERY_ECHO_HEADERS",   //                 73
   NULL},
  {L"HTTP_QUERY_ECHO_HEADERS_CRLF",   //            74
   NULL},
  {L"HTTP_QUERY_PROXY_SUPPORT",   //                75
   NULL},
  {L"HTTP_QUERY_AUTHENTICATION_INFO",   //          76
   NULL},
  {L"HTTP_QUERY_PASSPORT_URLS",   //                77
   NULL},
  {L"HTTP_QUERY_PASSPORT_CONFIG",   //              78
   NULL},
};

#ifdef DEBUG
const char16 *GetWinInetInfoLabel(DWORD option) {
  if (option == WININETINFO_OPTION_LOCK_HANDLE) {
    return kWinInetLockHandle;
  } else if (option > ARRAYSIZE(kWinInetInfoOptionLabels) - 1) {
    return kUnknownOption;
  } else {
    return kWinInetInfoOptionLabels[option];
  }
}


const char16 *GetWinInetHttpInfoLabel(DWORD option) {
  if (option == WININETINFO_OPTION_LOCK_HANDLE) {
    return kWinInetLockHandle;
  } else if (option > ARRAYSIZE(kHttpQueryInfoOptions) - 1) {
    return kUnknownOption;
  } else {
    return kHttpQueryInfoOptions[option].label;
  }
}


const char16 *GetProtocolInfoLabel(QUERYOPTION option) {
  static const char16* kQueryOptionLabels[] = {
        L"QUERY_OPTION_INVALID",
        L"QUERY_EXPIRATION_DATE",
        L"QUERY_TIME_OF_LAST_CHANGE",
        L"QUERY_CONTENT_ENCODING",
        L"QUERY_CONTENT_TYPE",
        L"QUERY_REFRESH",
        L"QUERY_RECOMBINE",
        L"QUERY_CAN_NAVIGATE",
        L"QUERY_USES_NETWORK",
        L"QUERY_IS_CACHED",
        L"QUERY_IS_INSTALLEDENTRY",
        L"QUERY_IS_CACHED_OR_MAPPED",
        L"QUERY_USES_CACHE",
        L"QUERY_IS_SECURE",
        L"QUERY_IS_SAFE",
        L"QUERY_USES_HISTORYFOLDER"
      };
  return ((option > 0) && (option <= QUERY_USES_HISTORYFOLDER))
         ? kQueryOptionLabels[option]
         : kQueryOptionLabels[0];
}

#endif  // DEBUG


const char16 *GetWinInetHttpInfoHeaderName(DWORD option) {
  if (option > ARRAYSIZE(kHttpQueryInfoOptions) - 1) {
    return NULL;
  } else {
    return kHttpQueryInfoOptions[option].header_name;
  }
}
