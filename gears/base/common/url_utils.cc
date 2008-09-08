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

#include "gears/base/common/url_utils.h"

#include "gears/base/common/base64.h"
#include "gears/base/common/string_utils.h"
#include "third_party/googleurl/src/gurl.h"

bool IsRelativeUrl(const char16 *url) {
  // From RFC 2396 (URI Generic Syntax):
  // * "Relative URI references are distinguished from absolute URI in that
  //    they do not begin with a scheme name."
  // * "Scheme names consist of a sequence of characters beginning with a
  //    lower case letter and followed by any combination of lower case
  //    letters, digits, plus ('+'), period ('.'), or hyphen ('-').  For
  //    resiliency, programs interpreting URI should treat upper case letters
  //    as equivalent to lower case in scheme names (e.g., allow 'HTTP' as
  //    well as 'http')."
  // The algorithm below does not support escaped characters.

  bool first_is_alpha = (url[0] >= 'a' && url[0] <= 'z') || 
                        (url[0] >= 'A' && url[0] <= 'Z');
  if (first_is_alpha) {
    int i = 1;
    while ((url[i] >= 'a' && url[i] <= 'z') || 
           (url[i] >= 'A' && url[i] <= 'Z') ||
           (url[i] >= '0' && url[i] <= '9') ||
           (url[i] == '+') || (url[i] == '.') || (url[i] == '-')) {
      ++i;
    }
    if (url[i] == ':') {
      return false;  // absolute URL
    }
  }
  return true;
}

bool IsDataUrl(const char16 *url) {
  return std::char_traits<char16>::compare(url, STRING16(L"data:"), 5) == 0;
}

// NOTE: based loosely on mozilla's nsDataChannel.cpp
bool ParseDataUrl(const std::string16& url, std::string16* mime_type,
                  std::string16* charset, std::vector<uint8>* data) {
  std::string16::const_iterator begin = url.begin();
  std::string16::const_iterator end = url.end();

  std::string16::const_iterator after_colon = std::find(begin, end, ':');
  if (after_colon == end)
    return false;
  ++after_colon;

  // first, find the start of the data
  std::string16::const_iterator comma = std::find(after_colon, end, ','); 
  if (comma == end)
    return false;

  const std::string16 kBase64Tag = STRING16(L";base64");
  std::string16::const_iterator it =
      std::search(after_colon, comma, kBase64Tag.begin(), kBase64Tag.end());

  bool base64_encoded = (it != comma);

  if (comma != after_colon) {
    // everything else is content type
    std::string16::const_iterator semi_colon =
        std::find(after_colon, comma, ';');
    if (semi_colon != after_colon) {
      mime_type->assign(after_colon, semi_colon);
      LowerString(*mime_type);
    }
    if (semi_colon != comma) {
      const std::string16 kCharsetTag = STRING16(L"charset=");
      it = std::search(semi_colon + 1, comma, kCharsetTag.begin(),
                       kCharsetTag.end());
      if (it != comma)
        charset->assign(it + kCharsetTag.size(), comma);
    }
  }

  // fallback to defaults if nothing specified in the URL:
  if (mime_type->empty())
    mime_type->assign(STRING16(L"text/plain"));
  if (charset->empty())
    charset->assign(STRING16(L"US-ASCII"));

  // Convert the data portion to single byte.
  std::string16 temp_data16(comma + 1, end);
  std::string temp_data;
  if (!String16ToUTF8(temp_data16.c_str(), &temp_data))
    return false;

  // Preserve spaces if dealing with text or xml input, same as mozilla:
  //   https://bugzilla.mozilla.org/show_bug.cgi?id=138052
  // but strip them otherwise:
  //   https://bugzilla.mozilla.org/show_bug.cgi?id=37200
  // (Spaces in a data URL should be escaped, which is handled below, so any
  // spaces now are wrong. People expect to be able to enter them in the URL
  // bar for text, and it can't hurt, so we allow it.)
  temp_data = UnescapeURL(temp_data);

  if (base64_encoded ||
      !(mime_type->compare(0, 5, STRING16(L"text/")) == 0 ||
        mime_type->find(STRING16(L"xml")) != std::string16::npos)) {
    temp_data.erase(std::remove_if(temp_data.begin(), temp_data.end(), isspace),
                    temp_data.end());
  }

  if (base64_encoded)
    return Base64Decode(temp_data, data);

  data->assign(temp_data.begin(), temp_data.end());
  return true;
}


template <class str>
str UnescapeURLImpl(const str& escaped_text, bool replace_plus) {
  if (escaped_text.length() < 3 && !replace_plus)
    return escaped_text;  // Can't possibly have an escaped char

  // The output of the unescaping is always smaller than the input, so we can
  // reserve the input size to make sure we have enough buffer and don't have
  // to allocate in the loop below.
  str result;
  result.reserve(escaped_text.length());

  for (size_t i = 0, max = escaped_text.size(); i < max; ++i) {
     if (escaped_text[i] == '%' && i + 2 < max) {
      const typename str::value_type most_sig_digit(escaped_text[i + 1]);
      const typename str::value_type least_sig_digit(escaped_text[i + 2]);
      if (IsHex(most_sig_digit) && IsHex(least_sig_digit)) {
        result.push_back((HexToInt(most_sig_digit) * 16) +
                         HexToInt(least_sig_digit));
        i += 2;
      } else {
        result.push_back('%');
      }
    } else if (replace_plus && escaped_text[i] == '+') {
      result.push_back(' ');
    } else {
      result.push_back(escaped_text[i]);
    }
  }

  return result;
}

std::string UnescapeURL(const std::string& escaped_text) {
  return UnescapeURLImpl(escaped_text, false);
}

