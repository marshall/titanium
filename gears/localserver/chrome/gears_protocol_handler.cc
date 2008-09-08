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

#include "gears/localserver/chrome/gears_protocol_handler.h"

#include <map>

#include "gears/base/chrome/module_cr.h"
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/http_constants.h"
#include "third_party/googleurl/src/gurl.h"
#include "third_party/googleurl/src/url_util.h"

// TODO(mpcomplete): maybe this resource fetching should be generalized?
#ifdef WIN32
#include "gears/base/ie/atl_headers.h"
#define HTML MAKEINTRESOURCE(23)

static LANGID LocaleCodeToLANGID(const std::string16 &locale) {
  // This table was pulled from "HKEY_CLASSES_ROOT\MIME\Database\Rfc1766".
  struct {
    LANGID langid;
    const wchar_t *locale;
  } table[] = {
    { 0x0436, L"af" },
    { 0x041C, L"sq" },
    { 0x0001, L"ar" },
    { 0x0401, L"ar-sa" },
    { 0x0801, L"ar-iq" },
    { 0x0C01, L"ar-eg" },
    { 0x1001, L"ar-ly" },
    { 0x1401, L"ar-dz" },
    { 0x1801, L"ar-ma" },
    { 0x1C01, L"ar-tn" },
    { 0x2001, L"ar-om" },
    { 0x2401, L"ar-ye" },
    { 0x2801, L"ar-sy" },
    { 0x2C01, L"ar-jo" },
    { 0x3001, L"ar-lb" },
    { 0x3401, L"ar-kw" },
    { 0x3801, L"ar-ae" },
    { 0x3C01, L"ar-bh" },
    { 0x4001, L"ar-qa" },
    { 0x042D, L"eu" },
    { 0x0402, L"bg" },
    { 0x0423, L"be" },
    { 0x0403, L"ca" },
    { 0x0004, L"zh" },
    { 0x0404, L"zh-tw" },
    { 0x0804, L"zh-cn" },
    { 0x0C04, L"zh-hk" },
    { 0x1004, L"zh-sg" },
    { 0x041A, L"hr" },
    { 0x0405, L"cs" },
    { 0x0406, L"da" },
    { 0x0413, L"nl" },
    { 0x0813, L"nl-be" },
    { 0x0009, L"en" },
    { 0x0409, L"en-us" },
    { 0x0809, L"en-gb" },
    { 0x0C09, L"en-au" },
    { 0x1009, L"en-ca" },
    { 0x1409, L"en-nz" },
    { 0x1809, L"en-ie" },
    { 0x1C09, L"en-za" },
    { 0x2009, L"en-jm" },
    { 0x2809, L"en-bz" },
    { 0x2C09, L"en-tt" },
    { 0x0425, L"et" },
    { 0x0438, L"fo" },
    { 0x0429, L"fa" },
    { 0x040B, L"fi" },
    { 0x040C, L"fr" },
    { 0x080C, L"fr-be" },
    { 0x0C0C, L"fr-ca" },
    { 0x100C, L"fr-ch" },
    { 0x140C, L"fr-lu" },
    { 0x043C, L"gd" },
    { 0x0407, L"de" },
    { 0x0807, L"de-ch" },
    { 0x0C07, L"de-at" },
    { 0x1007, L"de-lu" },
    { 0x1407, L"de-li" },
    { 0x0408, L"el" },
    { 0x040D, L"he" },
    { 0x0439, L"hi" },
    { 0x040E, L"hu" },
    { 0x040F, L"is" },
    { 0x0421, L"in" },
    { 0x0410, L"it" },
    { 0x0810, L"it-ch" },
    { 0x0411, L"ja" },
    { 0x0412, L"ko" },
    { 0x0426, L"lv" },
    { 0x0427, L"lt" },
    { 0x042F, L"mk" },
    { 0x043E, L"ms" },
    { 0x043A, L"mt" },
    { 0x0414, L"no" },
    { 0x0814, L"no" },
    { 0x0415, L"pl" },
    { 0x0416, L"pt-br" },
    { 0x0816, L"pt" },
    { 0x0417, L"rm" },
    { 0x0418, L"ro" },
    { 0x0818, L"ro-mo" },
    { 0x0419, L"ru" },
    { 0x0819, L"ru-mo" },
    { 0x0C1A, L"sr" },
    { 0x081A, L"sr" },
    { 0x041B, L"sk" },
    { 0x0424, L"sl" },
    { 0x042E, L"sb" },
    { 0x040A, L"es" },
    { 0x080A, L"es-mx" },
    { 0x0C0A, L"es" },
    { 0x100A, L"es-gt" },
    { 0x140A, L"es-cr" },
    { 0x180A, L"es-pa" },
    { 0x1C0A, L"es-do" },
    { 0x200A, L"es-ve" },
    { 0x240A, L"es-co" },
    { 0x280A, L"es-pe" },
    { 0x2C0A, L"es-ar" },
    { 0x300A, L"es-ec" },
    { 0x340A, L"es-cl" },
    { 0x380A, L"es-uy" },
    { 0x3C0A, L"es-py" },
    { 0x400A, L"es-bo" },
    { 0x440A, L"es-sv" },
    { 0x480A, L"es-hn" },
    { 0x4C0A, L"es-ni" },
    { 0x500A, L"es-pr" },
    { 0x0430, L"sx" },
    { 0x041D, L"sv" },
    { 0x081D, L"sv-fi" },
    { 0x041E, L"th" },
    { 0x0431, L"ts" },
    { 0x0432, L"tn" },
    { 0x041F, L"tr" },
    { 0x0422, L"uk" },
    { 0x0420, L"ur" },
    { 0x042A, L"vi" },
    { 0x0434, L"xh" },
    { 0x043D, L"ji" },
    { 0x0435, L"zu" },
  };

  // Cache the LANGID since locale does not change for the life of the program.
  static LANGID cached_langid = 0;
  if (cached_langid != 0)
    return cached_langid;

  std::string16 locale_lower = MakeLowerString(locale);

  for (size_t i = 0; i < ARRAYSIZE(table); ++i) {
    if (locale_lower == table[i].locale)
      return (cached_langid = table[i].langid);
  }

  LOG16((L"Could not find LANGID for locale '%s'\n", locale.c_str()));
  assert(false);
  return (cached_langid = LANG_USER_DEFAULT);
}

