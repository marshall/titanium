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

#ifdef USING_CCTESTS

#include <string>
#include "gears/base/common/common.h"
#include "gears/base/common/string_utils.h"

// We don't use google3/testing/base/gunit because our tests depend of
// browser specific code that needs to run in the context of the browser.

static bool TestStartsWithAndEndsWith();
static bool TestStrUtilsReplaceAll();
static bool TestStringCompareIgnoreCase();
static bool TestStringMatch();
static bool TestUTFConversion();
static bool TestStringToInteger();

bool TestStringUtils(std::string16 *error) {
  bool ok = true;
  ok &= TestStringCompareIgnoreCase();
  ok &= TestStartsWithAndEndsWith();
  ok &= TestStrUtilsReplaceAll();
  ok &= TestStringMatch();
  ok &= TestUTFConversion();
  ok &= TestStringToInteger();
  if (!ok) {
    assert(error); \
    *error += STRING16(L"TestStringUtils - failed. "); \
  }
  return ok;
}


#if WIN32
#define strcasecmp _stricmp
#endif

static bool TestStringCompareIgnoreCase() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestStringCompareIgnoreCase - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}
  {
    std::string empty_str;
    std::string aaa_str("aaa");
    std::string bbb_str("bbb");
    std::string aaaa_str("aaaa");
    std::string aaa_upper_str = MakeUpperString(aaa_str);

    // First test our assumptions about the proper results by invoking
    // the CRT's impl for 8 bit characters
    TEST_ASSERT(strcasecmp(aaa_str.c_str(), aaa_str.c_str()) == 0);
    TEST_ASSERT(strcasecmp(aaa_str.c_str(), bbb_str.c_str()) < 0);
    TEST_ASSERT(strcasecmp(bbb_str.c_str(), aaa_str.c_str()) > 0);
    TEST_ASSERT(strcasecmp(aaa_str.c_str(), aaaa_str.c_str()) < 0);
    TEST_ASSERT(strcasecmp(aaa_str.c_str(), empty_str.c_str()) > 0);
    TEST_ASSERT(strcasecmp(aaa_str.c_str(),
                           aaa_upper_str.c_str()) == 0);

    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), aaa_str.c_str()) == 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), bbb_str.c_str()) < 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    bbb_str.c_str(), aaa_str.c_str()) > 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), aaaa_str.c_str()) < 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), empty_str.c_str()) > 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), aaa_upper_str.c_str()) == 0);
  }
  {
    std::string16 empty_str;
    std::string16 aaa_str(STRING16(L"aaa"));
    std::string16 bbb_str(STRING16(L"bbb"));
    std::string16 aaaa_str(STRING16(L"aaaa"));
    std::string16 aaa_upper_str = MakeUpperString(aaa_str);

    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), aaa_str.c_str()) == 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), bbb_str.c_str()) < 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    bbb_str.c_str(), aaa_str.c_str()) > 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), aaaa_str.c_str()) < 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), empty_str.c_str()) > 0);
    TEST_ASSERT(StringCompareIgnoreCase(
                    aaa_str.c_str(), aaa_upper_str.c_str()) == 0);
  }
  return true;
}


static bool TestStrUtilsReplaceAll() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestStrUtilsReplaceAll - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}
  std::string str("bbaaabbaaabb");
  TEST_ASSERT(ReplaceAll(str, std::string("bb"), std::string("cc")) == 3);
  TEST_ASSERT(str == "ccaaaccaaacc");

  TEST_ASSERT(ReplaceAll(str, std::string("cc"), std::string("d")) == 3);
  TEST_ASSERT(str == "daaadaaad");

  TEST_ASSERT(ReplaceAll(str, std::string("d"), std::string("ff")) == 3);
  TEST_ASSERT(str == "ffaaaffaaaff");

  TEST_ASSERT(ReplaceAll(str, std::string("ff"), std::string(1, 0)) == 3);
  TEST_ASSERT(str.length() == 9);

  TEST_ASSERT(ReplaceAll(str, std::string(1, 0), std::string("bb")) == 3);
  TEST_ASSERT(str == "bbaaabbaaabb");

  str = "aaaa";
  TEST_ASSERT(ReplaceAll(str, std::string("a"), std::string("aa")) == 4);
  TEST_ASSERT(str == "aaaaaaaa");

  TEST_ASSERT(ReplaceAll(str, std::string("aa"), std::string("a")) == 4);
  TEST_ASSERT(str == "aaaa");

  TEST_ASSERT(ReplaceAll(str, std::string("b"), std::string("c")) == 0);
  TEST_ASSERT(str == "aaaa");

  return true;
}

