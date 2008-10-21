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

#include <stdio.h>

#include "gears/cctests/test.h"

#include "third_party/jsoncpp/json.h"

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/file.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/blob/blob.h"
#include "gears/blob/buffer_blob.h"
#include "gears/desktop/desktop_test.h"

#ifdef WIN32
// For some reason Win32 thinks snprintf() needs to be marked as non-standard.
#define snprintf _snprintf
#endif

DECLARE_GEARS_WRAPPER(GearsTest);

template<>
void Dispatcher<GearsTest>::Init() {
  RegisterMethod("runTests", &GearsTest::RunTests);
  RegisterMethod("testPassArguments", &GearsTest::TestPassArguments);
  RegisterMethod("testPassArgumentsCallback",
                 &GearsTest::TestPassArgumentsCallback);
  RegisterMethod("testPassArgumentsOptional",
                 &GearsTest::TestPassArgumentsOptional);
  RegisterMethod("testObjectProperties", &GearsTest::TestObjectProperties);
  RegisterMethod("testPassObject", &GearsTest::TestPassObject);
  RegisterMethod("testCreateObject", &GearsTest::TestCreateObject);
  RegisterMethod("testCreateError", &GearsTest::TestCreateError);
  RegisterMethod("testCoerceBool", &GearsTest::TestCoerceBool);
  RegisterMethod("testCoerceInt", &GearsTest::TestCoerceInt);
  RegisterMethod("testCoerceDouble", &GearsTest::TestCoerceDouble);
  RegisterMethod("testCoerceString", &GearsTest::TestCoerceString);
  RegisterMethod("testGetType", &GearsTest::TestGetType);
#ifdef WINCE
  RegisterMethod("removeEntriesFromBrowserCache",
                 &GearsTest::RemoveEntriesFromBrowserCache);
  RegisterMethod("testEntriesPresentInBrowserCache",
                 &GearsTest::TestEntriesPresentInBrowserCache);
#endif
  RegisterMethod("getSystemTime", &GearsTest::GetSystemTime);
  RegisterMethod("startPerfTimer", &GearsTest::StartPerfTimer);
  RegisterMethod("stopPerfTimer", &GearsTest::StopPerfTimer);
  RegisterMethod("testParseGeolocationOptions",
                 &GearsTest::TestParseGeolocationOptions);
  RegisterMethod("testGeolocationFormRequestBody",
                 &GearsTest::TestGeolocationFormRequestBody);
  RegisterMethod("testGeolocationGetLocationFromResponse",
                 &GearsTest::TestGeolocationGetLocationFromResponse);
  RegisterMethod("configureGeolocationRadioDataProviderForTest",
                 &GearsTest::ConfigureGeolocationRadioDataProviderForTest);
  RegisterMethod("configureGeolocationWifiDataProviderForTest",
                 &GearsTest::ConfigureGeolocationWifiDataProviderForTest);
  RegisterMethod("configureGeolocationMockLocationProviderForTest",
                 &GearsTest::ConfigureGeolocationMockLocationProviderForTest);
  RegisterMethod("removeGeolocationMockLocationProvider",
                 &GearsTest::RemoveGeolocationMockLocationProvider);
#ifdef OFFICIAL_BUILD
  // The Audio API has not been finalized for official builds.
#else
  RegisterMethod("configureAudioRecorderForTest",
                 &GearsTest::ConfigureAudioRecorderForTest);
#endif
  RegisterMethod("createBlobFromString", &GearsTest::CreateBlobFromString);
  RegisterMethod("testLocalServerPerformance",
                 &GearsTest::TestLocalServerPerformance);
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef OS_ANDROID
  // The notification API has not been implemented for Android.
#else
  RegisterMethod("testNotifier", &GearsTest::TestNotifier);
#endif  // OS_ANDROID
#endif  // OFFICIAL_BUILD
}

#ifdef WIN32
#include <windows.h>  // must manually #include before nsIEventQueueService.h
// the #define max on win32 conflicts with std::numeric_limits<T>::max()
#if defined(WIN32) && defined(max)
#undef max
#endif
#endif

#include <cmath>
#include <limits>
#include <sstream>
#ifdef LINUX
#include <unistd.h>  // For usleep.
#include <sys/wait.h>
#endif

#include "gears/base/common/name_value_table_test.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/permissions_db_test.h"
#include "gears/base/common/sqlite_wrapper_test.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/timed_call_test.h"
#ifdef WINCE
#include "gears/base/common/url_utils.h"
#include "gears/base/common/wince_compatibility.h"
#endif
#include "gears/database/database_utils_test.h"
#include "gears/geolocation/geolocation_db_test.h"
#include "gears/geolocation/geolocation_test.h"
#ifdef OFFICIAL_BUILD
// The Audio API has not been finalized for official builds.
#else
#include "gears/media/audio_recorder_test.h"
#endif
#include "gears/localserver/common/http_cookies.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/common/localserver_db.h"
#include "gears/localserver/common/managed_resource_store.h"
#include "gears/localserver/common/manifest.h"
#include "gears/localserver/common/resource_store.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

bool TestAllMutex(std::string16 *error);  // from mutex_test.cc
#if BROWSER_FF
// from blob_input_stream_ff_test.cc
bool TestBlobInputStreamFf(std::string16 *error);
#endif
#if BROWSER_WEBKIT
// from blob_input_stream_sf_test.cc
bool TestBlobInputStreamSf(std::string16 *error);
#endif
bool TestByteStore(std::string16 *error);  // from byte_store_test.cc
bool TestHttpCookies(BrowsingContext *context, std::string16 *error);
bool TestHttpRequest(BrowsingContext *context, std::string16 *error);
bool TestManifest(std::string16 *error);
bool TestMemoryBuffer(std::string16 *error);  // from memory_buffer_test.cc
bool TestMessageService(std::string16 *error);  // from message_service_test.cc
bool TestLocalServerDB(BrowsingContext *context, std::string16 *error);
bool TestResourceStore(std::string16 *error);
bool TestManagedResourceStore(std::string16 *error);
bool TestParseHttpStatusLine(std::string16 *error);
bool TestSecurityModel(std::string16 *error);  // from security_model_test.cc
bool TestFileUtils(std::string16 *error);  // from file_test.cc
bool TestUrlUtils(std::string16 *error);  // from url_utils_test.cc
bool TestStringUtils(std::string16 *error);  // from string_utils_test.cc
bool TestSerialization(std::string16 *error);  // from serialization_test.cc
bool TestCircularBuffer(std::string16 *error);  // from circular_buffer_test.cc
bool TestRefCount(std::string16 *error);  // from scoped_refptr_test.cc
bool TestBlob(std::string16 *error);  // from blob_test.cc
#if (defined(WIN32) && !defined(WINCE)) || \
    defined(LINUX) || defined(OS_MACOSX)
// from ipc_message_queue_test.cc
bool TestIpcSystemQueue(std::string16 *error);
bool TestIpcPeerQueue(std::string16 *error);
#endif
#ifdef OS_ANDROID
bool TestThreadMessageQueue(std::string16* error);
bool TestJavaClass(std::string16* error);
#endif
bool TestStopwatch(std::string16 *error);
bool TestJsonEscaping(std::string16 *error);
bool TestArray(JsRunnerInterface *js_runner, JsCallContext *context,
               std::string16 *error);
bool TestObject(JsRunnerInterface *js_runner, JsCallContext *context,
                std::string16 *error);
bool TestEvent(std::string16 *error);  // From event_test.cc

void CreateObjectBool(JsCallContext* context,
                      JsRunnerInterface* js_runner,
                      JsObject* out);
void CreateObjectInt(JsCallContext* context,
                        JsRunnerInterface* js_runner,
                        JsObject* out);
void CreateObjectDouble(JsCallContext* context,
                        JsRunnerInterface* js_runner,
                        JsObject* out);
void CreateObjectString(JsCallContext* context,
                        JsRunnerInterface* js_runner,
                        JsObject* out);
void CreateObjectArray(JsCallContext* context,
                       JsRunnerInterface* js_runner,
                       JsRootedCallback* func,
                       JsObject* out);
void CreateObjectObject(JsCallContext* context,
                        JsRunnerInterface* js_runner,
                        JsObject* out);
void CreateObjectDate(JsCallContext* context,
                      JsRunnerInterface* js_runner,
                      JsObject* out);
void CreateObjectFunction(JsCallContext* context,
                          JsRootedCallback* func,
                          JsObject* out);

void TestObjectBool(JsCallContext* context, const JsObject& obj);
void TestObjectInt(JsCallContext* context, const JsObject& obj);
void TestObjectDouble(JsCallContext* context, const JsObject& obj);
void TestObjectString(JsCallContext* context, const JsObject& obj);
void TestObjectArray(JsCallContext* context,
                     const JsObject& obj,
                     const ModuleImplBaseClass& base);
void TestObjectObject(JsCallContext* context, const JsObject& obj);
void TestObjectFunction(JsCallContext* context,
                        const JsObject& obj,
                        const ModuleImplBaseClass& base);

// from localserver_perf_test.cc
bool RunLocalServerPerfTests(int num_origins, int num_stores, int num_items,
                             std::string16 *results);

void GearsTest::TestLocalServerPerformance(JsCallContext *context) {
  int num_origins = 1;
  int num_stores = 10;
  int num_items = 100;
  JsArgument argv[] = {
    {JSPARAM_OPTIONAL, JSPARAM_INT, &num_origins},
    {JSPARAM_OPTIONAL, JSPARAM_INT, &num_stores},
    {JSPARAM_OPTIONAL, JSPARAM_INT, &num_items}
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }

  std::string16 results;
  RunLocalServerPerfTests(num_origins, num_stores, num_items, &results);
  context->SetReturnValue(JSPARAM_STRING16, &results);
}

// Return the system time as int64.
void GearsTest::GetSystemTime(JsCallContext *context) {
  int64 msec = GetCurrentTimeMillis();
  // SetReturnValue will fail and set an exception if msec exceeds JS_INT_MAX,
  // but this should not happen for 300,000 years!
  assert(msec > 0);
  context->SetReturnValue(JSPARAM_INT64, &msec);
}

// Start the perf timer.
void GearsTest::StartPerfTimer(JsCallContext *context) {
  if (start_ticks_ != 0) {
    context->SetException(STRING16(L"Perf timer is already running."));
    return;
  }
  start_ticks_ = GetTicks();
}

// Return the elapsed time in microseconds as int64.
void GearsTest::StopPerfTimer(JsCallContext *context) {
  if (start_ticks_ == 0) {
    context->SetException(STRING16(L"Perf timer has not been started."));
    return;
  }
  int64 elapsed = GetTickDeltaMicros(start_ticks_, GetTicks());
  start_ticks_ = 0;
  // SetReturnValue will fail and set an exception if elapsed exceeds
  // JS_INT_MAX, but this is an interval of 300 years!
  assert(elapsed >= 0);
  context->SetReturnValue(JSPARAM_INT64, &elapsed);
}

