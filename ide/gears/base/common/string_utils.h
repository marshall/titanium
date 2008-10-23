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

#ifndef GEARS_BASE_COMMON_STRING_UTILS_H__
#define GEARS_BASE_COMMON_STRING_UTILS_H__

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <string>
#include <vector>
#include "gears/base/common/string16.h" // for char16

// ----------------------------------------------------------------------
// Upper/Lower case conversions for std::basic_string
// ----------------------------------------------------------------------

// Converts a string to lower case in place
template<class StringT>
inline void LowerString(StringT &str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 static_cast<int(*)(int)>(std::tolower));
}
// Converts a string to upper case in place
template<class StringT>
inline void UpperString(StringT &str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 static_cast<int(*)(int)>(std::toupper));
}

// Returns a lower case version of the string
template<class StringT>
inline StringT MakeLowerString(const StringT &src) {
  StringT dest(src);
  LowerString(dest);
  return dest;
}
// Returns an upper case version of the string
template<class StringT>
inline StringT MakeUpperString(const StringT &src) {
  StringT dest(src);
  UpperString(dest);
  return dest;
}

// ----------------------------------------------------------------------
// StripWhiteSpace
//
//    Removes whitespace from both sides of string.  Pass in a
//    pointer to an array of characters, and its length.  The function
//    changes the pointer and length to refer to a substring that does
//    not contain leading or trailing spaces.
//
//    Note: to be completely type safe, this function should be
//    parameterized as a template: template<typename anyChar> void
//    StripWhiteSpace(anyChar** str, int* len), where the expectation
//    is that anyChar could be char, const char, w_char, const w_char,
//    unicode_char, or any other character type we want.  However, we
//    just provided a version for char and const char.  C++ is
//    inconvenient, but correct, here.  Ask Amit is you want to know
//    the type safety details.
// ----------------------------------------------------------------------
template<class CharT>
inline void StripWhiteSpace(const CharT** str, int* len) {
  // strip off trailing whitespace
  while ((*len) > 0 && std::isspace((*str)[(*len) - 1])) {
    (*len)--;
  }
  // strip off leading whitespace
  while ((*len) > 0 && std::isspace((*str)[0])) {
    (*len)--;
    (*str)++;
  }
}

// StripWhiteSpace, but taking and returning a std::basic_string<T>.
template<class T>
inline std::basic_string<T> StripWhiteSpace(const std::basic_string<T> &str) {
  const T *start = str.data();
  int len = static_cast<int>(str.size());
  StripWhiteSpace(&start, &len);
  return std::basic_string<T>(start, len);
}

// ----------------------------------------------------------------------
// memmatch and memstr functions
//
//  These behave similarly to the CRT strstr() function allowing callers to
//  search a string for a pattern and either get a pointer to the first
//  occurrence of the pattern in the string or null if the pattern was not
//  found.
//  - 1- and 2-byte chars are supported
//  - Case sensitive and insensitive searching is supported
//  - The needle and haystack can contain embedded nulls
//  - Function templates accepting a variety of input arguments are provided
//  - Length arguments specify the number of characters, not bytes
// ----------------------------------------------------------------------

const char *memmatch(const char *haystack, size_t haylen,
                     const char *needle, size_t neelen,
                     bool case_sensitive);

const char16 *memmatch(const char16 *haystack, size_t haylen,
                       const char16 *needle, size_t neelen,
                       bool case_sensitive);

// memstr - case-sensitive variants

// memstr function template taking null-terminated C-string arguments
template<class CharT>
inline const CharT *memstr(const CharT *haystack, const CharT *needle) {
  return memmatch(haystack, std::char_traits<CharT>::length(haystack),
                  needle, std::char_traits<CharT>::length(needle), true);
}

// memstr function template taking C-string arguments
template<class CharT>
inline const CharT *memstr(const CharT *haystack, size_t haylen,
                           const CharT *needle, size_t needlelen) {
  return memmatch(haystack, haylen, needle, needlelen, true);
}

// memstr function template taking a mix of C and C++ string arguments
template<class StringT>
inline const typename StringT::value_type
    *memstr(const typename StringT::value_type *haystack,
            size_t haylen,
            const StringT &needle) {
  return memmatch(haystack, haylen, needle.c_str(), needle.length(), true);
}

// memstr function template taking C++ string arguments
template<class StringT>
inline const typename StringT::value_type
    *memstr(const StringT &haystack,
            const StringT &needle) {
  return memmatch(haystack.c_str(), haystack.length(),
                  needle.c_str(), needle.length(), true);
}

// memistr - case-insensitive variants

// memstr function template taking null-terminated C string arguments
template<class CharT>
inline const CharT *memistr(const CharT *haystack, const CharT *needle) {
  return memmatch(haystack, std::char_traits<CharT>::length(haystack),
                  needle, std::char_traits<CharT>::length(needle), false);
}

// memistr function template taking C string arguments
template<class CharT>
inline const CharT *memistr(const CharT *haystack, size_t haylen,
                            const CharT *needle, size_t needlelen) {
  return memmatch(haystack, haylen, needle, needlelen, false);
}