static bool TestStartsWithAndEndsWith() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestBeginsWithAndEndsWith - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}
  // std::string16 tests
  {
    const std::string16 prefix(STRING16(L"prefix"));
    const std::string16 suffix(STRING16(L"suffix"));
    const std::string16 test(STRING16(L"prefix_string_suffix"));
    TEST_ASSERT(StartsWith(test, prefix));
    TEST_ASSERT(EndsWith(test, suffix));
    TEST_ASSERT(!StartsWith(test, suffix));
    TEST_ASSERT(!EndsWith(test, prefix));
    TEST_ASSERT(!StartsWith(prefix, test));
    TEST_ASSERT(!EndsWith(suffix, test));
  }
  // std::string tests
  {
    const std::string prefix("prefix");
    const std::string suffix("suffix");
    const std::string test("prefix_string_suffix");
    TEST_ASSERT(StartsWith(test, prefix));
    TEST_ASSERT(EndsWith(test, suffix));
    TEST_ASSERT(!StartsWith(test, suffix));
    TEST_ASSERT(!EndsWith(test, prefix));
    TEST_ASSERT(!StartsWith(prefix, test));
    TEST_ASSERT(!EndsWith(suffix, test));
  }
  LOG(("TestStartsWithAndEndsWith - passed\n"));
  return true;
}

static bool TestStringMatch() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestStringMatch - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}

  // 8 bit characters
  TEST_ASSERT(StringMatch("A.B.C.D", "a.b.c.d"));
  TEST_ASSERT(StringMatch("www.TEST.GOOGLE.COM", "www.*.com"));
  TEST_ASSERT(StringMatch("127.0.0.1", "12*.0.*1"));
  TEST_ASSERT(StringMatch("127.1.0.21", "12*.0.*1"));
  TEST_ASSERT(!StringMatch("127.0.0.0", "12*.0.*1"));
  TEST_ASSERT(!StringMatch("127.0.0.0", "12*.0.*1"));
  TEST_ASSERT(!StringMatch("127.1.1.21", "12*.0.*1"));

  // 16 bit characters
  TEST_ASSERT(StringMatch(L"A.B.C.D", L"a.b.c.d"));
  TEST_ASSERT(StringMatch(L"www.TEST.GOOGLE.COM", L"www.*.com"));
  TEST_ASSERT(StringMatch(L"127.0.0.1", L"12*.0.*1"));
  TEST_ASSERT(StringMatch(L"127.1.0.21", L"12*.0.*1"));
  TEST_ASSERT(!StringMatch(L"127.0.0.0", L"12*.0.*1"));
  TEST_ASSERT(!StringMatch(L"127.0.0.0", L"12*.0.*1"));
  TEST_ASSERT(!StringMatch(L"127.1.1.21", L"12*.0.*1"));

  return true;
}

static bool TestUTFConversion() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestUTFConversion - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}
  // string16 -> utf8 tests
  {
    // 1 byte utf8
    const std::string16 sample16(STRING16(L"Sample of a string"));
    const std::string expected8("Sample of a string");
    std::string result8;
    TEST_ASSERT(String16ToUTF8(sample16.c_str(), &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16, &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16) == expected8);
  }
  {
    // 2 byte utf8
    const std::string16 sample16(STRING16(L"Sample of a string \x0430"));
    const std::string expected8("Sample of a string \xD0\xB0");
    std::string result8;
    TEST_ASSERT(String16ToUTF8(sample16.c_str(), &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16, &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16) == expected8);
  }
  {
    // 3 byte utf8
    const std::string16 sample16(STRING16(L"\x4E8C"));
    const std::string expected8("\xE4\xBA\x8C");
    std::string result8;
    TEST_ASSERT(String16ToUTF8(sample16.c_str(), &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16, &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16) == expected8);
  }
  {
    // 4 byte utf8
    const std::string16 sample16(STRING16(L"\xD800\xDF02"));
    const std::string expected8("\xF0\x90\x8C\x82");
    std::string result8;
    TEST_ASSERT(String16ToUTF8(sample16.c_str(), &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16, &result8));
    TEST_ASSERT(expected8 == result8);
    TEST_ASSERT(String16ToUTF8(sample16) == expected8);
  }
  {
    // Illegal UTF-16 string
    const std::string16 sample16(STRING16(L"\xD800\xD800"));
    const std::string expected8;
    std::string result8;
    TEST_ASSERT(!String16ToUTF8(sample16.c_str(), &result8));
    TEST_ASSERT(!String16ToUTF8(sample16, &result8));
    TEST_ASSERT(String16ToUTF8(sample16) == expected8);
  }
  // utf8 -> string16 tests
  {
    // 1 byte utf8
    const std::string sample8("Sample of a string");
    const std::string16 expected16(STRING16(L"Sample of a string"));
    std::string16 result16;
    TEST_ASSERT(UTF8ToString16(sample8.c_str(), &result16));
    TEST_ASSERT(expected16 == result16);
  }
  {
    // 2 byte utf8
    const std::string sample8("Sample of a string \xD0\xB0");
    const std::string16 expected16(STRING16(L"Sample of a string \x0430"));
    std::string16 result16;
    TEST_ASSERT(UTF8ToString16(sample8.c_str(), &result16));
    TEST_ASSERT(expected16 == result16);
  }
  {
    // 3 byte utf8
    const std::string sample8("\xE4\xBA\x8C");
    const std::string16 expected16(STRING16(L"\x4E8C"));
    std::string16 result16;
    TEST_ASSERT(UTF8ToString16(sample8.c_str(), &result16));
    TEST_ASSERT(expected16 == result16);
  }
  {
    // 4 byte utf8
    const std::string sample8("\xF0\x90\x8C\x82");
    const std::string16 expected16(STRING16(L"\xD800\xDF02"));
    std::string16 result16;
    TEST_ASSERT(UTF8ToString16(sample8.c_str(), &result16));
    TEST_ASSERT(expected16 == result16);
  }
  {
    // Illegal utf8 string
    const std::string sample8("\xFF");
    const std::string16 expected16;
    std::string16 result16;
    TEST_ASSERT(!UTF8ToString16(sample8.c_str(), &result16));
    TEST_ASSERT(!UTF8ToString16(sample8, &result16));
    TEST_ASSERT(UTF8ToString16(sample8) == expected16);
  }
  LOG(("TestUTFConversion - passed\n"));
  return true;
}