void GearsTest::RunTests(JsCallContext *context) {
  bool is_worker = false;
  JsArgument argv[] = {
    {JSPARAM_REQUIRED, JSPARAM_BOOL, &is_worker},
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  // We need permissions to use the localserver.
  SecurityOrigin cc_tests_origin;
  cc_tests_origin.InitFromUrl(STRING16(L"http://cc_tests/"));
  PermissionsDB *permissions = PermissionsDB::GetDB();
  if (!permissions) {
    context->SetException(GET_INTERNAL_ERROR_MESSAGE());
    return;
  }

  permissions->SetPermission(cc_tests_origin,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_ALLOWED);

  std::string16 error;
  bool ok = true;
  BrowsingContext *browsing_context = EnvPageBrowsingContext();
  ok &= TestAllMutex(&error);
  ok &= TestByteStore(&error);
  ok &= TestStringUtils(&error);
  ok &= TestFileUtils(&error);
  ok &= TestUrlUtils(&error);
  ok &= TestParseHttpStatusLine(&error);
  ok &= TestHttpRequest(browsing_context, &error);
  ok &= TestHttpCookies(browsing_context, &error);
  ok &= TestSecurityModel(&error);
  ok &= TestSqliteUtilsAll(&error);
  ok &= TestNameValueTableAll(&error);
  ok &= TestPermissionsDBAll(&error);
  ok &= TestDatabaseUtilsAll(&error);
  ok &= TestLocalServerDB(browsing_context, &error);
  ok &= TestResourceStore(&error);
  ok &= TestManifest(&error);
  ok &= TestManagedResourceStore(&error);
  ok &= TestMemoryBuffer(&error);
  ok &= TestMessageService(&error);
  ok &= TestSerialization(&error);
  ok &= TestCircularBuffer(&error);
  ok &= TestRefCount(&error);
  ok &= TestBlob(&error);

#if (defined(WIN32) && !defined(WINCE)) || \
    defined(LINUX) || defined(OS_MACOSX)
  ok &= TestIpcSystemQueue(&error);
#if BROWSER_IE
  ok &= TestIpcPeerQueue(&error);
#endif
#endif
#ifdef OS_ANDROID
  ok &= TestThreadMessageQueue(&error);
  ok &= TestJavaClass(&error);
#endif
#if BROWSER_FF
  ok &= TestBlobInputStreamFf(&error);
#endif
#if BROWSER_WEBKIT
  ok &= TestBlobInputStreamSf(&error);
#endif
  ok &= TestStopwatch(&error);
  ok &= TestJsonEscaping(&error);
  ok &= TestArray(GetJsRunner(), context, &error);
  ok &= TestEvent(&error);
  ok &= TestGeolocationDB(&error);
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef OS_ANDROID
  // The notification API has not been implemented for Android.
#else
  ok &= TestNotificationMessageOrdering(&error);
#endif  // OS_ANDROID
#endif  // OFFICIAL_BUILD

  // We have to call GetDB again since TestCapabilitiesDBAll deletes
  // the previous instance.
  permissions = PermissionsDB::GetDB();
  permissions->SetPermission(cc_tests_origin,
                             PermissionsDB::PERMISSION_LOCAL_DATA,
                             PermissionsDB::PERMISSION_NOT_SET);

  if (!ok) {
    if (error.empty()) {
      // If a test has failed but not set an error, set a generic message here.
      context->SetException(STRING16(L"RunTests failed."));
    } else {
      context->SetException(error);
    }
  }
}

void GearsTest::TestPassArguments(JsCallContext *context) {
  bool bool_value = false;
  int int_value = 0;
  int64 int64_value = 0;
  double double_value = 0.0;
  std::string16 string_value;

  JsArgument argv[] = {
    {JSPARAM_REQUIRED, JSPARAM_BOOL, &bool_value},
    {JSPARAM_REQUIRED, JSPARAM_INT, &int_value},
    {JSPARAM_REQUIRED, JSPARAM_INT64, &int64_value},
    {JSPARAM_REQUIRED, JSPARAM_DOUBLE, &double_value},
    {JSPARAM_REQUIRED, JSPARAM_STRING16, &string_value}
  };

  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (!bool_value) {
    context->SetException(STRING16(L"Incorrect value for parameter 1"));
  } else if (int_value != 42) {
    context->SetException(STRING16(L"Incorrect value for parameter 2"));
  } else if (int64_value != (GG_LONGLONG(1) << 42)) {
    context->SetException(STRING16(L"Incorrect value for parameter 3"));
  } else if (double_value != 88.8) {
    context->SetException(STRING16(L"Incorrect value for parameter 4"));
  } else if (string_value != STRING16(L"hotdog")) {
    context->SetException(STRING16(L"Incorrect value for parameter 5"));
  }
}

void GearsTest::TestPassArgumentsCallback(JsCallContext *context) {
  scoped_ptr<JsRootedCallback> function;
  JsArgument argv[] = {
    {JSPARAM_REQUIRED, JSPARAM_FUNCTION, as_out_parameter(function)},
  };
  assert(&argv);
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  bool bool_value = true;
  int int_value = 42;
  int64 int64_value = (GG_LONGLONG(1) << 42);
  double double_value = 88.8;
  std::string16 string_value(STRING16(L"hotdog"));
  JsParamToSend out_argv[] = {
    {JSPARAM_BOOL, &bool_value},
    {JSPARAM_INT, &int_value},
    {JSPARAM_INT64, &int64_value},
    {JSPARAM_DOUBLE, &double_value},
    {JSPARAM_STRING16, &string_value},
  };

  GetJsRunner()->InvokeCallback(function.get(), ARRAYSIZE(out_argv),
                                out_argv, NULL);
}

void GearsTest::TestPassArgumentsOptional(JsCallContext *context) {
  int int_values[3] = {};

  JsArgument argv[] = {
    {JSPARAM_REQUIRED, JSPARAM_INT, &(int_values[0])},
    {JSPARAM_OPTIONAL, JSPARAM_INT, &(int_values[1])},
    {JSPARAM_OPTIONAL, JSPARAM_INT, &(int_values[2])}
  };

  int argc = context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  for (int i = 0; i < argc; ++i) {
    if (int_values[i] != 42) {
      std::string16 error(STRING16(L"Incorrect value for parameter "));
      error += IntegerToString16(i + 1);
      error += STRING16(L".");
      context->SetException(error);
      return;
    }
  }
}

void GearsTest::TestObjectProperties(JsCallContext *context) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestObject failed at line %d\n", __LINE__)); \
    context->SetException(STRING16(L"TestObjectProperties failed.")); \
    return; \
  } \
}
  JsRunnerInterface *js_runner = GetJsRunner();

  // Internal tests on JsObject.
  scoped_ptr<JsObject> test_object(js_runner->NewObject());
  JsScopedToken token;
  static const char16 *kPropertyName = STRING16(L"genericPropertyName");
  // Test that a new object has no properties.
  std::vector<std::string16> property_names;
  TEST_ASSERT(test_object.get()->GetPropertyNames(&property_names));
  TEST_ASSERT(property_names.empty());
  TEST_ASSERT(test_object.get()->GetPropertyType(kPropertyName) ==
              JSPARAM_UNDEFINED);
  TEST_ASSERT(test_object.get()->GetProperty(kPropertyName, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
  // Test that we can set a property.
  const int int_in = 42;
  int int_out;
  TEST_ASSERT(test_object.get()->SetPropertyInt(kPropertyName, int_in));
  // Test that we can get a property.
  TEST_ASSERT(test_object.get()->GetPropertyType(kPropertyName) == JSPARAM_INT);
  TEST_ASSERT(test_object.get()->GetPropertyAsInt(kPropertyName, &int_out));
  TEST_ASSERT(int_in == int_out);
  // Test that we can reset a property.
  const std::string16 string_in(STRING16(L"test"));
  std::string16 string_out;
  TEST_ASSERT(test_object.get()->SetPropertyString(kPropertyName, string_in));
  TEST_ASSERT(test_object.get()->GetPropertyType(kPropertyName) ==
              JSPARAM_STRING16);
  TEST_ASSERT(test_object.get()->GetPropertyAsString(kPropertyName,
                                                     &string_out));
  TEST_ASSERT(string_in == string_out);
}

// An object is passed in from JavaScript with properties set to test all the
// JsObject::Get* functions.
void GearsTest::TestPassObject(JsCallContext *context) {
  const int argc = 1;
  JsObject obj;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_OBJECT, &obj } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  TestObjectBool(context, obj);
  if (context->is_exception_set()) return;

  TestObjectInt(context, obj);
  if (context->is_exception_set()) return;

  TestObjectDouble(context, obj);
  if (context->is_exception_set()) return;

  TestObjectString(context, obj);
  if (context->is_exception_set()) return;

  TestObjectArray(context, obj, *this);
  if (context->is_exception_set()) return;

  TestObjectObject(context, obj);
  if (context->is_exception_set()) return;

  TestObjectFunction(context, obj, *this);
}

void GearsTest::TestCreateObject(JsCallContext* context) {
  const int argc = 1;
  scoped_ptr<JsRootedCallback> func;
  JsArgument argv[argc] = { { JSPARAM_REQUIRED, JSPARAM_FUNCTION,
                              as_out_parameter(func) } };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  JsRunnerInterface* js_runner = GetJsRunner();
  if (!js_runner)
    context->SetException(STRING16(L"Failed to get JsRunnerInterface."));

  scoped_ptr<JsObject> js_object(js_runner->NewObject());
  if (!js_object.get())
    context->SetException(STRING16(L"Failed to create new javascript object."));

  CreateObjectBool(context, js_runner, js_object.get());
  if (context->is_exception_set()) return;

  CreateObjectInt(context, js_runner, js_object.get());
  if (context->is_exception_set()) return;

  CreateObjectDouble(context, js_runner, js_object.get());
  if (context->is_exception_set()) return;

  CreateObjectString(context, js_runner, js_object.get());
  if (context->is_exception_set()) return;

  CreateObjectArray(context, js_runner, func.get(), js_object.get());
  if (context->is_exception_set()) return;

  CreateObjectObject(context, js_runner, js_object.get());
  if (context->is_exception_set()) return;

  CreateObjectDate(context, js_runner, js_object.get());
  if (context->is_exception_set()) return;

  CreateObjectFunction(context, func.get(), js_object.get());
  if (context->is_exception_set()) return;

  context->SetReturnValue(JSPARAM_OBJECT, js_object.get());
}

// We don't test creation of an Error object as part of TestCreateObject, which
// tests every property of the object, because an Error object contains a number
// of hard-to-test properties, such as the line number at which the error
// occurred.
void GearsTest::TestCreateError(JsCallContext* context) {
  JsRunnerInterface* js_runner = GetJsRunner();
  if (!js_runner)
    context->SetException(STRING16(L"Failed to get JsRunnerInterface."));
  scoped_ptr<JsObject> js_object(js_runner->NewError(
      STRING16(L"test error\r\nwith 'special' \\characters\\")));
  if (!js_object.get()) {
    context->SetException(STRING16(L"Failed to create Error object"));
    return;
  }
  context->SetReturnValue(JSPARAM_OBJECT, js_object.get());
}

//------------------------------------------------------------------------------
// Coercion Tests
// TODO(aa): There is no real need to pass the values to coerce from JavaScript.
// Implement BoolToJsToken(), IntToJsToken(), etc and use to create tokens and
// test coercion directly in C++.
//------------------------------------------------------------------------------
// Coerces the first parameter to a bool and ensures the coerced value is equal
// to the expected value.
void GearsTest::TestCoerceBool(JsCallContext *context) {
  JsToken value;
  bool expected_value;
  const int argc = 2;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_TOKEN, &value },
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &expected_value }
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  bool coerced_value;
  if (!JsTokenToBool_Coerce(value, context->js_context(), &coerced_value)) {
    context->SetException(STRING16(L"Could not coerce argument to bool."));
    return;
  }

  bool ok = (coerced_value == expected_value);
  context->SetReturnValue(JSPARAM_BOOL, &ok);
}

