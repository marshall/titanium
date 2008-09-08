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

#ifdef USING_CCTESTS

#ifndef GEARS_CCTESTS_TEST_H__
#define GEARS_CCTESTS_TEST_H__

#ifdef WINCE
#include <windows.h>
#include <wininet.h>  // For INTERNET_CACHE_ENTRY_INFO
#endif
#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/geolocation/geolocation.h"

class GearsTest : public ModuleImplBaseClass {
 public:
  GearsTest()
    : ModuleImplBaseClass("GearsTest"),
      start_ticks_(0) {}

  // IN: nothing
  // OUT: int64 time
  void GetSystemTime(JsCallContext *context);

  // IN: nothing
  // OUT: nothing
  void StartPerfTimer(JsCallContext *context);

  // IN: nothing
  // OUT: int64 elapsed_microseconds
  void StopPerfTimer(JsCallContext *context);

  // IN: bool is_worker
  // OUT: nothing
  void RunTests(JsCallContext *context);


  // JsObject and JsArray tests

  // IN: object value
  // OUT: void
  // throws exception on failure
  void TestPassObject(JsCallContext *context);

  // IN: function test_function
  // OUT: object created_object
  // throws exception on failure
  void TestCreateObject(JsCallContext *context);

  // IN: void
  // OUT: object created_object
  // throws exception on failure
  void TestCreateError(JsCallContext *context);

  // IN: nothing
  // OUT: nothing
  // throws exception on failure
  void TestObjectProperties(JsCallContext *context);

  // Argument passing tests

  // IN: bool bool_value, int int_value, int64 int64_value, double double_value,
  //     string string_value
  // OUT: void
  void TestPassArguments(JsCallContext *context);

  // IN: function
  // OUT: bool bool_value, int int_value, int64 int64_value,
  //      double double_value, string string_value
  void TestPassArgumentsCallback(JsCallContext *context);

  // IN: int value1, optional int value2, optional int value3
  // OUT: void
  void TestPassArgumentsOptional(JsCallContext *context);

  // Coercion tests

  // IN: variant value, bool expected_value
  // OUT: bool
  void TestCoerceBool(JsCallContext *context);
  // IN: variant value, int expected_value
  // OUT: bool
  void TestCoerceInt(JsCallContext *context);
  // IN: variant value, double expected_value
  // OUT: bool
  void TestCoerceDouble(JsCallContext *context);
  // IN: variant value, string expected_value
  // OUT: bool
  void TestCoerceString(JsCallContext *context);
  // IN: string type, variant value
  // OUT: bool
  void TestGetType(JsCallContext *context);

#ifdef WINCE
  // These methods are used by the JavaScript testBrowserCache test.

  // IN: variant urls
  // OUT: bool
  void RemoveEntriesFromBrowserCache(JsCallContext *context);
  // IN: variant urls, bool should_be_present, bool should_be_bogus
  // OUT: bool
  void TestEntriesPresentInBrowserCache(JsCallContext *context);
#endif

  // Geolocation internal tests.

  // IN: object position_options
  // OUT: object parsed_options
  void TestParseGeolocationOptions(JsCallContext *context);

  // IN: nothing
  // OUT: string request_body
  void TestGeolocationFormRequestBody(JsCallContext *context);

  // IN: string response_body
  // OUT: object position
  void TestGeolocationGetLocationFromResponse(JsCallContext *context);

  // Configures the radio data provider factory to use a mock radio device data
  // provider and sets the data that the mock provider will provide. Note that
  // we only support data for one cell. Fields are cell_id, location_area_code,
  // mobile_network_code, mobile_country_code, age, radio_signal_strength,
  // timing_advance, device_id, home_mobile_network_code,
  // home_mobile_country_code, radio_type and carrier.
  // IN: object radio_data
  // OUT: nothing
  void ConfigureGeolocationRadioDataProviderForTest(JsCallContext *context);

  // Configures the wifi data provider factory to use a mock wifi device data
  // provider and sets the that the mock provider will provide. Note that
  // we only support data for one access_point. Fields are mac_address,
  // radio_signal_strength, age, channel, signal_to_noise and ssid.
  // IN: object wifi_data
  // OUT: nothing
  void ConfigureGeolocationWifiDataProviderForTest(JsCallContext *context);

  // Configures the location provider pool to use a mock location provider. Sets
  // the position that the mock provider will provide. Fields are latitude,
  // longitude, altitude, horizontalAccuracy, verticalAccuracy and error. Note
  // that this a subset of the properties available on the Position object
  // returned by the Geolocation module.
  // IN: object position.
  // OUT: nothing
  void ConfigureGeolocationMockLocationProviderForTest(JsCallContext *context);

  // Configures the location provider pool to not use the mock location
  // provider.
  void RemoveGeolocationMockLocationProvider(JsCallContext *context);

#ifdef OFFICIAL_BUILD
  // The Audio API has not been finalized for official builds.
#else
  // Audio internal tests.

  // Sets the (audio recorder) device factories to use mock device.
  // IN: nothing
  // OUT: nothing
  void ConfigureAudioRecorderForTest(JsCallContext *context);
#endif

  // IN: string input
  // OUT: GearsBlob
  // The resultant Blob's contents will the input string in UTF-8 format.
  void CreateBlobFromString(JsCallContext *context);

  // IN: optional int numOrigs
  //     optional int numStoresPerOrigin
  //     optional int numItemsPerStore
  // OUT: string, timing results
  void TestLocalServerPerformance(JsCallContext *context);

#ifdef OFFICIAL_BUILD
  // The Notification API has not been finalized for official builds.
#else
  // Notification internal tests.

  // IN: nothing
  // OUT: nothing
  void TestNotifier(JsCallContext *context);
#endif  // OFFICIAL_BUILD

 private:
  // The tick count used by the perf timer.
  int64 start_ticks_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsTest);
};

#ifdef WINCE
// These functions are declared in wince_compatibility.cc. They are wrappers
// around static functions defined there.
INTERNET_CACHE_ENTRY_INFO* GetEntryInfoTest(const char16 *url);
bool IsEntryBogusTest(INTERNET_CACHE_ENTRY_INFO *info);
#endif

#endif  // GEARS_CCTESTS_TEST_H__

#endif  // USING_CCTESTS
