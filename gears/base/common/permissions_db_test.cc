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

#include <algorithm>
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/thread_locals.h"

// For use with sort().
static bool SecurityOriginLT(const SecurityOrigin &a,
                             const SecurityOrigin &b) {
  return a.url() < b.url();
}

// Return true if permissions has the set of origins in expected, else
// false.
static bool VerifyOrigins(PermissionsDB *permissions,
                          const std::vector<SecurityOrigin> &expected) {
  std::vector<SecurityOrigin> origins;
  if (!permissions->GetOriginsWithShortcuts(&origins)) {
    return false;
  }

  if (origins.size() != expected.size()) {
    return false;
  }

  std::vector<SecurityOrigin> sorted_expected(expected.begin(),
                                              expected.end());
  std::sort(sorted_expected.begin(), sorted_expected.end(), SecurityOriginLT);
  std::sort(origins.begin(), origins.end(), SecurityOriginLT);

  for (size_t ii = 0; ii < origins.size(); ++ii) {
    if (origins[ii].url() != sorted_expected[ii].url()) {
      return false;
    }
  }
  return true;
}

bool TestPermissionsDBAll(std::string16 *error) {
// TODO(aa): Refactor into a common location for all the internal tests.
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestPermissionsDBAll - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestPermissionsDBAll - failed. "); \
    return false; \
  } \
}

  PermissionsDB *permissions = PermissionsDB::GetDB();
  TEST_ASSERT(permissions);

  // Set some permissions
  SecurityOrigin foo;
  foo.InitFromUrl(STRING16(L"http://unittest.foo.example.com"));
  permissions->SetPermission(foo,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);

  SecurityOrigin bar;
  bar.InitFromUrl(STRING16(L"http://unittest.bar.example.com"));
  permissions->SetPermission(bar,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_DENIED);

  // Get the threadlocal instance and make sure we got the same instance back
  TEST_ASSERT(PermissionsDB::GetDB() == permissions);

  // Now destory the threadlocal instance and get a new one to test whether our
  // values were saved.
  ThreadLocals::DestroyValue(PermissionsDB::kThreadLocalKey);
  permissions = PermissionsDB::GetDB();

  TEST_ASSERT(permissions->GetPermission(
      foo, PermissionsDB::PERMISSION_LOCAL_DATA) ==
      PermissionsDB::PERMISSION_ALLOWED);
  TEST_ASSERT(permissions->GetPermission(
      bar, PermissionsDB::PERMISSION_LOCAL_DATA) ==
      PermissionsDB::PERMISSION_DENIED);

  // Try searching for PERMISSION_NOT_SET (should not be allowed).
  std::vector<SecurityOrigin> list;
  TEST_ASSERT(!permissions->GetOriginsByValue(
      PermissionsDB::PERMISSION_NOT_SET,
      PermissionsDB::PERMISSION_LOCAL_DATA,
      &list));

  // Now try resetting
  permissions->SetPermission(bar,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_NOT_SET);
  TEST_ASSERT(permissions->GetPermission(
      bar, PermissionsDB::PERMISSION_LOCAL_DATA) ==
      PermissionsDB::PERMISSION_NOT_SET);

  // Test GetPermissionsByOrigin().
  // We already have foo in the permission table with LOCAL_DATA allowed.
  // We add bar back with LOCAL_DATA denied.
  // We then add totally_good (has both LOCAL_DATA and LOCATION_DATA allowed),
  // totally_evil (has has both LOCAL_DATA and LOCATION_DATA denied),
  // partially_evil (has LOCAL_DATA allowed and LOCATION_DATA denied),
  // location_good (has LOCATION_DATA allowed) and, finally,
  // location_evil (has LOCATION_DATA denied).
  permissions->SetPermission(bar,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_DENIED);

  SecurityOrigin totally_good;
  totally_good.InitFromUrl(STRING16(L"http://unittest.totallygood.com"));
  permissions->SetPermission(totally_good,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);
  permissions->SetPermission(totally_good,
                             PermissionsDB::PERMISSION_LOCATION_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);

  SecurityOrigin partially_bad;
  partially_bad.InitFromUrl(STRING16(L"http://unittest.partiallyevil.com"));
  permissions->SetPermission(partially_bad,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);
  permissions->SetPermission(partially_bad,
                             PermissionsDB::PERMISSION_LOCATION_DATA,
                             PermissionsDB::PERMISSION_DENIED);

  SecurityOrigin totally_bad;
  totally_bad.InitFromUrl(STRING16(L"http://unittest.totallybad.com"));
  permissions->SetPermission(totally_bad,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_DENIED);
  permissions->SetPermission(totally_bad,
                             PermissionsDB::PERMISSION_LOCATION_DATA,
                             PermissionsDB::PERMISSION_DENIED);

  SecurityOrigin location_good;
  location_good.InitFromUrl(STRING16(L"http://unittest.locationgood.com"));
  permissions->SetPermission(location_good,
                             PermissionsDB::PERMISSION_LOCATION_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);

  SecurityOrigin location_bad;
  location_bad.InitFromUrl(STRING16(L"http://unittest.locationbad.com"));
  permissions->SetPermission(location_bad,
                             PermissionsDB::PERMISSION_LOCATION_DATA,
                             PermissionsDB::PERMISSION_DENIED);

  PermissionsDB::PermissionsList permissions_list;
  TEST_ASSERT(permissions->GetPermissionsSorted(&permissions_list));
  // The permissions vector should then contain our test entries in the
  // following order (note that there may be other origins as well) :
  // foo,           (<LOCAL_DATA -> PERMISSION_ALLOWED>)
  // location_good, (<LOCATION_DATA -> PERMISSION_ALLOWED>)
  // partially_evil,(<LOCAL_DATA -> PERMISSION_ALLOWED>,
  //                 <LOCATION_DATA -> PERMISSION_DENIED>)
  // totally_good,  (<LOCAL_DATA -> PERMISSION_ALLOWED>,
  //                 <LOCATION_DATA -> PERMISSION_ALLOWED>)
  // bar,           (<LOCAL_DATA -> PERMISSION_DENIED>)
  // location_bad,  (<LOCATION_DATA -> PERMISSION_DENIED>)
  // totally_bad,   (<LOCAL_DATA -> PERMISSION_DENIED>,
  //                 <LOCATION_DATA-> PERMISSION_DENIED>)

  SecurityOrigin *next_expected_origin = &foo;
  int count = 0;
  for (int i = 0; i < static_cast<int>(permissions_list.size()); ++i) {
    if (permissions_list[i].first == foo.full_url()) {

      TEST_ASSERT(next_expected_origin->full_url() == foo.full_url());
      TEST_ASSERT(permissions_list[i].second.size() == 1);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCAL_DATA] ==
              PermissionsDB::PERMISSION_ALLOWED);
      next_expected_origin = &location_good;
      count++;

    } else if (permissions_list[i].first == location_good.full_url()) {

      TEST_ASSERT(next_expected_origin->full_url()  ==
          location_good.full_url());
      TEST_ASSERT(permissions_list[i].second.size() == 1);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCATION_DATA] ==
              PermissionsDB::PERMISSION_ALLOWED);
      next_expected_origin = &partially_bad;
      count++;

    } else if (permissions_list[i].first == partially_bad.full_url()) {

      TEST_ASSERT(next_expected_origin->full_url()  ==
          partially_bad.full_url());
      TEST_ASSERT(permissions_list[i].second.size() == 2);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCAL_DATA] ==
              PermissionsDB::PERMISSION_ALLOWED);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCATION_DATA] ==
              PermissionsDB::PERMISSION_DENIED);
      next_expected_origin = &totally_good;
      count++;

    } else if (permissions_list[i].first == totally_good.full_url()) {

      TEST_ASSERT(next_expected_origin->full_url()  == totally_good.full_url());
      TEST_ASSERT(permissions_list[i].second.size() == 2);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCAL_DATA] ==
              PermissionsDB::PERMISSION_ALLOWED);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCATION_DATA] ==
              PermissionsDB::PERMISSION_ALLOWED);
      next_expected_origin = &bar;
      count++;

    } else if (permissions_list[i].first == bar.full_url()) {

      TEST_ASSERT(next_expected_origin->full_url() == bar.full_url());
      TEST_ASSERT(permissions_list[i].second.size() == 1);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCAL_DATA] ==
              PermissionsDB::PERMISSION_DENIED);
      next_expected_origin = &location_bad;
      count++;

    } else if (permissions_list[i].first == location_bad.full_url()) {
    
      TEST_ASSERT(next_expected_origin->full_url() == location_bad.full_url());
      TEST_ASSERT(permissions_list[i].second.size() == 1);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCATION_DATA] ==
              PermissionsDB::PERMISSION_DENIED);
      next_expected_origin = &totally_bad;
      count++;

    } else if (permissions_list[i].first == totally_bad.full_url()) {

      TEST_ASSERT(next_expected_origin->full_url() == totally_bad.full_url());
      TEST_ASSERT(permissions_list[i].second.size() == 2);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCAL_DATA] ==
              PermissionsDB::PERMISSION_DENIED);
      TEST_ASSERT(
          permissions_list[i].second[PermissionsDB::PERMISSION_LOCATION_DATA] ==
              PermissionsDB::PERMISSION_DENIED);
      count++;

    }
  }

  TEST_ASSERT(count == 7);

  // TODO(shess) Constants for later comparison.
  const std::string16 kFooTest1(STRING16(L"Test"));
  const std::string16 kFooTest1Url(STRING16(L"http://www.foo.com/Test.html"));
  const std::string16
      kFooTest1IcoUrl(STRING16(L"http://www.foo.com/Test.ico"));
  const std::string16 kFooTest1Msg(STRING16(L"This is the message."));

  const std::string16 kFooTest2(STRING16(L"Another"));
  const std::string16
      kFooTest2Url(STRING16(L"http://www.foo.com/Another.html"));
  const std::string16
      kFooTest2IcoUrl0(STRING16(L"http://www.foo.com/Another.ico"));
  const std::string16
      kFooTest2IcoUrl1(STRING16(L"http://www.foo.com/YetAnother.ico"));
  const std::string16
      kFooTest2IcoUrl2(STRING16(L"http://www.foo.com/YetAnother2.ico"));
  const std::string16
      kFooTest2IcoUrl3(STRING16(L"http://www.foo.com/YetAnother3.ico"));
  const std::string16 kFooTest2Msg(STRING16(L"This is another message."));

  const std::string16 kBarTest(STRING16(L"Test"));
  const std::string16 kBarTestUrl(STRING16(L"http://www.bar.com/Test.html"));
  const std::string16 kBarTestIcoUrl(STRING16(L"http://www.bar.com/Test.ico"));
  const std::string16 kBarTestMsg(STRING16(L"This is a message."));

  // TODO(shess): It would be about 100x better if this could be
  // hermetic.  We could potentially develop a friendly relationship
  // with PermissionsDB, and put this entire routine into a
  // transaction, which would allow us to just blow everything away.

  // Get the complete set of origins with shortcuts, factor out those
  // which we're going to use to test, then verify that we still have
  // the right set of origins with shortcuts.
  std::vector<SecurityOrigin> other_origins;
  list.clear();
  TEST_ASSERT(permissions->GetOriginsWithShortcuts(&list));
  for (size_t ii = 0; ii < list.size(); ++ii) {
    if (list[ii].url() == foo.url()) {
      TEST_ASSERT(permissions->DeleteShortcuts(foo));
    } else if (list[ii].url() == bar.url()) {
      TEST_ASSERT(permissions->DeleteShortcuts(bar));
    } else {
      other_origins.push_back(list[ii]);
    }
  }
  TEST_ASSERT(VerifyOrigins(permissions, other_origins));

  // Load up some shortcuts.
  TEST_ASSERT(
      permissions->SetShortcut(foo, kFooTest1.c_str(), kFooTest1Url.c_str(),
                               kFooTest1IcoUrl.c_str(), STRING16(L""),
                               STRING16(L""), STRING16(L""),
                               kFooTest1Msg.c_str(), true));

  TEST_ASSERT(
      permissions->SetShortcut(foo, kFooTest2.c_str(), kFooTest2Url.c_str(),
                               kFooTest2IcoUrl0.c_str(),
                               kFooTest2IcoUrl1.c_str(),
                               kFooTest2IcoUrl2.c_str(),
                               kFooTest2IcoUrl3.c_str(),
                               kFooTest2Msg.c_str(), false));

  TEST_ASSERT(
      permissions->SetShortcut(bar, kBarTest.c_str(), kBarTestUrl.c_str(),
                               kBarTestIcoUrl.c_str(), STRING16(L""),
                               STRING16(L""), STRING16(L""),
                               kBarTestMsg.c_str(), false));

  // Expect 2 additional origins with shortcuts.
  std::vector<SecurityOrigin> all_origins(other_origins);
  all_origins.push_back(foo);
  all_origins.push_back(bar);
  TEST_ASSERT(VerifyOrigins(permissions, all_origins));

  // Expect 2 shortcuts for origin foo.
  std::vector<std::string16> names;
  TEST_ASSERT(permissions->GetOriginShortcuts(foo, &names));
  TEST_ASSERT(names.size() == 2);
  std::sort(names.begin(), names.end());
  TEST_ASSERT(names[0] == kFooTest2);
  TEST_ASSERT(names[1] == kFooTest1);

  // Test shortcut 2 to see if the data comes back right.
  {
    std::string16 app_url, msg;
    std::string16 icon_url0;
    std::string16 icon_url1;
    std::string16 icon_url2;
    std::string16 icon_url3;
    bool allow_shortcut;
    TEST_ASSERT(permissions->GetShortcut(foo, kFooTest2.c_str(),
                                         &app_url, &icon_url0, &icon_url1,
                                         &icon_url2, &icon_url3, &msg,
                                         &allow_shortcut));
    TEST_ASSERT(app_url == kFooTest2Url);
    TEST_ASSERT(msg == kFooTest2Msg);
    TEST_ASSERT(icon_url0 == kFooTest2IcoUrl0);
    TEST_ASSERT(icon_url1 == kFooTest2IcoUrl1);
    TEST_ASSERT(icon_url2 == kFooTest2IcoUrl2);
    TEST_ASSERT(icon_url3 == kFooTest2IcoUrl3);
    TEST_ASSERT(allow_shortcut == false);
  }

  // Test that value of true is correctly saved for allow_shortcut.
  {
    std::string16 app_url, msg;
    std::string16 icon_urls[4];
    bool allow_shortcut;
    TEST_ASSERT(permissions->GetShortcut(foo, kFooTest1.c_str(),
                                         &app_url, &icon_urls[0], &icon_urls[1],
                                         &icon_urls[2], &icon_urls[3], &msg,
                                         &allow_shortcut));
    TEST_ASSERT(allow_shortcut == true);
  }

  // Test that deleting a specific shortcut doesn't impact other
  // shortcuts for that origin.
  TEST_ASSERT(permissions->DeleteShortcut(foo, kFooTest2.c_str()));
  TEST_ASSERT(VerifyOrigins(permissions, all_origins));

  names.clear();
  TEST_ASSERT(permissions->GetOriginShortcuts(foo, &names));
  LOG(("names.size() == %d\n", names.size()));
  TEST_ASSERT(names.size() == 1);
  TEST_ASSERT(names[0] == kFooTest1);

  // Test that deleting a specific origin's shortcuts doesn't impact
  // other origins.
  TEST_ASSERT(permissions->DeleteShortcuts(foo));

  all_origins = other_origins;
  all_origins.push_back(bar);
  TEST_ASSERT(VerifyOrigins(permissions, all_origins));

  // Make sure we've cleaned up after ourselves.
  TEST_ASSERT(permissions->DeleteShortcuts(bar));
  TEST_ASSERT(VerifyOrigins(permissions, other_origins));

  // Test that we can get a database name, and that when we mark a
  // database corrupt we get a new name.
  const char16 *kFooDatabaseName = STRING16(L"corruption_test");
  std::string16 basename;
  TEST_ASSERT(permissions->GetDatabaseBasename(foo, kFooDatabaseName,
                                               &basename));

  TEST_ASSERT(permissions->MarkDatabaseCorrupt(foo, kFooDatabaseName,
                                               basename.c_str()));

  std::string16 new_basename;
  TEST_ASSERT(permissions->GetDatabaseBasename(foo, kFooDatabaseName,
                                               &new_basename));
  TEST_ASSERT(basename != new_basename);

  // TODO(shess): Consider whether to poke into permissions.db and
  // clear the database entries to prevent cruft from accumulating.

  LOG(("TestPermissionsDBAll - passed\n"));
  return true;
}