// Coerces the first parameter to an int and ensures the coerced value is equal
// to the expected value.
void GearsTest::TestCoerceInt(JsCallContext *context) {
  JsToken value;
  int expected_value;
  const int argc = 2;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_TOKEN, &value },
    { JSPARAM_REQUIRED, JSPARAM_INT, &expected_value },
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  int coerced_value;
  if (!JsTokenToInt_Coerce(value, context->js_context(), &coerced_value)) {
    context->SetException(STRING16(L"Could not coerce argument to int."));
    return;
  }

  bool ok = (coerced_value == expected_value);
  context->SetReturnValue(JSPARAM_BOOL, &ok);
}

// Coerces the first parameter to a double and ensures the coerced value is
// equal to the expected value.
void GearsTest::TestCoerceDouble(JsCallContext *context) {
  JsToken value;
  double expected_value;
  const int argc = 2;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_TOKEN, &value },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &expected_value },
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  double coerced_value;
  if (!JsTokenToDouble_Coerce(value, context->js_context(), &coerced_value)) {
    context->SetException(STRING16(L"Could not coerce argument to double."));
    return;
  }

  bool ok = (coerced_value == expected_value);
  context->SetReturnValue(JSPARAM_BOOL, &ok);
}

// Coerces the first parameter to a string and ensures the coerced value is
// equal to the expected value.
void GearsTest::TestCoerceString(JsCallContext *context) {
  JsToken value;
  std::string16 expected_value;
  const int argc = 2;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_TOKEN, &value },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &expected_value },
  };
  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  std::string16 coerced_value;
  if (!JsTokenToString_Coerce(value, context->js_context(), &coerced_value)) {
    context->SetException(STRING16(L"Could not coerce argument to string."));
    return;
  }

  bool ok = (coerced_value == expected_value);
  context->SetReturnValue(JSPARAM_BOOL, &ok);
}

// Checks that the second parameter is of the type specified by the first
// parameter using GetType(). First parameter should be one of "bool", "int",
// "double", "string", "null", "undefined", "array", "function", "object".
void GearsTest::TestGetType(JsCallContext *context) {
  // Don't really care about the actual value of the second parameter. We
  // specify an argument of type JSPARAM_TOKEN because all types (other than
  // NULL and undefined) can be cast to this type by GetArguments. In these two
  // cases, GetArguments will not parse the argument (it is optional) and will
  // return 1, rather than 2.
  std::string16 type;
  JsToken value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &type },
    { JSPARAM_OPTIONAL, JSPARAM_TOKEN, &value },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;
  // At this point, the number of arguments can only be greater than
  // or equal to 1. If it was 0, an exception would have been set since
  // the first parameter of this function is marked as required.

  bool ok = false;
  JsParamType t = context->GetArgumentType(1);
  if (type == STRING16(L"bool") && t == JSPARAM_BOOL ||
      type == STRING16(L"int") && t == JSPARAM_INT ||
      type == STRING16(L"double") && t == JSPARAM_DOUBLE ||
      type == STRING16(L"string") && t == JSPARAM_STRING16 ||
      type == STRING16(L"null") && t == JSPARAM_NULL ||
      type == STRING16(L"undefined") && t == JSPARAM_UNDEFINED ||
      type == STRING16(L"array") && t == JSPARAM_ARRAY ||
      type == STRING16(L"function") && t == JSPARAM_FUNCTION ||
      type == STRING16(L"object") && t == JSPARAM_OBJECT) {
    ok = true;
  }
  context->SetReturnValue(JSPARAM_BOOL, &ok);
}

//------------------------------------------------------------------------------
// TestHttpCookies
//------------------------------------------------------------------------------
bool TestHttpCookies(BrowsingContext *context, std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestHttpCookies - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestHttpCookies - failed. "); \
    return false; \
  } \
}

  std::vector<std::string> tokens;
  std::string tokens_string("a,b,c,d");
  TEST_ASSERT(Tokenize(tokens_string, std::string(","), &tokens) == 4);
  TEST_ASSERT(tokens[0] == "a");
  TEST_ASSERT(tokens[1] == "b");
  TEST_ASSERT(tokens[2] == "c");
  TEST_ASSERT(tokens[3] == "d");
  tokens_string = ", aaaa;bbbb ,;cccc;,";
  TEST_ASSERT(Tokenize(tokens_string, std::string(",;"), &tokens) == 3);
  TEST_ASSERT(tokens[0] == " aaaa");
  TEST_ASSERT(tokens[1] == "bbbb ");
  TEST_ASSERT(tokens[2] == "cccc");

  std::string16 name, value;
  const std::string16 kName(STRING16(L"name"));
  const std::string16 kNameEq(STRING16(L"name="));
  const std::string16 kNameEqSp(STRING16(L"name= "));
  ParseCookieNameAndValue(kNameEq, &name, &value);
  TEST_ASSERT(name == kName);
  TEST_ASSERT(value.empty());
  ParseCookieNameAndValue(kNameEqSp, &name, &value);
  TEST_ASSERT(name == kName);
  TEST_ASSERT(value.empty());

  const std::string16 kValue(STRING16(L"value"));
  const std::string16 kName2(STRING16(L"name 2"));
  const std::string16 kValue2(STRING16(L"value 2"));
  const std::string16 kCookie3(STRING16(L"cookie3"));
  CookieMap map;
  std::string16 cookie_string(
        STRING16(L"name=value; name 2 = value 2; cookie3 "));
  ParseCookieString(cookie_string, &map);
  TEST_ASSERT(map.size() == 3);
  TEST_ASSERT(map.GetCookie(kName, &value));
  TEST_ASSERT(value == kValue);
  TEST_ASSERT(map.HasSpecificCookie(kName2, kValue2));
  TEST_ASSERT(map.HasCookie(kCookie3));
  TEST_ASSERT(!map.HasCookie(kCookie3 + kName2));

  TEST_ASSERT(GetCookieString(STRING16(L"http://www.google.com/"),
              context, &cookie_string));
  ParseCookieString(cookie_string, &map);

  LOG(("TestHttpCookies - passed\n"));
  return true;
}


//------------------------------------------------------------------------------
// TestManifest
//------------------------------------------------------------------------------
bool TestManifest(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestManifest - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestManifest - failed. "); \
    return false; \
  } \
}

  const char16 *manifest_url = STRING16(L"http://cc_tests/manifest.json");
  const std::string16 expected_version = STRING16(L"expected_version");
  const std::string16 expected_redirect(
                 STRING16(L"http://cc_tests.other_origin/redirectUrl"));
  const char16 *json16 = STRING16(
    L"{ 'betaManifestVersion': 1, \n"
    L"  'version': 'expected_version', \n"
    L"  'redirectUrl': 'http://cc_tests.other_origin/redirectUrl', \n"
    L"  'entries': [ \n"
    L"       { 'url': 'test_url', 'src': 'test_src' }, \n"
    L"       { 'url': 'test_url2' }, \n"
    L"       { 'url': 'test_url3', 'ignoreQuery': true}, \n"
    L"       { 'url': 'test_redirect_url', 'redirect': 'test_url3?blah' } \n"
    L"     ] \n"
    L"}");

  std::string json8;
  TEST_ASSERT(String16ToUTF8(json16, &json8));
  ReplaceAll(json8, std::string("'"), std::string("\""));

  Manifest manifest;
  bool ok = manifest.Parse(manifest_url, json8.c_str(), json8.length());
  TEST_ASSERT(ok);
  TEST_ASSERT(manifest.IsValid());
  TEST_ASSERT(expected_version == manifest.GetVersion());
  TEST_ASSERT(expected_redirect == manifest.GetRedirectUrl());
  TEST_ASSERT(manifest.GetEntries()->size() == 4);

  const Manifest::Entry *entry1 = &manifest.GetEntries()->at(0);
  TEST_ASSERT(entry1->url == STRING16(L"http://cc_tests/test_url"));
  TEST_ASSERT(entry1->src == STRING16(L"http://cc_tests/test_src"));
  TEST_ASSERT(entry1->redirect.empty());
  TEST_ASSERT(!entry1->ignore_query);

  const Manifest::Entry *entry2 = &manifest.GetEntries()->at(1);
  TEST_ASSERT(entry2->url == STRING16(L"http://cc_tests/test_url2"));
  TEST_ASSERT(entry2->src.empty());
  TEST_ASSERT(entry2->redirect.empty());

  const Manifest::Entry *entry3 = &manifest.GetEntries()->at(2);
  TEST_ASSERT(entry3->url == STRING16(L"http://cc_tests/test_url3"));
  TEST_ASSERT(entry3->src.empty());
  TEST_ASSERT(entry3->redirect.empty());
  TEST_ASSERT(entry3->ignore_query);

  const Manifest::Entry *entry4 = &manifest.GetEntries()->at(3);
  TEST_ASSERT(entry4->url == STRING16(L"http://cc_tests/test_redirect_url"));
  TEST_ASSERT(entry4->src.empty());
  TEST_ASSERT(entry4->redirect == STRING16(L"http://cc_tests/test_url3?blah"));


  const char *json_not_an_object = "\"A string, but we need an object\"";
  Manifest manifest_should_not_parse;
  ok = manifest_should_not_parse.Parse(manifest_url, json_not_an_object);
  TEST_ASSERT(!ok);

  LOG(("TestManifest - passed\n"));
  return true;
}