// memistr function template taking a mix of C and C++ string arguments
template<typename StringT>
inline const typename StringT::value_type
    *memistr(const typename StringT::value_type *haystack,
             size_t haylen,
             const StringT &needle) {
  return memmatch(haystack, haylen, needle.c_str(), needle.length(), false);
}

// memistr function template taking C++ string arguments
template<class StringT>
inline const typename StringT::value_type
    *memistr(const StringT &haystack,
             const StringT &needle) {
  return memmatch(haystack.c_str(), haystack.length(),
                  needle.c_str(), needle.length(), false);
}

// Returns true if and only if the entire string (other than terminating null)
// consists entirely of characters meeting the following criteria:
//
// - visible ASCII
// - None of the following characters: / \ : * ? " < > | ; ,
// - Doesn't start with a dot.
// - Doesn't end with a dot.
//
// This function is a heuristic that should identify most strings that are
// invalid pathnames on popular OSes. It's both overinclusive and
// underinclusive, though. Thus, TODO(miket): rather than a test, convert
// string to something that's sure to be a legal pathname.
template<class CharT>
inline bool IsStringValidPathComponent(const CharT *s) {
  if (s == NULL) {
    return false;
  }

  // an empty string is considered valid, callers depend on this
  if (*s == 0) {
    return true;
  }

  // Does it start with a dot?
  if (*s == '.') {
    return false;
  }
  // Is every char valid?
  while (CharT c = *s++) {
    if (!IsCharValidInPathComponent(c, true)) {
      return false;
    }
  }
  // Does it end with a dot?
  s -= 2;
  return (*s != '.');
}

// Modifies a string, replacing characters that are not valid in a file path
// component with the '_' character. Also replaces leading and trailing dots
// with the '_' character.  If 'strict' is true, we also disallow non-ASCII
// characters.
// See IsCharValidInPathComponent
template<class StringT>
inline void EnsureStringValidPathComponent(StringT &s, bool strict) {
  if (s.empty()) {
    return;
  }

  typename StringT::iterator iter = s.begin();
  typename StringT::iterator end = s.end();

  // Does it start with a dot?
  if (*iter == '.') {
    *iter = '_';
    ++iter;  // skip it in the loop below
  }
  // Is every char valid?
  while (iter != end) {
    if (!IsCharValidInPathComponent(*iter, strict)) {
      *iter = '_';
    }
    ++iter;
  }
  // Does it end with a dot?
  --iter;
  if (*iter == '.') {
    *iter = '_';
  }
}

// Returns true if and only if the char meets the following criteria:
//
// - visible ASCII
// - None of the following characters: / \ : * ? " < > | ; ,
// - if strict, no spaces or characters above 126.
//
// This function is a heuristic that should identify most strings that are
// invalid pathnames on popular OSes. It's both overinclusive and
// underinclusive, though.
template<class CharT>
inline bool IsCharValidInPathComponent(CharT c, bool strict) {
  // Not visible ASCII?
  if (c < 32) {
    return false;
  }
  if (strict && (c == 32 || c >= 127)) {
    return false;
  }

  switch (c) {
    case '/':
    case '\\':
    case ':':
    case '*':
    case '?':
    case '"':
    case '<':
    case '>':
    case '|':
    case ';':
    case ',':
      return false;

    default:
      return true;
  }
}

// ----------------------------------------------------------------------
// Hexadecimal <-> integer conversions
// ----------------------------------------------------------------------

template <class CharT>
inline bool IsHex(CharT ch) {
  return (ch >= '0' && ch <= '9') ||
         (ch >= 'A' && ch <= 'F') ||
         (ch >= 'a' && ch <= 'f');
}

template <class CharT>
inline CharT HexToInt(CharT ch) {
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'A' && ch <= 'F')
    return ch - 'A' + 10;
  if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  assert(false);
  return 0;
}

inline char IntToHex(int i) {
  static const char* const kHexString = "0123456789ABCDEF";
  assert(i >= 0 && i <= 15);
  return kHexString[i];
}

// ----------------------------------------------------------------------
// Character encoding conversions
// ----------------------------------------------------------------------

bool UTF8ToString16(const char *in, int len, std::string16 *out16);

inline bool UTF8ToString16(const char *in, std::string16 *out16) {
  assert(in);
  return UTF8ToString16(in, strlen(in), out16);
}

inline bool UTF8ToString16(const std::string &in, std::string16 *out16) {
  return UTF8ToString16(in.data(), in.size(), out16);
}

// Returns empty string on error. Not distinguished from legal empty string.
inline std::string16 UTF8ToString16(const char *in) {
  std::string16 out16;
  return UTF8ToString16(in, &out16) ? out16 : std::string16();
}

// Returns empty string on error. Not distinguished from legal empty string.
inline std::string16 UTF8ToString16(const std::string &in) {
  std::string16 out16;
  return UTF8ToString16(in, &out16) ? out16 : std::string16();
}

bool String16ToUTF8(const char16 *in, int len, std::string *out8);

