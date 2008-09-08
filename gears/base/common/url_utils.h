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

#ifndef GEARS_BASE_COMMON_URL_UTILS_H__
#define GEARS_BASE_COMMON_URL_UTILS_H__

#include <vector>

#include "gears/base/common/string16.h" // for string16

bool IsRelativeUrl(const char16 *url);

// Returns a resolved, normalized URL in 'out'.
// Note that that any  fragment identifier present in the input is removed.
// 'url' can be relative or absolute.  'base' can be NULL for absolute URLs.
bool ResolveAndNormalize(const char16 *base, const char16 *url,
                         std::string16 *out);

// Returns true if 'url' is a data URL.
bool IsDataUrl(const char16 *url);

// This method can be used to parse a 'data' URL into its component pieces.
//
// The resulting mime_type is normalized to lowercase.  The data is the
// decoded data (e.g.., if the data URL specifies base64 encoding, then the
// returned data is base64 decoded, and any %-escaped bytes are unescaped).
//
// If the URL is malformed, then this method will return false, and its
// output variables will remain unchanged.  On success, true is returned.
//
bool ParseDataUrl(const std::string16& url, std::string16* mime_type,
                  std::string16* charset, std::vector<uint8>* data);

// Unescapes |escaped_text| and returns the result.
// Unescaping consists of looking for the exact pattern "%XX", where each X is
// a hex digit, and converting to the character with the numerical value of
// those digits.  Thus "i%20=%203%3b" unescapes to "i = 3;".
//
// Watch out: this doesn't necessarily result in the correct final result,
// because the encoding may be unknown. For example, the input might be ASCII,
// which, after unescaping, is supposed to be interpreted as UTF-8, and then
// converted into full wide chars. This function won't tell you if any
// conversions need to take place, it only unescapes.
std::string UnescapeURL(const std::string& escaped_text);
std::string16 UnescapeURL(const std::string16& escaped_text);

// ----------------------------------------------------------------------
// Converts a UTF8 path to a percent encoded "file:///" URL.
// ----------------------------------------------------------------------
enum EscapeUrlFlags {
  ESCAPE_SCHEME        =     1,
  ESCAPE_USERNAME      =     2,
  ESCAPE_PASSWORD      =     4,
  ESCAPE_HOST          =     8,
  ESCAPE_DIRECTORY     =    16,
  ESCAPE_FILEBASENAME  =    32,
  ESCAPE_FILEEXTENSION =    64,
  ESCAPE_PARAM         =   128,
  ESCAPE_QUERY         =   256,
  ESCAPE_REF           =   512,
  // special flags
  ESCAPE_FORCED        =  1024,  // forces escaping of existing escape
                              // sequences
  ESCAPE_ONLYASCII     =  2048,  // causes non-ascii octets to be skipped
  ESCAPE_ONLYNONASCII  =  4096,  // causes _graphic_ ascii octets (0x20-0x7E)
                              // to be skipped when escaping. causes all
                              // ascii octets to be skipped when
                              // unescaping
  ESCAPE_ALWAYSCOPY    =  8192,  // copy input to result buf even if escaping
                              // is unnecessary
  ESCAPE_COLON         = 16384,  // forces escape of colon
  ESCAPE_SKIPCONTROL   = 32768   // skips C0 and DEL from unescaping
};
std::string UTF8PathToUrl(const std::string &path, bool directory);
std::string EscapeUrl(const std::string &source, unsigned int flags);

#endif // GEARS_BASE_COMMON_URL_UTILS_H__