//------------------------------------------------------------------------------
// TestResourceStore
//------------------------------------------------------------------------------
bool TestResourceStore(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestResourceStore - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestResourceStore - failed. "); \
    return false; \
  } \
}
  const char16 *name = STRING16(L"name");
  const char16 *url1 = STRING16(L"http://cc_tests/url1");
  const char16 *url2 = STRING16(L"http://cc_tests/url2");
  const char16 *url3 = STRING16(L"http://cc_tests/url3");
  const char16 *required_cookie = STRING16(L"required_cookie");
  const char *data1 = "Hello world";
  const char16 *headers1 =
      STRING16(L"Content-Type: text/plain\r\nContent-Length: 11\r\n\r\n");

  SecurityOrigin security_origin;
  TEST_ASSERT(security_origin.InitFromUrl(url1));

  ResourceStore wcs;
  TEST_ASSERT(wcs.CreateOrOpen(security_origin, name, required_cookie));

  ResourceStore::Item item1;
  item1.entry.url = url1;
  item1.payload.headers = headers1;
  item1.payload.data.reset(new std::vector<uint8>);
  item1.payload.data->assign(data1, data1 + strlen(data1));
  item1.payload.status_line = STRING16(L"HTTP/1.0 200 OK");
  item1.payload.status_code = HttpConstants::HTTP_OK;
  TEST_ASSERT(wcs.PutItem(&item1));

  TEST_ASSERT(wcs.IsCaptured(url1));

  std::string16 headers;
  TEST_ASSERT(wcs.GetAllHeaders(url1, &headers));
  TEST_ASSERT(headers == item1.payload.headers);

  std::string16 content_type;
  TEST_ASSERT(wcs.GetHeader(url1, STRING16(L"Content-Type"), &content_type));
  TEST_ASSERT(content_type == STRING16(L"text/plain"));

  ResourceStore::Item test_item1;
  TEST_ASSERT(wcs.GetItem(url1, &test_item1));

  TEST_ASSERT(wcs.Copy(url1, url2));

  ResourceStore::Item test_item2;
  TEST_ASSERT(wcs.GetItem(url2, &test_item2));
  TEST_ASSERT(test_item1.entry.id != test_item2.entry.id);
  TEST_ASSERT(test_item1.entry.payload_id == test_item2.entry.payload_id);
  TEST_ASSERT(test_item1.entry.src == test_item2.entry.src);
  TEST_ASSERT(test_item1.entry.url != test_item2.entry.url);
  TEST_ASSERT(test_item1.entry.version_id == test_item2.entry.version_id);
  TEST_ASSERT(test_item1.payload.id == test_item2.payload.id);
  TEST_ASSERT(test_item1.payload.creation_date ==
              test_item2.payload.creation_date);
  TEST_ASSERT(test_item1.payload.headers == test_item2.payload.headers);
  TEST_ASSERT(test_item1.payload.status_line == test_item2.payload.status_line);
  TEST_ASSERT(test_item1.payload.status_code == test_item2.payload.status_code);

  TEST_ASSERT(wcs.Rename(url2, url3));

  ResourceStore::Item test_item3;
  TEST_ASSERT(wcs.GetItem(url3, &test_item3));
  TEST_ASSERT(test_item3.entry.id == test_item2.entry.id);
  TEST_ASSERT(test_item3.entry.payload_id == test_item2.entry.payload_id);
  TEST_ASSERT(test_item3.entry.src == test_item2.entry.src);
  TEST_ASSERT(test_item3.entry.url != test_item2.entry.url);
  TEST_ASSERT(test_item3.entry.version_id == test_item2.entry.version_id);
  TEST_ASSERT(test_item3.payload.id == test_item2.payload.id);
  TEST_ASSERT(test_item3.payload.creation_date ==
              test_item2.payload.creation_date);
  TEST_ASSERT(test_item3.payload.headers == test_item2.payload.headers);
  TEST_ASSERT(test_item3.payload.status_line == test_item2.payload.status_line);
  TEST_ASSERT(test_item3.payload.status_code == test_item2.payload.status_code);

  LOG(("TestResourceStore - passed\n"));
  return true;
}


//------------------------------------------------------------------------------
// TestManagedResourceStore
//------------------------------------------------------------------------------
bool TestManagedResourceStore(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestManagedResourceStore - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestManagedResourceStore - failed. "); \
    return false; \
  } \
}

  const char *manifest1_json =
    "{ \"betaManifestVersion\": 1, "
    "  \"version\": \"test_version\", "
    "  \"redirectUrl\": \"redirectUrl\", "
    "  \"entries\": [ "
    "       { \"url\": \"test_url\", \"src\": \"test_src\" }, "
    "       { \"url\": \"test_url2\" }, "
    "       { \"url\": \"test_url3\", \"ignoreQuery\": true } "
    "     ]"
    "}";

  const char16 *manifest_url = STRING16(L"http://cc_tests/manifest.json");
  const char16 *name = STRING16(L"name");
  const char16 *required_cookie = STRING16(L"user=joe");

  SecurityOrigin security_origin;
  TEST_ASSERT(security_origin.InitFromUrl(manifest_url));

  // Clear out data from previous test runs
  int64 existing_store_id = WebCacheDB::kUnknownID;
  if (ManagedResourceStore::ExistsInDB(security_origin, name,
                                       required_cookie, &existing_store_id)) {
    ManagedResourceStore remover;
    TEST_ASSERT(remover.Open(existing_store_id));
    TEST_ASSERT(remover.Remove());
  }
  // Bring the ManagedResourceStore thru the states that it will go thru
  // during an install / update process, and verify that it works as expected.

  ManagedResourceStore app;
  TEST_ASSERT(app.CreateOrOpen(security_origin, name, required_cookie));

  // Ensure it looks freshly created
  TEST_ASSERT(app.StillExistsInDB());
  TEST_ASSERT(!app.HasVersion(WebCacheDB::VERSION_CURRENT));
  TEST_ASSERT(!app.HasVersion(WebCacheDB::VERSION_DOWNLOADING));

  // Test Get/Set UpdateInfo
  WebCacheDB::UpdateStatus update_status;
  int64 last_time;
  TEST_ASSERT(app.GetUpdateInfo(&update_status, &last_time, NULL, NULL));
  TEST_ASSERT(update_status == WebCacheDB::UPDATE_OK);
  TEST_ASSERT(last_time == 0);
  TEST_ASSERT(app.SetUpdateInfo(WebCacheDB::UPDATE_FAILED, 1, NULL, NULL));
  TEST_ASSERT(app.GetUpdateInfo(&update_status, &last_time, NULL, NULL));
  TEST_ASSERT(update_status == WebCacheDB::UPDATE_FAILED);
  TEST_ASSERT(last_time == 1);

  // Add a version in the "downloading" state

  Manifest manifest1;
  TEST_ASSERT(manifest1.Parse(manifest_url, manifest1_json));

  int64 manifest1_version_id;
  TEST_ASSERT(app.AddManifestAsDownloadingVersion(&manifest1,
                                                  &manifest1_version_id));
  TEST_ASSERT(app.HasVersion(WebCacheDB::VERSION_DOWNLOADING));

  std::string16 version_string;
  TEST_ASSERT(app.GetVersionString(WebCacheDB::VERSION_DOWNLOADING,
                                   &version_string));
  TEST_ASSERT(version_string == manifest1.GetVersion());

  // Transition to current

  TEST_ASSERT(app.SetDownloadingVersionAsCurrent());
  TEST_ASSERT(app.HasVersion(WebCacheDB::VERSION_CURRENT));
  TEST_ASSERT(!app.HasVersion(WebCacheDB::VERSION_DOWNLOADING));

  LOG(("TestManagedResourceStore - passed\n"));
  return true;
}

//------------------------------------------------------------------------------
// TestLocalServerDB
//------------------------------------------------------------------------------
bool TestLocalServerDB(BrowsingContext *context, std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestWebCacheDB - failed (%d)\n", __LINE__)); \
    SetFakeCookieString(NULL, NULL); \
    assert(error); \
    *error += STRING16(L"TestWebCacheDB - failed. "); \
    return false; \
  } \
}

  const char16 *name = STRING16(L"name");
  const char16 *required_cookie = STRING16(L"user=joe");
  const char16 *testurl = STRING16(L"http://cc_tests/url");

  SecurityOrigin security_origin;
  TEST_ASSERT(security_origin.InitFromUrl(testurl));

  WebCacheDB *db = WebCacheDB::GetDB();
  TEST_ASSERT(db);

  // delete existing info from a previous test run
  WebCacheDB::ServerInfo existing_server;
  if (db->FindServer(security_origin, name, required_cookie,
                     WebCacheDB::MANAGED_RESOURCE_STORE,
                     &existing_server)) {
    db->DeleteServer(existing_server.id);
  }

  // insert a server
  WebCacheDB::ServerInfo server;
  server.server_type = WebCacheDB::MANAGED_RESOURCE_STORE;
  server.security_origin_url = security_origin.url();
  server.name = name;
  server.required_cookie = required_cookie;
  server.manifest_url = STRING16(L"http://cc_tests/manifest_url");
  TEST_ASSERT(db->InsertServer(&server));

  // insert a current version1 that specifies a redirect
  WebCacheDB::VersionInfo version1;
  version1.server_id = server.id;
  version1.version_string = STRING16(L"version_string");
  version1.ready_state = WebCacheDB::VERSION_CURRENT;
  version1.session_redirect_url = STRING16(L"http://cc_tests/redirect_url");
  TEST_ASSERT(db->InsertVersion(&version1));

  // insert an entry with a bogus payload id for the current version
  WebCacheDB::EntryInfo entry;
  entry.version_id = version1.id;
  entry.url = testurl;
  entry.payload_id = kint64max;
  TEST_ASSERT(db->InsertEntry(&entry));

  // we should be able to service a request for testurl, the response
  // should redirect to our session_redirect_url
  SetFakeCookieString(testurl, NULL);
  TEST_ASSERT(db->CanService(testurl, context));

  WebCacheDB::PayloadInfo payload;
  TEST_ASSERT(db->Service(testurl, context, true, &payload));
  TEST_ASSERT(payload.IsHttpRedirect());
  std::string16 test_redirect_url;
  TEST_ASSERT(payload.GetHeader(HttpConstants::kLocationHeader,
                                &test_redirect_url) &&
              test_redirect_url == version1.session_redirect_url);

  // insert a downloaded version2 w/o a redirect
  WebCacheDB::VersionInfo version2;
  version2.server_id = server.id;
  version2.version_string = STRING16(L"version_string2");
  version2.ready_state = WebCacheDB::VERSION_DOWNLOADING;
  TEST_ASSERT(db->InsertVersion(&version2));

  // insert an entry for version2
  WebCacheDB::EntryInfo entry2;
  entry2.version_id = version2.id;
  entry2.url = testurl;
  entry2.payload_id = kint64max;
  TEST_ASSERT(db->InsertEntry(&entry2));

  // we should still be able to service a request for testurl from version1
  TEST_ASSERT(db->CanService(testurl, context));

  // delete version1
  TEST_ASSERT(db->DeleteVersion(version1.id));

  // we shouldn't be able to service a request for testurl
  TEST_ASSERT(!db->CanService(testurl, context));

  // now make the ready version current
  TEST_ASSERT(db->UpdateVersion(version2.id, WebCacheDB::VERSION_CURRENT));

  // we should still not be able to service a request for testurl as there
  // is no session yet and version2 does not have a redirect_url
  TEST_ASSERT(!db->CanService(testurl, context));

  // now set the required cookie (fake)
  SetFakeCookieString(testurl, required_cookie);

  // we should be able to service a request for testurl again
  TEST_ASSERT(db->CanService(testurl, context));

  // clear the cookie string for our testurl (fake)
  SetFakeCookieString(testurl, NULL);

  // off again
  TEST_ASSERT(!db->CanService(testurl, context));

  // delete version2
  TEST_ASSERT(db->DeleteVersion(version2.id));

  // insert version3 for that server that requires a session and has a redirect
  WebCacheDB::VersionInfo version3;
  version3.server_id = server.id;
  version3.version_string = STRING16(L"version_string3");
  version3.ready_state = WebCacheDB::VERSION_CURRENT;
  version3.session_redirect_url = STRING16(
                                      L"http://cc_tests/session_redirect_url");
  TEST_ASSERT(db->InsertVersion(&version3));

  // insert an entry for the ready version
  WebCacheDB::EntryInfo entry3;
  entry3.version_id = version3.id;
  entry3.url = testurl;
  entry3.ignore_query = true;
  entry3.payload_id = kint64max;
  TEST_ASSERT(db->InsertEntry(&entry3));

  // this entry s/b hit for request with arbitrary query parameters
  std::string16 testurl_query(testurl);
  testurl_query += STRING16(L"?blah");

  // on again, s/b responding with redirects
  SetFakeCookieString(testurl, NULL);
  TEST_ASSERT(db->CanService(testurl, context));
  SetFakeCookieString(testurl_query.c_str(), NULL);
  TEST_ASSERT(db->CanService(testurl_query.c_str(), context));

  // still on, s/b responding with payloads
  SetFakeCookieString(testurl, required_cookie);
  TEST_ASSERT(db->CanService(testurl, context));
  SetFakeCookieString(testurl_query.c_str(), required_cookie);
  TEST_ASSERT(db->CanService(testurl_query.c_str(), context));

  // delete the server altogether
  TEST_ASSERT(db->DeleteServer(server.id));

  // we shouldn't be able to service a request for the test url
  TEST_ASSERT(!db->CanService(testurl, context));
  TEST_ASSERT(!db->CanService(testurl_query.c_str(), context));

  SetFakeCookieString(NULL, NULL);

  // we made it thru as expected
  LOG(("TestWebCacheDB - passed\n"));
  return true;
}