inline bool String16ToUTF8(const char16 *in, std::string *out8) {
  assert(in);
  return String16ToUTF8(in, std::char_traits<char16>::length(in), out8);
}

inline bool String16ToUTF8(const std::string16 &in, std::string *out8) {
  return String16ToUTF8(in.data(), in.size(), out8);
}

// Returns empty string on error. Not distinguished from legal empty string.
inline std::string String16ToUTF8(const char16 *in) {
  std::string out8;
  return String16ToUTF8(in, &out8) ? out8 : std::string();
}

// Returns empty string on error. Not distinguished from legal empty string.
inline std::string String16ToUTF8(const std::string16 &in) {
  std::string out8;
  return String16ToUTF8(in, &out8) ? out8 : std::string();
}

// ----------------------------------------------------------------------
// Replaces all occurences of old_pattern found in str with new_pattern
// and returns the number of replacements that were made. All arguments
// must be of the same type of basic_string derived class.
// ----------------------------------------------------------------------
template<class StringT>
inline int ReplaceAll(StringT &str,
                      const StringT &old_pattern,
                      const StringT &new_pattern) {
  int replacements_made = 0;
  size_t index = StringT::npos;
  size_t offset = 0;
  while ((index = str.find(old_pattern, offset)) != StringT::npos) {
    str.replace(index, old_pattern.length(), new_pattern);
    offset = index + new_pattern.length();
    ++replacements_made;
  }
  return replacements_made;
}

// Does "left" start with "right"
template<class StringT>
inline bool StartsWith(const StringT &left, const StringT &right) {
  size_t right_len = right.length();
  return (left.length() >= right_len) &&
         memstr(left.c_str(), right_len, right.c_str(), right_len);
}

// Does "left" end with "right"
template<class StringT>
inline bool EndsWith(const StringT &left, const StringT &right) {
  size_t left_len = left.length();
  size_t right_len = right.length();
  return (left_len >= right_len) &&
         memstr(left.c_str() + (left_len - right_len), right.c_str());
}

// Split a string into its fields delimited by any of the charactes
// in "delimiters". Each field is added to the "tokens" vector.
// Returns the number of tokens found.
template <class StringT>
inline int Tokenize(const StringT &str,
                    const StringT &delimiters,
                    std::vector<StringT> *tokens) {
  tokens->clear();
  typename StringT::size_type start = str.find_first_not_of(delimiters);
  while (start != StringT::npos) {
    typename StringT::size_type end = str.find_first_of(delimiters, start + 1);
    if (end == StringT::npos) {
      tokens->push_back(str.substr(start));
      break;
    } else {
      tokens->push_back(str.substr(start, end - start));
      start = str.find_first_not_of(delimiters, end + 1);
    }
  }
  return tokens->size();
}


// Performs a case-insensitive comparison of char* or char16* strings.
template<class CharT>
inline int StringCompareIgnoreCase(const CharT *lhs, const CharT *rhs) {
  assert(lhs && rhs);
  int ret = 0;
  while (!(ret = std::tolower(*lhs) - std::tolower(*rhs)) && *rhs) {
    ++lhs;
    ++rhs;
  }
  if (ret < 0)
    ret = -1 ;
  else if (ret > 0)
    ret = 1 ;
  return ret;
}

// Determines whether the simple wildcard pattern matches target.
// Alpha characters in pattern match case-insensitively.
// Asterisks in pattern match 0 or more characters.
// Ex: StringMatch("www.TEST.GOOGLE.COM", "www.*.com") -> true
template<class CharT>
bool StringMatch(const CharT* target, const CharT* pattern) {
  while (*pattern) {
    if (*pattern == '*') {
      if (!*++pattern) {
        return true;
      }
      while (*target) {
        if ((std::toupper(*pattern) == std::toupper(*target))
            && StringMatch(target + 1, pattern + 1)) {
          return true;
        }
        ++target;
      }
      return false;
    } else {
      if (std::toupper(*pattern) != std::toupper(*target)) {
        return false;
      }
      ++target;
      ++pattern;
    }
  }
  return !*target;
}

// TODO: consider merging the functions to parse string to integer or changing
// all the callers of ParseLeadingInteger to use the more robust funciton.

// This function parses the given string up to the first non-integer character.
// If endptr is non-NULL, *endptr will be made to point at this character.
//
// This function is intentionally very simple.  In particular:
// - it only supports base 10
// - it does not handle exotic chars (negation, whitespace, exponentiation)
// - the return value is undefined for integer overflow
//
// On failure, *endptr == str, and the return value is undefined.
int ParseLeadingInteger(const char16 *str, const char16 **endptr);
int ParseLeadingInteger(const char *str, const char **endptr);

// Converts a string to a decimal integer. Returns false if the conversion can
// not be performed.
bool StringToInt(const char *str, int *value);
bool String16ToInt(const char16 *str, int *value);

#ifdef ANDROID
// Wide character version of strlen, not implemented on Android.
size_t wcslen(const char16 *str);

// std::wstring not defined on Android.
namespace std {
  typedef basic_string<char16> wstring;
}
#endif

#endif  // GEARS_BASE_COMMON_STRING_UTILS_H__