// Returns true if we have a resource with the given name in the DLL.
static bool HasResource(const std::string16 &path) {
  HMODULE hmodule = _AtlBaseModule.GetModuleInstance();
  return FindResource(hmodule, path.c_str(), HTML) != NULL;
}

// Returns a pointer to a resource contained in the DLL.
static bool FetchResource(const std::string16 &path, void **data, int *len) {
  HMODULE hmodule = _AtlBaseModule.GetModuleInstance();
  LANGID langid = LocaleCodeToLANGID(CP::locale());
  HRSRC hresource = FindResourceEx(hmodule, HTML, path.c_str(), langid);

  if (!hresource) {
    // If we don't have locale-specific data, fall back to the generic data.
    // This can happen for image files, for example.
    hresource = FindResource(hmodule, path.c_str(), HTML);
    if (!hresource)
      return false;
  }

  HGLOBAL hdata = LoadResource(hmodule, hresource);
  if (!hdata)
    return false;

  *data = LockResource(hdata);
  *len = SizeofResource(hmodule, hresource);
  return true;
}
#endif

static const char kGearsScheme[] = "gears";
static const char kResourceSource[] = "resources";
static const char kDebugSource[] = "debug";

// Return true if the given URL is a "gears:" URL.
static bool IsGearsProtocol(const std::string16 &url) {
  return url_util::FindAndCompareScheme(url.data(), url.size(),
                                        kGearsScheme, NULL);
}

// Extract the source and path from an URL, and return true on success.
static bool GearsUrlToPath(const std::string16 &url,
                      std::string *source, std::string *path) {
  if (!IsGearsProtocol(url))
    return false;

  GURL gurl(url);  // Canonicalize the URL.
  if (!gurl.is_valid())
    return false;

  size_t slashpos = gurl.path().find('/');
  if (slashpos == std::string::npos)
    return false;

  source->assign(gurl.host());
  path->assign(gurl.path().substr(slashpos + 1));
  return true;
}