bool TestParseHttpStatusLine(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestParseHttpStatusLine - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestParseHttpStatusLine - failed. "); \
    return false; \
  } \
}
  std::string16 good(STRING16(L"HTTP/1.1 200 OK"));
  std::string16 version, text;
  int code;
  TEST_ASSERT(ParseHttpStatusLine(good, &version, &code, &text));
  TEST_ASSERT(version == STRING16(L"HTTP/1.1"));
  TEST_ASSERT(code == 200);
  TEST_ASSERT(text == STRING16(L"OK"));
  TEST_ASSERT(ParseHttpStatusLine(good, &version, NULL, NULL));
  TEST_ASSERT(ParseHttpStatusLine(good, NULL, &code, NULL));
  TEST_ASSERT(ParseHttpStatusLine(good, NULL, NULL, &text));

  const char16 *acceptable[] = {
    STRING16(L"HTTP/1.0 200"),  // no status
    STRING16(L"HTTP 200 ABBREVIATED VERSION"),
    STRING16(L"HTTP/1.1 500 REASON: CONTAINING COLON")
  };
  for (size_t i = 0; i < ARRAYSIZE(acceptable); ++i) {
    std::string16 acceptable_str(acceptable[i]);
    TEST_ASSERT(ParseHttpStatusLine(acceptable_str, NULL, NULL, NULL));
  }

  const char16 *bad[] = {
    STRING16(L" HTTP/1.1 200 SPACE AT START"),
    STRING16(L"WTFP/1.1 200 WRONG SCHEME"),
    STRING16(L"HTTP/1.1 2 CODE TOO SMALL"),
    STRING16(L"HTTP/1.0 2000 CODE TOO BIG"),
    STRING16(L"HTTP/1.0 NO CODE"),
    STRING16(L"complete_gibberish"),
    STRING16(L""),  // an empty string
    STRING16(L"    \t \t  "),  // whitespace only
  };
  for (size_t i = 0; i < ARRAYSIZE(bad); ++i) {
    std::string16 bad_str(bad[i]);
    TEST_ASSERT(!ParseHttpStatusLine(bad_str, NULL, NULL, NULL));
  }
  LOG(("TestParseHttpStatusLine - passed\n"));
  return true;
}


class TestHttpRequestListener : public HttpRequest::HttpListener {
 public:
  explicit TestHttpRequestListener(HttpRequest *request) : request_(request) {}

  virtual void ReadyStateChanged(HttpRequest *source) {
    HttpRequest::ReadyState state = HttpRequest::UNINITIALIZED;
    source->GetReadyState(&state);
    if (state == HttpRequest::COMPLETE) {
      int status = 0;
      std::string16 headers;
      scoped_refptr<BlobInterface> body;
      source->GetStatus(&status);
      source->GetAllResponseHeaders(&headers);
      source->GetResponseBody(&body);
      source->SetListener(NULL, false);
      delete this;
      LOG(("TestHttpRequest - complete (%d)\n", status));
    }
  }

  scoped_refptr<HttpRequest> request_;
};


bool TestHttpRequest(BrowsingContext *context, std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestHttpRequest - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestHttpRequest - failed. "); \
    return false; \
  } \
}

  scoped_refptr<HttpRequest> request;

  // Send a request synchronously
  TEST_ASSERT(HttpRequest::Create(&request));
  TestHttpRequestListener *listener =
      new TestHttpRequestListener(request.get());
  request->SetListener(listener, false);
  bool ok = request->Open(HttpConstants::kHttpGET,
                          STRING16(L"http://www.google.com/"),
                          false, context);
  if (ok) {
    // Sync requests are not fully supported yet, when they are revisit
    // note: we leak the request and listener in this case
    ok = request->Send(NULL);
    TEST_ASSERT(ok);
  } else {
    // TODO(michaeln): Once SafeHttpRequest::Open can do sync requests, then we
    // should not need an explicit clean-up, and let the listener self-delete
    // during TestHttpRequestListener::ReadyStateChanged.
    delete listener;
    listener = NULL;
  }
  request.reset(NULL);

  // Send an async request
  TEST_ASSERT(HttpRequest::Create(&request));
  request->SetListener(new TestHttpRequestListener(request.get()), false);
  ok = request->Open(HttpConstants::kHttpGET,
                     STRING16(L"http://www.google.com/"),
                     true, context);
  TEST_ASSERT(ok);
  ok = request->Send(NULL);
  TEST_ASSERT(ok);
  return true;
}

bool TestStopwatch(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestStopwatch - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestStopwatch - failed at line "); \
    *error += IntegerToString16(__LINE__); \
    *error += STRING16(L". "); \
    return false; \
  } \
}

// We define a millisecond-resolution sleep function because Windows does not
// provide an equivalent to usleep.
#ifdef WIN32
#define SleepForMilliseconds Sleep
#else
#define SleepForMilliseconds(x) \
{ \
  assert(x < 1000); \
  usleep(x * 1000); \
}
#endif

  // Test initialized to zero.
  Stopwatch sw1;
  TEST_ASSERT(sw1.GetElapsed() == 0);

  // Test simple use.
  // TODO(steveblock): Address this failing test and uncomment
  //Stopwatch sw2;
  //sw2.Start();
  //SleepForMilliseconds(10);
  //sw2.Stop();
  //TEST_ASSERT(sw2.GetElapsed() > 0);


  // Test small time increment.
  Stopwatch sw3;
  sw3.Start();
  sw3.Stop();
  TEST_ASSERT(sw3.GetElapsed() >= 0);

  // Test nested use.
  // TODO(steveblock): Address this failing test and uncomment
  //Stopwatch sw4;
  //sw4.Start();
  //sw4.Start();
  //sw4.Start();
  //SleepForMilliseconds(10);
  //sw4.Stop();
  //TEST_ASSERT(sw4.GetElapsed() == 0);
  //sw4.Stop();
  //TEST_ASSERT(sw4.GetElapsed() == 0);
  //sw4.Stop();
  //TEST_ASSERT(sw4.GetElapsed() > 0);

  // Test scoped stopwatch.
  // TODO(steveblock): Address this failing test and uncomment
  //Stopwatch sw5;
  //{
  //  ScopedStopwatch scopedStopwatch(&sw5);
  //  SleepForMilliseconds(10);
  //}
  //TEST_ASSERT(sw5.GetElapsed() > 0);

  return true;
}

bool TestJsonEscaping(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestJsonEscaping - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestJsonEscaping - failed. "); \
    return false; \
  } \
}

  Json::Value object1(Json::objectValue);
  object1["a"] = "foo";
  object1["b"] = "foo\nbar";
  object1["c"] = "\"foobar\"";
  object1["d"] = "bar\\";
  std::string serialized(object1.toStyledString());

  Json::Value object2;
  Json::Reader reader;
  TEST_ASSERT(reader.parse(serialized, object2));

  TEST_ASSERT(object1["a"] == object2["a"]);
  TEST_ASSERT(object1["b"] == object2["b"]);
  TEST_ASSERT(object1["c"] == object2["c"]);
  TEST_ASSERT(object1["d"] == object2["d"]);
  TEST_ASSERT(serialized == object2.toStyledString());

  return true;
}

