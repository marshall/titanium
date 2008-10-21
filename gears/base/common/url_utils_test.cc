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

#include "gears/base/common/url_utils.h"

#include "gears/base/common/common.h"  // only for LOG()
#include "gears/base/common/string_utils.h"

static bool TestUrlUTF8FileToUrl();
static bool TestUrlResolve();

bool TestUrlUtils(std::string16 *error) {
  bool ok = true;
  ok &= TestUrlResolve();
  ok &= TestUrlUTF8FileToUrl();
  if (ok) {
    LOG(("TestUrlUtilsAll - passed\n"));
  } else {
    assert(error); \
    *error += STRING16(L"TestUrlUtilsAll - failed. "); \
  }
  return ok;
}

static bool TestUrlResolve() {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestUrlResolve - failed (%d)\n", __LINE__)); \
    return false; \
  } \
}

  // URLs that begin with a possibly-valid scheme should be non-relative.

  TEST_ASSERT(!IsRelativeUrl(STRING16(L"http:")));
  TEST_ASSERT(!IsRelativeUrl(STRING16(L"HTTP:")));
  TEST_ASSERT(!IsRelativeUrl(STRING16(L"hTTp:")));

  TEST_ASSERT(!IsRelativeUrl(STRING16(L"funkyABCDEFGHIJKLMNOPQRSTUVWXYZ:")));
  TEST_ASSERT(!IsRelativeUrl(STRING16(L"funkyabcdefghijklmnopqrstuvwxyz:")));
  TEST_ASSERT(!IsRelativeUrl(STRING16(L"funky0123456789:")));
  TEST_ASSERT(!IsRelativeUrl(STRING16(L"funky+.-:")));

  TEST_ASSERT(!IsRelativeUrl(STRING16(L"http://www.example.com/foo.txt")));


  // All other URLs should be relative.

  TEST_ASSERT(IsRelativeUrl(STRING16(L"foo.txt")));
  TEST_ASSERT(IsRelativeUrl(STRING16(L"foo.txt?http://evil.com")));
  TEST_ASSERT(IsRelativeUrl(STRING16(L"/foo.txt")));

  TEST_ASSERT(IsRelativeUrl(STRING16(L"http")));  // no trailing colon
  TEST_ASSERT(IsRelativeUrl(STRING16(L"0http:")));  // no leading alpha
  TEST_ASSERT(IsRelativeUrl(STRING16(L":")));


  // ResolveAndNormalize test cases

  const char16 *base_page =
                   STRING16(L"http://server/directory/page");
  const char16 *base_page_with_hash =
                   STRING16(L"http://server/directory/page#bar");
  const char16 *base_page_with_query =
                   STRING16(L"http://server/directory/page?query");
  const char16 *base_dir =
                   STRING16(L"http://server/directory/");
  const char16 *base_dir_with_hash =
                   STRING16(L"http://server/directory/#bar");
  const char16 *base_dir_with_query =
                   STRING16(L"http://server/directory/?query");

  const struct {
      const char16 *base;
      const char16 *url;
      const char16 *resolved;
  } kCases[] = {
    {
      base_page,
      STRING16(L"foo"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_page_with_hash,
      STRING16(L"foo"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_page_with_query,
      STRING16(L"foo"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_dir,
      STRING16(L"foo"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_dir_with_hash,
      STRING16(L"foo"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_dir_with_query,
      STRING16(L"foo"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_page,
      STRING16(L"/foo"),
      STRING16(L"http://server/foo")
    },
    {
      base_page,
      STRING16(L"./foo"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_page,
      STRING16(L"../foo"),
      STRING16(L"http://server/foo")
    },
    {
      base_page,
      STRING16(L"foo#bar"),
      STRING16(L"http://server/directory/foo")
    },
    {
      base_page,
      STRING16(L"foo?bar"),
      STRING16(L"http://server/directory/foo?bar")
    },
    {
      NULL,
      STRING16(L"http://server/directory/foo#bar"),
      STRING16(L"http://server/directory/foo")
    },
#ifdef BROWSER_IE
    // TODO(playmobil): escape URLs on IE6.
    // Behavior for this case is inconsistent between IE6 and IE7.
    // IE6 doesn't escape the URL and return 'http://server/a b/c d'.
    // IE7 behaves the same as Firefox.
#else
    {  // Check that we escape base and url.
      STRING16(L"http://server/a b/"),
      STRING16(L"c d"),
      STRING16(L"http://server/a%20b/c%20d")
    },
    {  // Check partially escaped URLs
      STRING16(L"http://server/a b/c%20 d/"),
      STRING16(L"e%20 f"),
      STRING16(L"http://server/a%20b/c%20%20d/e%20%20f")
    },
    {  // Escape correctly if base not specified
      NULL,
      STRING16(L"http://server/a b/c%20 d/"),
      STRING16(L"http://server/a%20b/c%20%20d/")
    },
#endif

#ifdef BROWSER_IE
    // TODO(playmobil): make IE7 Resolve&Normalize handle user:pass in URL.
    // Behavior for this case is inconsistent between IE6 and IE7.
    // IE6 handles http://user:pass@domain without issues.
    // IE7 fails on such URLs.
#else
    {  // Lowercase scheme & host but don't touch path.
      STRING16(L"HTTp://fOo:bAbA@SErver:8080/"),
      STRING16(L"fOo.hTml?a=bB"),
      STRING16(L"http://fOo:bAbA@server:8080/fOo.hTml?a=bB")
     },
#endif
    {  // Lowercase scheme & host but don't touch path.
      STRING16(L"HTTp://SErver:8080/"),
      STRING16(L"fOo.hTml?a=bB"),
      STRING16(L"http://server:8080/fOo.hTml?a=bB")
     },
    {  // Add trailing slash to domain.
      STRING16(L"http://server"),
      STRING16(L""),
      STRING16(L"http://server/")
    },
    {  // Make sure we don't try to remove directory index.
      base_dir,
      STRING16(L"index.html"),
      STRING16(L"http://server/directory/index.html")
    },
    {  // Preserve capitalization of escape sequences.
       // Some normalizers will output: http://server/directory/a%C2%B1b
      base_dir,
      STRING16(L"a%c2%B1b"),
      STRING16(L"http://server/directory/a%c2%B1b")
    },
    {  // Remove default port for http.
      STRING16(L"http://server:80/directory/"),
      STRING16(L""),
      STRING16(L"http://server/directory/")
    },
    {  // Remove default port for https.
      STRING16(L"https://server:443/directory/"),
      STRING16(L""),
      STRING16(L"https://server/directory/")
    },
    {  // Collapse URL Fragments.
      STRING16(L"http://server/.././../a/b/../c/"),
      STRING16(L"./d.html"),
      STRING16(L"http://server/a/c/d.html")
    },
    {  // Leave www intact.
       // Some normalizers will output: http://foo.com/
      STRING16(L"http://www.foo.com/"),
      STRING16(L""),
      STRING16(L"http://www.foo.com/")
    },
    {  // Don't sort page variables.
       // Some normalizers will output: http://server/directory/foo.html?a=b&b=c
      base_dir,
      STRING16(L"foo.html?b=c&a=b"),
      STRING16(L"http://server/directory/foo.html?b=c&a=b")
    },
    {  // Remove empty query string.
       //  Some normalizers will remove the trailing '?'
      base_dir,
      STRING16(L"foo.html?"),
      STRING16(L"http://server/directory/foo.html?")
    },
    { // It's illegal to unsescape %2F -> '/' during normalization.
      // Thanks to brettw for confirming that this is the desired behavior. 
      base_dir,
      STRING16(L"..%2Fbar%2Ffoo.html?%2F"),
      STRING16(L"http://server/directory/..%2Fbar%2Ffoo.html?%2F")
    },

  };

  for (size_t i = 0; i < ARRAYSIZE(kCases); ++i) {
    std::string16 resolved;
    TEST_ASSERT(ResolveAndNormalize(kCases[i].base, kCases[i].url, &resolved));
    if (resolved != kCases[i].resolved) {
      std::string expected_utf8;
      String16ToUTF8(kCases[i].resolved, &expected_utf8);
      std::string resolved_utf8;
      String16ToUTF8(resolved.c_str(), &resolved_utf8);
      LOG(("URL Resolution failed, expected(%s) got (%s)\n", 
            expected_utf8.c_str(), resolved_utf8.c_str()));
      return false;
    }
  }

  LOG(("TestUrlResolve - passed\n"));
  return true;
}

static bool TestUrlUTF8FileToUrl() {
#undef TEST_ASSERT
#define TEST_ASSERT(b,test_name) \
{ \
  if (!(b)) { \
  LOG(("TestUrlUTF8FileToUrl: %s - failed (%d)\n", test_name, __LINE__)); \
    return false; \
  } \
}
  struct URLCase {
    const char *input;
    const char *expected;
    const char *test_name;
  } cases[] = {
    {"c:/Dead/Beef.txt", "file:///c:/Dead/Beef.txt", "No escapes"},
    {"c:\\Dead\\Beef.txt", "file:///c:/Dead/Beef.txt", "Backslash"},
    {"c:/Dead/Beef/42;.txt", "file:///c:/Dead/Beef/42%3B.txt", "Semicolon"},
    {"c:/Dead/Beef/42#{}.txt", "file:///c:/Dead/Beef/42%23%7B%7D.txt",
      "Disallowed Characters"},
    {"c:/Dead/Beef/牛肉.txt",
      "file:///c:/Dead/Beef/%E7%89%9B%E8%82%89.txt",
      "Non-Ascii Characters"}
  };

  struct URLCase directory_cases[] = {
    {"c:/Dead/Beef/", "file:///c:/Dead/Beef/", "Trailing slash"},
    {"c:\\Dead\\Beef\\", "file:///c:/Dead/Beef/", "Trailing backslash"},
    {"c:/Dead/Beef", "file:///c:/Dead/Beef/", "No trailing slash"},
    {"c:\\Dead\\Beef", "file:///c:/Dead/Beef/", "No trailing backslash"},
  };

  for (unsigned int i = 0; i < ARRAYSIZE(cases); ++i) {
    std::string input(cases[i].input);
    std::string output(UTF8PathToUrl(input, false));
    TEST_ASSERT(output == cases[i].expected, cases[i].test_name);
  }

  for (unsigned int i = 0; i < ARRAYSIZE(directory_cases); ++i) {
    std::string input(directory_cases[i].input);
    std::string output(UTF8PathToUrl(input, true));
    TEST_ASSERT(output == directory_cases[i].expected,
                directory_cases[i].test_name);
  }

  LOG(("TestUrlUTF8FileToUrl - passed\n"));
  return true;
}