// Returns the mime type based on the extension.
static bool GetMimeTypeFromExtension(const std::string16 &path,
                                     std::string16 *mime_type) {
  size_t dotpos = path.rfind('.');
  if (dotpos == std::string16::npos)
    return false;

  // This list is very cheesy.  This is all the types we serve at the moment.
  std::string16 extension(path, dotpos + 1);
  if (extension == STRING16(L"html")|| extension == STRING16(L"htm")) {
    *mime_type = STRING16(L"text/html");
  } else if (extension == STRING16(L"css")) {
    *mime_type = STRING16(L"text/css");
  } else if (extension == STRING16(L"js")) {
    *mime_type = STRING16(L"application/x-javascript");
  } else if (extension == STRING16(L"gif")) {
    *mime_type = STRING16(L"image/gif");
  } else if (extension == STRING16(L"jpg") || extension == STRING16(L"jpeg")) {
    *mime_type = STRING16(L"image/jpeg");
  } else if (extension == STRING16(L"png")) {
    *mime_type = STRING16(L"image/png");
  } else {
    return false;
  }
  return true;
}


// Sets up the given payload object the given data and mime_type.
static void InitPayload(const void *data, int data_len,
                        const std::string16 &mime_type,
                        WebCacheDB::PayloadInfo *payload) {
  const std::string16 kColon = STRING16(L": ");
  const std::string16 kCrLf = HttpConstants::kCrLf;

  payload->id = 0;
  payload->creation_date = 0;
  payload->status_code = 200;
  payload->status_line = STRING16(L"HTTP/1.1 200 OK");
  payload->headers =
      HttpConstants::kContentTypeHeader + kColon + mime_type +
      kCrLf +
      HttpConstants::kContentLengthHeader + kColon +
      IntegerToString16(data_len) +
      kCrLf + kCrLf;
  payload->data.reset(new std::vector<uint8>);
  payload->is_synthesized_http_redirect = false;
  payload->data->resize(data_len);
  memcpy(&(*payload->data)[0], data, data_len);
}

// Handle gears://resource/* requests.
static bool ServiceResource(const std::string16 &path,
                            WebCacheDB::PayloadInfo *payload) {
  void *data;
  int len;
  if (!FetchResource(path, &data, &len))
    return false;

  std::string16 mime_type;
  if (!GetMimeTypeFromExtension(path, &mime_type)) {
    // We shouldn't have a resource with an unknown extension.
    assert(false);
    return false;
  }

  InitPayload(data, len, mime_type, payload);
  return true;
}

// Handle gears://debug/* requests.
static bool ServiceDebug(const std::string16 &path,
                         WebCacheDB::PayloadInfo *payload) {
  static const char kOfflineData[] =
    "<body>You are now offline!  Visit "
    "<a href='gears://debug/online'>gears://debug/online</a>"
    " to go online again.</body>";
  static const char kOnlineData[] = "<body>You are now online!</body>";

  std::string16 mime_type = STRING16(L"text/html");
  if (path == STRING16(L"offline")) {
    CP::set_offline_debug_mode(true);
    InitPayload(kOfflineData, ARRAYSIZE(kOfflineData) - 1, mime_type, payload);
    return true;
  } else if (path == STRING16(L"online")) {
    CP::set_offline_debug_mode(false);
    InitPayload(kOnlineData, ARRAYSIZE(kOnlineData) - 1, mime_type, payload);
    return true;
  }

  return false;
}

// static
bool GearsProtocolHandler::Service(const std::string16 &url,
                                   WebCacheDB::PayloadInfo *payload) {
  std::string source, path;
  if (!GearsUrlToPath(url, &source, &path))
    return false;

  std::string16 path16;
  if (!UTF8ToString16(path.data(), path.length(), &path16))
    return false;

  if (source == kResourceSource) {
    return ServiceResource(path16, payload);
  } else if (source == kDebugSource) {
    return ServiceDebug(path16, payload);
  }

  return false;
}