bool TestArray(JsRunnerInterface *js_runner, JsCallContext *context,
               std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestArray - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestArray - failed. "); \
    return false; \
  } \
}
  // Internal tests on JsArray.
  scoped_ptr<JsArray> test_array(js_runner->NewArray());
  JsScopedToken token;
  // Test that the length of a new array is zero.
  int length;
  TEST_ASSERT(test_array.get()->GetLength(&length));
  TEST_ASSERT(0 == length);
  // Test that all elements in an empty array are undefined.
  TEST_ASSERT(test_array.get()->GetElementType(0) == JSPARAM_UNDEFINED);
  TEST_ASSERT(test_array.get()->GetElement(0, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
  // Test that we can set elements.
  int int_in = 10;
  JsParamToSend int_element = {JSPARAM_INT, &int_in};
  ConvertJsParamToToken(int_element, context->js_context(), &token);
  TEST_ASSERT(test_array.get()->SetElement(0, token));
  TEST_ASSERT(test_array.get()->SetElement(2, token));
  // Test that we can get elements.
  int int_out;
  TEST_ASSERT(test_array.get()->GetElementType(0) == JSPARAM_INT);
  TEST_ASSERT(test_array.get()->GetElement(0, &token));
  TEST_ASSERT(JsTokenToInt_NoCoerce(token, context->js_context(), &int_out));
  TEST_ASSERT(int_out == int_in);
  TEST_ASSERT(test_array.get()->GetElementType(2) == JSPARAM_INT);
  TEST_ASSERT(test_array.get()->GetElement(2, &token));
  TEST_ASSERT(JsTokenToInt_NoCoerce(token, context->js_context(), &int_out));
  TEST_ASSERT(int_out == int_in);
  // Test that out-of-range and unspecified elements are undefined.
  TEST_ASSERT(test_array.get()->GetElementType(-1) == JSPARAM_UNDEFINED);
  TEST_ASSERT(test_array.get()->GetElement(-1, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
  TEST_ASSERT(test_array.get()->GetElementType(1) == JSPARAM_UNDEFINED);
  TEST_ASSERT(test_array.get()->GetElement(1, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
  TEST_ASSERT(test_array.get()->GetElementType(3) == JSPARAM_UNDEFINED);
  TEST_ASSERT(test_array.get()->GetElement(3, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
  // Test that an element set with type undefined has type undefined.
  JsParamToSend undefined_element = {JSPARAM_UNDEFINED, NULL};
  ConvertJsParamToToken(undefined_element, context->js_context(), &token);
  TEST_ASSERT(test_array.get()->SetElement(4, token));
  TEST_ASSERT(test_array.get()->GetElementType(4) == JSPARAM_UNDEFINED);
  TEST_ASSERT(test_array.get()->GetElement(4, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
  // Test that we can reset elements.
  std::string16 string_in(STRING16(L"test"));
  std::string16 string_out;
  JsParamToSend string_element = {JSPARAM_STRING16, &string_in};
  ConvertJsParamToToken(string_element, context->js_context(), &token);
  TEST_ASSERT(test_array.get()->SetElement(0, token));
  TEST_ASSERT(test_array.get()->GetElementType(0) == JSPARAM_STRING16);
  TEST_ASSERT(test_array.get()->GetElement(0, &token));
  TEST_ASSERT(JsTokenToString_NoCoerce(token, context->js_context(),
                                       &string_out));
  TEST_ASSERT(string_out == string_in);
  return true;
}

// JsObject test functions

#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    char tmp[256]; \
    snprintf(tmp, sizeof(tmp), "TestObject - failed ( %u: %s\n", \
             __LINE__, __FILE__); \
    LOG(("%s", tmp)); \
    std::string16 message; \
    if (UTF8ToString16(tmp, &message)) { \
      context->SetException(message); \
    } else { \
      context->SetException(STRING16(L"Failed to convert error message.")); \
    } \
  } \
}

void TestObjectBool(JsCallContext* context, const JsObject& obj) {
  bool property_value = false;
  TEST_ASSERT(obj.GetPropertyAsBool(STRING16(L"bool_true"), &property_value));
  TEST_ASSERT(property_value == true);

  property_value = true;
  TEST_ASSERT(obj.GetPropertyAsBool(STRING16(L"bool_false"), &property_value));
  TEST_ASSERT(property_value == false);

  JsArray arr;
  TEST_ASSERT(obj.GetPropertyAsArray(STRING16(L"bool_array"), &arr));

  int length = -1;
  TEST_ASSERT(arr.GetLength(&length));
  TEST_ASSERT(length == 2);

  property_value = false;
  TEST_ASSERT(arr.GetElementAsBool(0, &property_value));
  TEST_ASSERT(property_value == true);

  property_value = true;
  TEST_ASSERT(arr.GetElementAsBool(1, &property_value));
  TEST_ASSERT(property_value == false);
}

const static int int_large = 1073741823;  // 2 ** 30 - 1

void TestObjectInt(JsCallContext* context, const JsObject& obj) {
  // integer (assumed to be tagged 32 bit signed integer,
  //          30 bits magnitude, 1 bit sign, 1 bit tag)
  int property_value = -1;
  TEST_ASSERT(obj.GetPropertyAsInt(STRING16(L"int_0"), &property_value));
  TEST_ASSERT(property_value == 0);

  property_value = -1;
  TEST_ASSERT(obj.GetPropertyAsInt(STRING16(L"int_1"), &property_value));
  TEST_ASSERT(property_value == 1);

  property_value = -1;
  TEST_ASSERT(obj.GetPropertyAsInt(STRING16(L"int_large"), &property_value));
  TEST_ASSERT(property_value == int_large);

  property_value = 1;
  TEST_ASSERT(obj.GetPropertyAsInt(STRING16(L"int_negative_1"),
                                    &property_value));
  TEST_ASSERT(property_value == -1);

  property_value = 1;
  TEST_ASSERT(obj.GetPropertyAsInt(STRING16(L"int_negative_large"),
                                    &property_value));
  TEST_ASSERT(property_value == -int_large);

  JsArray arr;
  TEST_ASSERT(obj.GetPropertyAsArray(STRING16(L"int_array"), &arr));

  int length = -1;
  TEST_ASSERT(arr.GetLength(&length));
  TEST_ASSERT(length == 5);

  property_value = -1;
  TEST_ASSERT(arr.GetElementAsInt(0, &property_value));
  TEST_ASSERT(property_value == 0);

  property_value = -1;
  TEST_ASSERT(arr.GetElementAsInt(1, &property_value));
  TEST_ASSERT(property_value == 1);

  property_value = -1;
  TEST_ASSERT(arr.GetElementAsInt(2, &property_value));
  TEST_ASSERT(property_value == int_large);

  property_value = 1;
  TEST_ASSERT(arr.GetElementAsInt(3, &property_value));
  TEST_ASSERT(property_value == -1);

  property_value = -1;
  TEST_ASSERT(arr.GetElementAsInt(4, &property_value));
  TEST_ASSERT(property_value == -int_large);
}

// Magic number is from:
// http://developer.mozilla.org/en/docs/
//   Core_JavaScript_1.5_Reference:Global_Objects:Number:MIN_VALUE
const static double JS_NUMBER_MIN_VALUE = 5e-324;

void TestObjectDouble(JsCallContext* context, const JsObject& obj) {
  // JavaScript interprets 1.0 as an integer.
  // This is why 1 is 1.01.
  double property_value = -1.0;
  TEST_ASSERT(obj.GetPropertyAsDouble(STRING16(L"double_0"), &property_value));
  TEST_ASSERT(property_value == 0.01);

  property_value = -1.0;
  TEST_ASSERT(obj.GetPropertyAsDouble(STRING16(L"double_1"), &property_value));
  TEST_ASSERT(property_value == 1.01);

  property_value = -1;
  TEST_ASSERT(obj.GetPropertyAsDouble(STRING16(L"double_large"),
                                      &property_value));
  TEST_ASSERT(property_value == std::numeric_limits<double>::max());

  property_value = 1;
  TEST_ASSERT(obj.GetPropertyAsDouble(STRING16(L"double_negative_1"),
                                    &property_value));
  TEST_ASSERT(property_value == -1.01);

  property_value = 1;
  TEST_ASSERT(obj.GetPropertyAsDouble(STRING16(L"double_negative_large"),
                                    &property_value));
  TEST_ASSERT(property_value == JS_NUMBER_MIN_VALUE);

  JsArray arr;
  TEST_ASSERT(obj.GetPropertyAsArray(STRING16(L"double_array"), &arr));

  int length = -1;
  TEST_ASSERT(arr.GetLength(&length));
  TEST_ASSERT(length == 5);

  property_value = -1;
  TEST_ASSERT(arr.GetElementAsDouble(0, &property_value));
  TEST_ASSERT(property_value == 0.01);

  property_value = -1;
  TEST_ASSERT(arr.GetElementAsDouble(1, &property_value));
  TEST_ASSERT(property_value == 1.01);

  property_value = -1;
  TEST_ASSERT(arr.GetElementAsDouble(2, &property_value));
  TEST_ASSERT(property_value == std::numeric_limits<double>::max());

  property_value = 1;
  TEST_ASSERT(arr.GetElementAsDouble(3, &property_value));
  TEST_ASSERT(property_value == -1.01);

  property_value = 1;
  TEST_ASSERT(arr.GetElementAsDouble(4, &property_value));
  TEST_ASSERT(property_value == JS_NUMBER_MIN_VALUE);
}

void TestObjectString(JsCallContext* context, const JsObject& obj) {
  std::string16 property_value = STRING16(L"not empty");
  TEST_ASSERT(obj.GetPropertyAsString(STRING16(L"string_0"), &property_value));
  TEST_ASSERT(property_value.empty());

  property_value = STRING16(L"");
  TEST_ASSERT(obj.GetPropertyAsString(STRING16(L"string_1"), &property_value));
  TEST_ASSERT(property_value == STRING16(L"a"));

  property_value = STRING16(L"");
  TEST_ASSERT(obj.GetPropertyAsString(STRING16(L"string_many"),
                                      &property_value));
  const static std::string16 string_many(
      STRING16(L"asdjh1)!(@#*h38ind89!03234bnmd831%%%*&*jdlwnd8893asd1233"));
  TEST_ASSERT(property_value == string_many);

  JsArray arr;
  TEST_ASSERT(obj.GetPropertyAsArray(STRING16(L"string_array"), &arr));

  int length = -1;
  TEST_ASSERT(arr.GetLength(&length));
  TEST_ASSERT(length == 3);

  property_value = STRING16(L"");
  TEST_ASSERT(arr.GetElementAsString(0, &property_value));
  TEST_ASSERT(property_value.empty());

  property_value = STRING16(L"");
  TEST_ASSERT(arr.GetElementAsString(1, &property_value));
  TEST_ASSERT(property_value == STRING16(L"a"));

  property_value = STRING16(L"");
  TEST_ASSERT(arr.GetElementAsString(2, &property_value));
  TEST_ASSERT(property_value == string_many);
}

// The property with property_name is expected to be expected_length and contain
// integers where index == int value. I.e. length 3 is expected to be [0, 1, 2].
static bool ValidateGeneratedArray(const JsArray& arr, int expected_length) {
  int length = -1;
  if (!arr.GetLength(&length))
    return false;

  if (length != expected_length)
    return false;

  for (int i = 0; i < length; ++i) {
    int current_element = -1;
    if (!arr.GetElementAsInt(i, &current_element))
      return false;

    if (current_element != i)
      return false;
  }

  return true;
}

static bool ValidateGeneratedArray(const JsObject& obj,
                                   const std::string16& property_name,
                                   int expected_length) {
  JsArray arr;
  if (!obj.GetPropertyAsArray(property_name, &arr))
    return false;

  return ValidateGeneratedArray(arr, expected_length);
}

void TestObjectArray(JsCallContext* context,
                     const JsObject& obj,
                     const ModuleImplBaseClass& base) {
  TEST_ASSERT(ValidateGeneratedArray(obj, STRING16(L"array_0"), 0));
  TEST_ASSERT(ValidateGeneratedArray(obj, STRING16(L"array_1"), 1));
  TEST_ASSERT(ValidateGeneratedArray(obj, STRING16(L"array_8"), 8));
  TEST_ASSERT(ValidateGeneratedArray(obj, STRING16(L"array_10000"), 10000));

  JsArray array_many_types;
  TEST_ASSERT(obj.GetPropertyAsArray(STRING16(L"array_many_types"),
                                     &array_many_types));
  int array_many_types_length = -1;
  TEST_ASSERT(array_many_types.GetLength(&array_many_types_length));
  TEST_ASSERT(array_many_types_length == 7);

  // index 0
  bool bool_false = true;
  TEST_ASSERT(array_many_types.GetElementAsBool(0, &bool_false));
  TEST_ASSERT(bool_false == false);

  // index 1
  bool bool_true = false;
  TEST_ASSERT(array_many_types.GetElementAsBool(1, &bool_true));
  TEST_ASSERT(bool_true == true);

  // index 2
  int int_2 = -1;
  TEST_ASSERT(array_many_types.GetElementAsInt(2, &int_2));
  TEST_ASSERT(int_2 == 2);

  // index 3
  std::string16 string_3;
  TEST_ASSERT(array_many_types.GetElementAsString(3, &string_3));
  TEST_ASSERT(string_3 == STRING16(L"3"));

  // index 4
  double double_4 = -1;
  TEST_ASSERT(array_many_types.GetElementAsDouble(4, &double_4));
  TEST_ASSERT(double_4 == 4.01);

  // index 5
  JsArray array_5;
  TEST_ASSERT(array_many_types.GetElementAsArray(5, &array_5));
  TEST_ASSERT(ValidateGeneratedArray(array_5, 5));

  // index 6
  scoped_ptr<JsRootedCallback> function_6;
  TEST_ASSERT(array_many_types.GetElementAsFunction(6,
                                                 as_out_parameter(function_6)));
  TEST_ASSERT(function_6.get());
  JsRunnerInterface* js_runner = base.GetJsRunner();
  TEST_ASSERT(js_runner);
  scoped_ptr<JsRootedToken> retval;
  TEST_ASSERT(js_runner->InvokeCallback(function_6.get(), 0, NULL,
                                        as_out_parameter(retval)));
  std::string16 string_retval;
  JsContextPtr js_context = context->js_context();
  TEST_ASSERT(JsTokenToString_NoCoerce(retval->token(), js_context,
                                       &string_retval));
  TEST_ASSERT(string_retval == STRING16(L"i am a function"));

  // Test that out-of-range elements are undefined.
  JsScopedToken token;
  TEST_ASSERT(array_many_types.GetElementType(-1) == JSPARAM_UNDEFINED);
  TEST_ASSERT(array_many_types.GetElement(-1, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
  TEST_ASSERT(array_many_types.GetElementType(7) == JSPARAM_UNDEFINED);
  TEST_ASSERT(array_many_types.GetElement(7, &token));
  TEST_ASSERT(JsTokenGetType(token, context->js_context()) ==
              JSPARAM_UNDEFINED);
}

void TestObjectObject(JsCallContext* context, const JsObject& obj) {
  JsObject child_obj;
  TEST_ASSERT(obj.GetPropertyAsObject(STRING16(L"obj"), &child_obj));

  bool bool_true = false;
  TEST_ASSERT(child_obj.GetPropertyAsBool(STRING16(L"bool_true"), &bool_true));
  TEST_ASSERT(bool_true == true);

  int int_0 = -1;
  TEST_ASSERT(child_obj.GetPropertyAsInt(STRING16(L"int_0"), &int_0));
  TEST_ASSERT(int_0 == 0);

  double double_0 = -1.0;
  TEST_ASSERT(child_obj.GetPropertyAsDouble(STRING16(L"double_0"), &double_0));
  TEST_ASSERT(double_0 == 0.01);

  std::string16 string_0(STRING16(L"not empty"));
  TEST_ASSERT(child_obj.GetPropertyAsString(STRING16(L"string_0"), &string_0));
  TEST_ASSERT(string_0.empty());

  TEST_ASSERT(ValidateGeneratedArray(child_obj, STRING16(L"array_0"), 0));
}

void TestObjectFunction(JsCallContext* context,
                        const JsObject& obj,
                        const ModuleImplBaseClass& base) {
  scoped_ptr<JsRootedCallback> function;
  TEST_ASSERT(obj.GetPropertyAsFunction(STRING16(L"func"),
                                        as_out_parameter(function)));
  TEST_ASSERT(function.get());
  JsRunnerInterface* js_runner = base.GetJsRunner();
  TEST_ASSERT(js_runner);
  scoped_ptr<JsRootedToken> retval;
  TEST_ASSERT(js_runner->InvokeCallback(function.get(), 0, NULL,
                                        as_out_parameter(retval)));
  TEST_ASSERT(retval.get());
  std::string16 string_retval;
  JsContextPtr js_context = context->js_context();
  TEST_ASSERT(JsTokenToString_NoCoerce(retval->token(), js_context,
                                       &string_retval));
  TEST_ASSERT(string_retval == STRING16(L"i am a function"));
}

#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    char tmp[256]; \
    snprintf(tmp, sizeof(tmp), "CreateObject - failed ( %u: %s\n", \
             __LINE__, __FILE__); \
    LOG(("%s", tmp)); \
    std::string16 message; \
    if (UTF8ToString16(tmp, &message)) { \
      context->SetException(message); \
    } else { \
      context->SetException(STRING16(L"Failed to convert error message.")); \
    } \
  } \
}

void CreateObjectBool(JsCallContext* context,
                      JsRunnerInterface* js_runner,
                      JsObject* out) {
  TEST_ASSERT(out->SetPropertyBool(STRING16(L"bool_true"), true));
  TEST_ASSERT(out->SetPropertyBool(STRING16(L"bool_false"), false));
  scoped_ptr<JsArray> bool_array(js_runner->NewArray());
  TEST_ASSERT(bool_array.get());
  TEST_ASSERT(bool_array->SetElementBool(0, true));
  TEST_ASSERT(bool_array->SetElementBool(1, false));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"bool_array"), bool_array.get()));
}

void CreateObjectInt(JsCallContext* context,
                     JsRunnerInterface* js_runner,
                     JsObject* out) {
  TEST_ASSERT(out->SetPropertyInt(STRING16(L"int_0"), 0));
  TEST_ASSERT(out->SetPropertyInt(STRING16(L"int_1"), 1));
  TEST_ASSERT(out->SetPropertyInt(STRING16(L"int_large"), int_large));
  TEST_ASSERT(out->SetPropertyInt(STRING16(L"int_negative_1"), -1));
  TEST_ASSERT(out->SetPropertyInt(STRING16(L"int_negative_large"),
                                  -int_large));
  scoped_ptr<JsArray> int_array(js_runner->NewArray());
  TEST_ASSERT(int_array.get());
  TEST_ASSERT(int_array->SetElementInt(0, 0));
  TEST_ASSERT(int_array->SetElementInt(1, 1));
  TEST_ASSERT(int_array->SetElementInt(2, int_large));
  TEST_ASSERT(int_array->SetElementInt(3, -1));
  TEST_ASSERT(int_array->SetElementInt(4, -int_large));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"int_array"), int_array.get()));
}

void CreateObjectDouble(JsCallContext* context,
                        JsRunnerInterface* js_runner,
                        JsObject* out) {
  TEST_ASSERT(out->SetPropertyDouble(STRING16(L"double_0"), 0.01));
  TEST_ASSERT(out->SetPropertyDouble(STRING16(L"double_1"), 1.01));
  TEST_ASSERT(out->SetPropertyDouble(STRING16(L"double_large"),
                                     std::numeric_limits<double>::max()));
  TEST_ASSERT(out->SetPropertyDouble(STRING16(L"double_negative_1"), -1.01));
  TEST_ASSERT(out->SetPropertyDouble(STRING16(L"double_negative_large"),
                                     JS_NUMBER_MIN_VALUE));
  scoped_ptr<JsArray> double_array(js_runner->NewArray());
  TEST_ASSERT(double_array.get());
  TEST_ASSERT(double_array->SetElementDouble(0, 0.01));
  TEST_ASSERT(double_array->SetElementDouble(1, 1.01));
  TEST_ASSERT(double_array->SetElementDouble(2,
      std::numeric_limits<double>::max()));
  TEST_ASSERT(double_array->SetElementDouble(3, -1.01));
  TEST_ASSERT(double_array->SetElementDouble(4, JS_NUMBER_MIN_VALUE));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"double_array"),
                                    double_array.get()));
}