std::string16 UnescapeURL(const std::string16& escaped_text) {
  return UnescapeURLImpl(escaped_text, false);
}


//------------------------------------------------------------------------------
// UTF8PathToUrl
//------------------------------------------------------------------------------
// This is based on net_GetURLSpecFromFile in Firefox.  See:
// http://lxr.mozilla.org/seamonkey/source/netwerk/base/src/nsURLHelperWin.cpp
std::string UTF8PathToUrl(const std::string &path, bool directory) {
  std::string result("file:///");

  result += path;

  // Normalize all backslashes to forward slashes.
  size_t loc = result.find('\\');
  while (loc != std::string::npos) {
    result.replace(loc, 1, 1, '/');
    loc = result.find('\\', loc + 1);
  }

  result = EscapeUrl(result, ESCAPE_DIRECTORY|ESCAPE_FORCED);

  // EscapeUrl doesn't escape semi-colons, so do that here.
  loc = result.find(';');
  while (loc != std::string::npos) {
    result.replace(loc, 1, "%3B");
    loc = result.find('\\', loc + 1);
  }

  // If this is a directory, we need to make sure a slash is at the end.
  if (directory && result.c_str()[result.length() - 1] != '/') {
    result += "/";
  }

  return result;
}

//------------------------------------------------------------------------------
// EscapeUrl
//------------------------------------------------------------------------------
// This is based on NS_EscapeURL from Firefox.
// See: http://lxr.mozilla.org/seamonkey/source/xpcom/io/nsEscape.cpp

static const unsigned char HEX_ESCAPE = '%';
static const int escape_chars[256] =
/*  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
{
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 0x */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 1x */
     0,1023,   0, 512,1023,   0,1023,1023,1023,1023,1023,1023,1023,1023, 953, 784,       /* 2x   !"#$%&'()*+,-./ */
  1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1008, 912,   0,1008,   0, 768,       /* 3x  0123456789:;<=>? */
  1008,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,       /* 4x  @ABCDEFGHIJKLMNO  */
  1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023, 896, 896, 896, 896,1023,       /* 5x  PQRSTUVWXYZ[\]^_ */
     0,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,       /* 6x  `abcdefghijklmno */
  1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023, 896,1012, 896,1023,   0,       /* 7x  pqrstuvwxyz{|}~ */
     0   /* 8x  DEL               */
};

#define NO_NEED_ESC(C) (escape_chars[((unsigned int) (C))] & (flags))

//  Returns an escaped string.

std::string EscapeUrl(const std::string &source, unsigned int flags) {
  std::string result;
  unsigned int i = 0;
  static const char hex_chars[] = "0123456789ABCDEF";
  bool forced = (flags & ESCAPE_FORCED) != 0;
  bool ignore_non_ascii = (flags & ESCAPE_ONLYASCII) != 0;
  bool ignore_ascii = (flags & ESCAPE_ONLYNONASCII) != 0;
  bool colon = (flags & ESCAPE_COLON) != 0;

  char temp_buffer[100];
  unsigned int temp_buffer_pos = 0;

  bool previous_is_non_ascii = false;
  for (i = 0; i < source.length(); ++i) {
    unsigned char c = source.c_str()[i];

    // if the char has not to be escaped or whatever follows % is
    // a valid escaped string, just copy the char.
    //
    // Also the % will not be escaped until forced
    // See bugzilla bug 61269 for details why we changed this
    //
    // And, we will not escape non-ascii characters if requested.
    // On special request we will also escape the colon even when
    // not covered by the matrix.
    // ignoreAscii is not honored for control characters (C0 and DEL)
    //
    // And, we should escape the '|' character when it occurs after any
    // non-ASCII character as it may be part of a multi-byte character.
    if ((NO_NEED_ESC(c) || (c == HEX_ESCAPE && !forced)
         || (c > 0x7f && ignore_non_ascii)
         || (c > 0x1f && c < 0x7f && ignore_ascii))
        && !(c == ':' && colon)
        && !(previous_is_non_ascii && c == '|' && !ignore_non_ascii)) {
      temp_buffer[temp_buffer_pos] = c;
      ++temp_buffer_pos;
    }
    else {
      // Do the escape magic.
      temp_buffer[temp_buffer_pos] = HEX_ESCAPE;
      temp_buffer[temp_buffer_pos + 1] = hex_chars[c >> 4];  // high nibble
      temp_buffer[temp_buffer_pos + 2] = hex_chars[c & 0x0f];  // low nibble
      temp_buffer_pos += 3;
    }

    if (temp_buffer_pos >= sizeof(temp_buffer) - 4) {
      temp_buffer[temp_buffer_pos] = '\0';
      result += temp_buffer;
      temp_buffer_pos = 0;
    }

    previous_is_non_ascii = (c > 0x7f);
  }

  temp_buffer[temp_buffer_pos] = '\0';
  result += temp_buffer;

  return result;
}

bool ResolveAndNormalize(const char16 *base, const char16 *url,
                         std::string16 *out) {
  assert(url && out);
  GURL gurl;
  if (base != NULL) {
    GURL base_gurl(base);
    if (!base_gurl.is_valid()) {
      return false;
    }
    gurl = base_gurl.Resolve(url);
  } else {
    gurl = GURL(url);
  }

  if (!gurl.is_valid()) {
    return false;
  }

  // strip the fragment part of the url
  GURL::Replacements strip_fragment;
  strip_fragment.SetRef("", url_parse::Component());
  gurl = gurl.ReplaceComponents(strip_fragment);

  if (!UTF8ToString16(gurl.spec().c_str(), gurl.spec().length(), out)) {
    return false;
  }

  return true;
}
