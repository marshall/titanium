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

#ifdef USING_CCTESTS

#include <string>
#include "gears/base/common/common.h"
#include "gears/base/common/security_model.h"

// We don't use google3/testing/base/gunit because our tests depend of
// browser specific code that needs to run in the context of the browser.

#define ASSERT_TRUE(b) \
{ \
  if (!(b)) { \
    LOG(("TestSecurityModel - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestSecurityModel - failed. "); \
    return false; \
  } \
}

#define ASSERT_FALSE(b) ASSERT_TRUE(!(b))

bool TestSecurityModel(std::string16 *error) {
  SecurityOrigin origin;
  ASSERT_TRUE(origin.InitFromUrl(STRING16(L"http://www.google.com/")));
  ASSERT_TRUE(origin.scheme() == STRING16(L"http"));
  ASSERT_TRUE(origin.host() == STRING16(L"www.google.com"));
  ASSERT_TRUE(origin.port() == 80);

  ASSERT_TRUE(origin.InitFromUrl(STRING16(L"https://www.google.com/")));
  ASSERT_TRUE(origin.scheme() == STRING16(L"https"));
  ASSERT_TRUE(origin.host() == STRING16(L"www.google.com"));
  ASSERT_TRUE(origin.port() == 443);

  ASSERT_TRUE(origin.InitFromUrl(STRING16(L"file://whatever/")));
  ASSERT_TRUE(origin.scheme() == STRING16(L"file"));
  ASSERT_TRUE(origin.host() == kUnknownDomain);
  ASSERT_TRUE(origin.port() == 0);

  ASSERT_TRUE(origin.InitFromUrl(STRING16(L"http://www.google.com:99/")));
  ASSERT_TRUE(origin.scheme() == STRING16(L"http"));
  ASSERT_TRUE(origin.host() == STRING16(L"www.google.com"));
  ASSERT_TRUE(origin.port() == 99);

  ASSERT_TRUE(origin.InitFromUrl(STRING16(L"HTTP://www.GOOGLE.com/")));
  ASSERT_TRUE(origin.scheme() == STRING16(L"http"));
  ASSERT_TRUE(origin.host() == STRING16(L"www.google.com"));
  ASSERT_TRUE(origin.port() == 80);

  ASSERT_TRUE(origin.InitFromUrl(STRING16(L"HTTPS://www.GOOGLE.com/")));
  ASSERT_TRUE(origin.scheme() == STRING16(L"https"));
  ASSERT_TRUE(origin.host() == STRING16(L"www.google.com"));
  ASSERT_TRUE(origin.port() == 443);

  // Make sure we can crack the generated URL for local hosts
  const char16 *kLocalSecurityUrl = STRING16(L"file://_null_.localdomain");
  ASSERT_TRUE(origin.InitFromUrl(kLocalSecurityUrl));
  ASSERT_TRUE(origin.url() == kLocalSecurityUrl);

  // Explicitly disallow urls with userid:password
  ASSERT_FALSE(origin.InitFromUrl(
    STRING16(L"http://userid:password@www.google.com:33/")));

  ASSERT_FALSE(origin.InitFromUrl(STRING16(L"ftp://ftp.google.com/")));
  ASSERT_FALSE(origin.InitFromUrl(STRING16(L"blah")));
  ASSERT_FALSE(origin.InitFromUrl(STRING16(L"http://")));
  ASSERT_FALSE(origin.InitFromUrl(STRING16(L"")));

  SecurityOrigin origin2;
  const char16 *kScheme = STRING16(L"http");
  const char16 *kHost = STRING16(L"file");
  const char16 *kFullUrl = STRING16(L"http://dummy.url.not.used/");
  const int kPort = 1;
  // We use a value for kHost that is a supported scheme to avoid an
  // assert in IsDefaultPort about unsupported schemes. The private
  // Init method we use below calls thru to that function.

  origin.Init(kFullUrl, kScheme, kHost, kPort);
  origin2.Init(kFullUrl, kScheme, kHost, kPort);

  ASSERT_TRUE(origin.IsSameOrigin(origin2));

  origin2.Init(kFullUrl, kScheme, kHost, kPort + 1);
  ASSERT_FALSE(origin.IsSameOrigin(origin2));

  origin2.Init(kFullUrl, kHost, kHost, kPort);
  ASSERT_FALSE(origin.IsSameOrigin(origin2));

  origin2.Init(kFullUrl, kScheme, kScheme, kPort);
  ASSERT_FALSE(origin.IsSameOrigin(origin2));

  // Verify that copy works.
  SecurityOrigin copy;
  copy.CopyFrom(origin2);
  ASSERT_TRUE(copy.IsSameOrigin(origin2));

  copy.CopyFrom(origin);
  ASSERT_TRUE(copy.IsSameOrigin(origin));

  ASSERT_TRUE(origin2.InitFromUrl(STRING16(L"http://www.google.com:99/")));
  copy.CopyFrom(origin2);
  ASSERT_TRUE(origin2.InitFromUrl(STRING16(L"HTTPS://www.GOOGLE.com/")));
  ASSERT_TRUE(copy.full_url() == STRING16(L"http://www.google.com:99/"));
  ASSERT_TRUE(copy.url() == STRING16(L"http://www.google.com:99"));
  ASSERT_TRUE(copy.scheme() == STRING16(L"http"));
  ASSERT_TRUE(copy.host() == STRING16(L"www.google.com"));
  ASSERT_TRUE(copy.port() == 99);

  LOG(("TestSecurityModel - passed\n"));
  return true;
}

#endif  // USING_CCTESTS