void CreateObjectString(JsCallContext* context,
                        JsRunnerInterface* js_runner,
                        JsObject* out) {
  std::string16 string_0;
  std::string16 string_1(STRING16(L"a"));
  std::string16 string_many(
    STRING16(L"asdjh1)!(@#*h38ind89!03234bnmd831%%%*&*jdlwnd8893asd1233"));
  TEST_ASSERT(out->SetPropertyString(STRING16(L"string_0"), string_0));
  TEST_ASSERT(out->SetPropertyString(STRING16(L"string_1"), string_1));
  TEST_ASSERT(out->SetPropertyString(STRING16(L"string_many"), string_many));
  scoped_ptr<JsArray> string_array(js_runner->NewArray());
  TEST_ASSERT(string_array.get());
  TEST_ASSERT(string_array->SetElementString(0, string_0));
  TEST_ASSERT(string_array->SetElementString(1, string_1));
  TEST_ASSERT(string_array->SetElementString(2, string_many));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"string_array"),
                                    string_array.get()));
}

static bool FillTestArray(const int n, JsArray* arr) {
  for (int i = 0; i < n; ++i)
    if (!arr->SetElementInt(i, i)) return false;
  return true;
}

void CreateObjectArray(JsCallContext* context,
                       JsRunnerInterface* js_runner,
                       JsRootedCallback* func,
                       JsObject* out) {
  scoped_ptr<JsArray> array_0(js_runner->NewArray());
  TEST_ASSERT(array_0.get());
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"array_0"), array_0.get()));

  scoped_ptr<JsArray> array_1(js_runner->NewArray());
  TEST_ASSERT(array_1.get());
  TEST_ASSERT(FillTestArray(1, array_1.get()));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"array_1"), array_1.get()));

  scoped_ptr<JsArray> array_8(js_runner->NewArray());
  TEST_ASSERT(array_8.get());
  TEST_ASSERT(FillTestArray(8, array_8.get()));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"array_8"), array_8.get()));

  scoped_ptr<JsArray> array_10000(js_runner->NewArray());
  TEST_ASSERT(array_10000.get());
  TEST_ASSERT(FillTestArray(10000, array_10000.get()));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"array_10000"),
                                    array_10000.get()));

  scoped_ptr<JsArray> array_many_types(js_runner->NewArray());
  TEST_ASSERT(array_many_types.get());
  TEST_ASSERT(array_many_types->SetElementBool(0, false));
  TEST_ASSERT(array_many_types->SetElementBool(1, true));
  TEST_ASSERT(array_many_types->SetElementInt(2, 2));
  TEST_ASSERT(array_many_types->SetElementString(3, STRING16(L"3")));
  TEST_ASSERT(array_many_types->SetElementDouble(4, 4.01));
  scoped_ptr<JsArray> array_5(js_runner->NewArray());
  TEST_ASSERT(array_5.get());
  TEST_ASSERT(FillTestArray(5, array_5.get()));
  TEST_ASSERT(array_many_types->SetElementArray(5, array_5.get()));
  TEST_ASSERT(array_many_types->SetElementFunction(6, func));
  TEST_ASSERT(out->SetPropertyArray(STRING16(L"array_many_types"),
                                    array_many_types.get()));
}

void CreateObjectObject(JsCallContext* context,
                        JsRunnerInterface* js_runner,
                        JsObject* out) {
  scoped_ptr<JsObject> obj(js_runner->NewObject());
  TEST_ASSERT(obj.get());
  TEST_ASSERT(obj->SetPropertyBool(STRING16(L"bool_true"), true));
  TEST_ASSERT(obj->SetPropertyInt(STRING16(L"int_0"), 0));
  TEST_ASSERT(obj->SetPropertyDouble(STRING16(L"double_0"), 0.01));
  TEST_ASSERT(obj->SetPropertyString(STRING16(L"string_0"), STRING16(L"")));
  scoped_ptr<JsArray> array_0(js_runner->NewArray());
  TEST_ASSERT(array_0.get());
  TEST_ASSERT(obj->SetPropertyArray(STRING16(L"array_0"), array_0.get()));
  TEST_ASSERT(out->SetPropertyObject(STRING16(L"obj"), obj.get()));
}