static bool TestStringToInteger() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestStringToInteger - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}
  int value = 0;

  TEST_ASSERT(StringToInt("0", &value));
  TEST_ASSERT(value == 0);
  TEST_ASSERT(StringToInt("123", &value));
  TEST_ASSERT(value == 123);
  TEST_ASSERT(StringToInt("+123", &value));
  TEST_ASSERT(value == 123);
  TEST_ASSERT(StringToInt("-123", &value));
  TEST_ASSERT(value == -123);
  TEST_ASSERT(StringToInt("2147483647", &value));
  TEST_ASSERT(value == 2147483647);
  TEST_ASSERT(StringToInt("002147483647", &value));
  TEST_ASSERT(value == 2147483647);
  TEST_ASSERT(StringToInt("-2147483648", &value));
  TEST_ASSERT(value == static_cast<int>(0x80000000)); // to avoid gcc warning

  TEST_ASSERT(!StringToInt("a", &value));
  TEST_ASSERT(!StringToInt("123a", &value));
  TEST_ASSERT(!StringToInt("12.3", &value));
  TEST_ASSERT(!StringToInt("12e3", &value));
  TEST_ASSERT(!StringToInt("+-123", &value));
  TEST_ASSERT(!StringToInt("2147483648", &value));
  TEST_ASSERT(!StringToInt("-2147483649", &value));

  TEST_ASSERT(String16ToInt(STRING16(L"0"), &value));
  TEST_ASSERT(value == 0);
  TEST_ASSERT(String16ToInt(STRING16(L"123"), &value));
  TEST_ASSERT(value == 123);
  TEST_ASSERT(String16ToInt(STRING16(L"+123"), &value));
  TEST_ASSERT(value == 123);
  TEST_ASSERT(String16ToInt(STRING16(L"-123"), &value));
  TEST_ASSERT(value == -123);
  TEST_ASSERT(String16ToInt(STRING16(L"2147483647"), &value));
  TEST_ASSERT(value == 2147483647);
  TEST_ASSERT(String16ToInt(STRING16(L"002147483647"), &value));
  TEST_ASSERT(value == 2147483647);
  TEST_ASSERT(String16ToInt(STRING16(L"-2147483648"), &value));
  TEST_ASSERT(value == static_cast<int>(0x80000000)); // to avoid gcc warning

  TEST_ASSERT(!String16ToInt(STRING16(L"a"), &value));
  TEST_ASSERT(!String16ToInt(STRING16(L"123a"), &value));
  TEST_ASSERT(!String16ToInt(STRING16(L"12.3"), &value));
  TEST_ASSERT(!String16ToInt(STRING16(L"12e3"), &value));
  TEST_ASSERT(!String16ToInt(STRING16(L"+-123"), &value));
  TEST_ASSERT(!String16ToInt(STRING16(L"2147483648"), &value));
  TEST_ASSERT(!String16ToInt(STRING16(L"-2147483649"), &value));

  LOG(("TestStringToInteger - passed\n"));
  return true;
}
#endif  // USING_CCTESTS