void CreateObjectDate(JsCallContext* context,
                      JsRunnerInterface* js_runner,
                      JsObject* out) {
  assert(out);
  scoped_ptr<JsObject> obj(js_runner->NewDate(10));
  TEST_ASSERT(obj.get());
  TEST_ASSERT(out->SetPropertyObject(STRING16(L"date_object"), obj.get()));
}

void CreateObjectFunction(JsCallContext* context,
                          JsRootedCallback* func,
                          JsObject* out) {
  TEST_ASSERT(out->SetPropertyFunction(STRING16(L"func"), func));
}

#ifdef WINCE
// These methods are used by the JavaScript testBrowserCache test.

bool GetJsArrayAsStringVector(const JsArray &js_array,
                              std::vector<std::string16> *strings) {
  int array_size;
  if (!js_array.GetLength(&array_size)) {
    return false;
  }
  std::string16 url;
  for (int i = 0; i < array_size; ++i) {
    js_array.GetElementAsString(i, &url);
    strings->push_back(url);
  }
  return true;
}

void GearsTest::RemoveEntriesFromBrowserCache(JsCallContext *context) {
  bool ok = false;
  context->SetReturnValue(JSPARAM_BOOL, &ok);
  JsArray js_array;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_ARRAY, &js_array }
  };
  if (context->GetArguments(ARRAYSIZE(argv), argv) != ARRAYSIZE(argv)) {
    assert(context->is_exception_set());
    return;
  }
  std::vector<std::string16> urls;
  if (!GetJsArrayAsStringVector(js_array, &urls)) {
    context->SetException(STRING16(L"Failed to get urls."));
    return;
  }
  for (int i = 0; i < static_cast<int>(urls.size()); ++i) {
    std::string16 full_url;
    if (!ResolveAndNormalize(
             EnvPageLocationUrl().c_str(), urls[i].c_str(), &full_url)) {
      context->SetException(STRING16(L"Failed to resolve URL ") + urls[i]);
      return;
    }
    scoped_array<INTERNET_CACHE_ENTRY_INFO> info(
        GetEntryInfoTest(full_url.c_str()));
    if (info.get()) {
      if (DeleteUrlCacheEntry(full_url.c_str()) == FALSE) {
        context->SetException(
            STRING16(L"Failed to remove browser cache entry for ") +
            full_url +
            STRING16(L"."));
        return;
      }
    }
  }
  ok = true;
  context->SetReturnValue(JSPARAM_BOOL, &ok);
}

#undef TEST_ASSERT
#define TEST_ASSERT(test, message) \
{ \
  if (!(test)) { \
    context->SetException(message); \
    return; \
  } \
}

void GearsTest::TestEntriesPresentInBrowserCache(JsCallContext *context) {
  bool ok = false;
  context->SetReturnValue(JSPARAM_BOOL, &ok);
  JsArray js_array;
  bool should_be_present;
  bool should_be_bogus = true;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_ARRAY, &js_array },
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &should_be_present },
    { JSPARAM_OPTIONAL, JSPARAM_BOOL, &should_be_bogus }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  std::vector<std::string16> urls;
  if (!GetJsArrayAsStringVector(js_array, &urls)) {
    context->SetException(STRING16(L"Failed to get urls."));
    return;
  }
  for (int i = 0; i < static_cast<int>(urls.size()); ++i) {
    std::string16 full_url;
    if (!ResolveAndNormalize(
             EnvPageLocationUrl().c_str(), urls[i].c_str(), &full_url)) {
      context->SetException(STRING16(L"Failed to resolve URL ") + urls[i]);
      return;
    }
    scoped_array<INTERNET_CACHE_ENTRY_INFO> info(
        GetEntryInfoTest(full_url.c_str()));
    if (should_be_present) {
      TEST_ASSERT(info.get(),
                  STRING16(L"No browser cache entry for ") +
                  full_url +
                  STRING16(L"."));
      bool is_bogus = IsEntryBogusTest(info.get());
      TEST_ASSERT(!(should_be_bogus && !is_bogus),
                  STRING16(L"Browser cache entry for ") +
                  full_url +
                  STRING16(L" should be bogus but is not."));
      TEST_ASSERT(!(!should_be_bogus && is_bogus),
                  STRING16(L"Browser cache entry for ") +
                  full_url +
                  STRING16(L" should not be bogus but is."));
    } else {
      TEST_ASSERT(!info.get(),
                  STRING16(L"Spurious browser cache entry for ") +
                  full_url +
                  STRING16(L"."));
    }
  }
  ok = true;
  context->SetReturnValue(JSPARAM_BOOL, &ok);
}
#endif

void GearsTest::TestParseGeolocationOptions(JsCallContext *context) {
  ::TestParseGeolocationOptions(context, GetJsRunner());
}

void GearsTest::TestGeolocationFormRequestBody(JsCallContext *context) {
  ::TestGeolocationFormRequestBody(context);
}

void GearsTest::TestGeolocationGetLocationFromResponse(JsCallContext *context) {
  ::TestGeolocationGetLocationFromResponse(context, GetJsRunner());
}

void GearsTest::ConfigureGeolocationRadioDataProviderForTest(
    JsCallContext *context) {
  ::ConfigureGeolocationRadioDataProviderForTest(context);
}

void GearsTest::ConfigureGeolocationWifiDataProviderForTest(
    JsCallContext *context) {
  ::ConfigureGeolocationWifiDataProviderForTest(context);
}

void GearsTest::ConfigureGeolocationMockLocationProviderForTest(
    JsCallContext *context) {
  ::ConfigureGeolocationMockLocationProviderForTest(context);
}

void GearsTest::RemoveGeolocationMockLocationProvider(
    JsCallContext * /* context */) {
  ::RemoveGeolocationMockLocationProvider();
}

#ifdef OFFICIAL_BUILD
// The Audio API has not been finalized for official builds.
#else
void GearsTest::ConfigureAudioRecorderForTest(JsCallContext *context) {
  ::ConfigureAudioRecorderForTest(context);
}
#endif

void GearsTest::CreateBlobFromString(JsCallContext *context) {
  std::string16 input_utf16;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &input_utf16 }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  std::string input_utf8;
  String16ToUTF8(input_utf16.c_str(), &input_utf8);
  scoped_refptr<GearsBlob> gears_blob;
  if (!CreateModule<GearsBlob>(module_environment_.get(),
                               context, &gears_blob)) {
    return;
  }
  gears_blob->Reset(new BufferBlob(input_utf8.c_str(), input_utf8.size()));
  context->SetReturnValue(JSPARAM_MODULE, gears_blob.get());
}


#ifdef OFFICIAL_BUILD
// The notification API has not been finalized for official builds.
#else

#if (defined(WIN32) && !defined(WINCE)) || defined(LINUX)
class ProcessCreator {
 public:
  static ProcessCreator* Create(const char16 *full_filepath);

  // Returns the exit code from the process, but only waits
  // at most wait_seconds for it to finish.  If the process
  // hasn't exited, then it returns -1;
  int GetExitCode(int wait_seconds);

 private:
#if defined(WIN32)
  explicit ProcessCreator(HANDLE process) : process_(process) {}
#else
  explicit ProcessCreator(pid_t pid) : child_process_pid_(pid) {}
#endif  // WIN32


#if defined(WIN32)
  HANDLE process_;
#else
  pid_t child_process_pid_;
#endif  // WIN32
  DISALLOW_EVIL_CONSTRUCTORS(ProcessCreator);
};

#if defined(WIN32)
ProcessCreator* ProcessCreator::Create(const char16 *full_filepath) {
  STARTUPINFO startup_info = {0};
  startup_info.cb = sizeof(STARTUPINFO);
  PROCESS_INFORMATION process_information = {0};
  if (!::CreateProcess(full_filepath,
                       NULL,
                       NULL,
                       NULL,
                       false,
                       CREATE_NO_WINDOW,
                       NULL,
                       NULL,
                       &startup_info,
                       &process_information)) {
    return NULL;
  }
  ::CloseHandle(process_information.hThread);
  return new ProcessCreator(process_information.hProcess);
}

int ProcessCreator::GetExitCode(int wait_seconds) {
  // Ensure that the process exited correctly and didn't timeout.
  if (::WaitForSingleObject(process_, wait_seconds * 1000) != WAIT_OBJECT_0) {
    return -1;
  }

  DWORD exit_code = -1;
  if (!::GetExitCodeProcess(process_, &exit_code)) {
    return -1;
  }
  return exit_code;
}
#else
ProcessCreator* ProcessCreator::Create(const char16 *full_filepath) {
  pid_t pid = fork();
  if (pid < 0) {
    return NULL;
  }
  if (pid == 0) {
    std::string file;
    if (!String16ToUTF8(full_filepath, &file)) {
      return NULL;
    }
    execl(file.c_str(), "notifier_test", NULL);
    exit(-1);
  }
  return new ProcessCreator(pid);
}

int ProcessCreator::GetExitCode(int wait_seconds) {
  // Set up a signal to interrupt waitpid from waiting forever.
  alarm(wait_seconds);
  int exit_code = -1;
  if (waitpid(child_process_pid_, &exit_code, WUNTRACED) == -1) {
    exit_code = -1;
  }
  // Cancel the alarm.
  alarm(0);
  return exit_code;
}
#endif  // WIN32
#endif  // (defined(WIN32) && !defined(WINCE)) || defined(LINUX)

#if defined(WIN32)
const char16* kNotifierTestApp = STRING16(L"notifier_test.exe");
#else
const char16* kNotifierTestApp = STRING16(L"notifier_test");
#endif  // WIN32

void GearsTest::TestNotifier(JsCallContext *context) {
#if defined(WIN32) && !defined(WINCE)
  std::string16 component_directory;
  if (!GetComponentDirectory(&component_directory)) {
    context->SetException(STRING16(L"Couldn't get install directory."));
    return;
  }

  // Find the notifier_test application.
  std::string16 notifier_test(component_directory);
  AppendName(kNotifierTestApp, &notifier_test);
  if (!File::Exists(notifier_test.c_str())) {
    notifier_test.assign(component_directory);
    RemoveName(&notifier_test);
    AppendName(STRING16(L"common"), &notifier_test);
    AppendName(kNotifierTestApp, &notifier_test);
  }
  if (!File::Exists(notifier_test.c_str())) {
    context->SetException(STRING16(L"Couldn't find notifier_test."));
    return;
  }

  scoped_ptr<ProcessCreator> process(
      ProcessCreator::Create(notifier_test.c_str()));
  if (!process.get()) {
    context->SetException(STRING16(L"Couldn't start notifier_test."));
    return;
  }
  if (process->GetExitCode(5 * 60) != 0) {
    context->SetException(STRING16(L"notifier_test failed.  Run "
                                   L"notifier_test.exe for more information."));
    return;
  }
#endif  // defined(WIN32) && !defined(WINCE)

}
#endif  // OFFICIAL_BUILD

#endif  // USING_CCTESTS
