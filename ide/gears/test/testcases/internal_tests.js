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

if (isUsingCCTests) {
  var internalTests = google.gears.factory.create('beta.test');
}

function testInternal() {
  if (isUsingCCTests) {
    internalTests.runTests(google.gears.workerPool ? true : false);
  }
}


// test functions for passing arguments
function testPassArguments() {
  if (isUsingCCTests) {
    // Have to do this because Gears methods don't support apply() in IE.
    function callTest(args) {
      internalTests.testPassArguments(args[0], args[1], args[2], args[3],
                                      args[4]);
    }
    
    var good_bool = true;
    var good_int = 42;
    var good_int64 = Math.pow(2, 42);
    var good_double = 88.8;
    var good_string = "hotdog";
    
    var bad_bool = false;
    var bad_int = 43;
    var bad_int64 = Math.pow(2, 43);
    var bad_double = 88.9;
    var bad_string = "hotdof";
    
    // Test a good one
    var good_vals = [good_bool, good_int, good_int64, good_double, good_string];
    callTest(good_vals);
    
    // Test passing wrong values
    var bad_vals = [bad_bool, bad_int, bad_int64, bad_double, bad_string];
    for (var i = 0; i < good_vals.length; i++) {
      var test_vals = [].concat(good_vals);
      test_vals[i] = bad_vals[i];
      assertError(function() { callTest(test_vals) },
                  "Incorrect value for parameter " + (i + 1));
    }

    // Test not passing required args
    var required_argument_error = "Required argument 5 is missing.";
    assertError(function() {
      internalTests.testPassArguments(good_bool, good_int, good_int64,
                                      good_double, null); },
      required_argument_error);
    assertError(function() {
      internalTests.testPassArguments(good_bool, good_int, good_int64,
                                      good_double, undefined); },
      required_argument_error);
    assertError(function() {
      internalTests.testPassArguments(good_bool, good_int, good_int64,
                                      good_double); },
      required_argument_error);

    // Test passing wrong type
    for (var i = 0; i < good_vals.length; i++) {
      for (var j = 0; j < good_vals.length; j++) {
        if (i == j) continue;

        var expect_conversion_allowed = true;

        // We implicitly convert int to int64, so we don't expect errors in
        // that case.
        if (good_vals[i] == good_int64 && good_vals[j] == good_int) {
          expect_conversion_allowed = false;
        }
        
        // Same for converting int and int64 to double.
        if (good_vals[i] == good_double) {
          if (good_vals[j] == good_int || good_vals[j] == good_int64) {
            expect_conversion_allowed = false;
          }
        }
        
        var test_vals = [].concat(good_vals);
        test_vals[i] = test_vals[j];

        var expected_error = expect_conversion_allowed ?
            "Argument " + (i + 1) + " has invalid type or is outside allowed " +
            "range" :
            "Incorrect value for parameter " + (i + 1);
        
        assertError(function() { callTest(test_vals); }, expected_error);
      }
    }
  }
}

function testPassArgumentsCallback() {
  function testPassArgumentCallbackFunction(arg1, arg2, arg3, arg4, arg5) {
    assert(arg1 === true, "Bad value for argument 1");
    assert(arg2 === 42, "Bad value for argument 2");
    assert(arg3 === Math.pow(2, 42), "Bad value for argument 3");
    assert(arg4 === 88.8, "Bad value for argument 4");
    assert(arg5 === "hotdog", "Bad value for argument 5");
  }
  if (isUsingCCTests) {
    internalTests.testPassArgumentsCallback(testPassArgumentCallbackFunction);
  }
}

function testPassArgumentsOptional() {
  if (isUsingCCTests) {
    internalTests.testPassArgumentsOptional(42, 42, 42);
    internalTests.testPassArgumentsOptional(42, 42);
    internalTests.testPassArgumentsOptional(42);
    
    assertError(
      function() { internalTests.testPassArgumentsOptional(42, "hello"); },
      "Argument 2 has invalid type or is outside allowed range");
      
    assertError(
      function() { internalTests.testPassArgumentsOptional(42, 43); },
      "Incorrect value for parameter 2");
  }
}



// JsObject test functions

// Test passing an object with different types of properties to C++.
function testObjectProperties() {
  if (isUsingCCTests) {
    internalTests.testObjectProperties();
  }
}

function testPassObject() {
  if (isUsingCCTests) {
    internalTests.testPassObject(new TestObject());
  }
}

// Test creating a javascript object in C++ and returning it.
function testCreateObject() {
  if (isUsingCCTests) {
    // TODO(cdevries): Enable this test on FF in worker pools when
    //                 SetReturnValue() has been implemented.
    var isFirefox = google.gears.factory.getBuildInfo().indexOf("firefox") > -1;
    var isFirefoxWorker = isFirefox && google.gears.workerPool;

    if (!isFirefoxWorker) {
      var testObject = new TestObject();
      var createdObject = internalTests.testCreateObject(createTestFunction());
      // assertObjectEqual does not compare function results
      assertObjectEqual(testObject, createdObject);
      assertEqual(testObject.array_many_types[6](),
          createdObject.array_many_types[6]());
      assertEqual(testObject.func(), createdObject.func());
      // Explicitly test date object.
      assertEqual(testObject.date_object.getTime(),
                  createdObject.date_object.getTime());
    }
  }
}

// Test creating a JavaScript Error object in C++ and returning it.
function testCreateError() {
  if (isUsingCCTests) {
    // TODO(cdevries): Enable this test on FF in worker pools when
    //                 SetReturnValue() has been implemented.
    var isFirefox = google.gears.factory.getBuildInfo().indexOf("firefox") > -1;
    var isFirefoxWorker = isFirefox && google.gears.workerPool;

    if (!isFirefoxWorker) {
      var createdObject = internalTests.testCreateError();
      assertEqual('test error\r\nwith \'special\' \\characters\\',
                  createdObject.message);
    }
  }
}

// Tests the bogus browser cache entries used on WinCE to prevent the 'Cannot
// Connect' popup when serving content from the LocalServer when offline.
// TODO(steveblock): Add more tests once test harness is fully working for
// WinCE.
// - Managed resource store.
// - Test that existing browser cache entires are not over-written.
function testBrowserCache() {
  if (isUsingCCTests && isWince) {
    // Clear the browser cache.
    var urls = ['/testcases/test_file_1.txt',
                '/testcases/test_file_1024.txt'];
    internalTests.removeEntriesFromBrowserCache(urls);
    // Create a resource store and capture some resources.
    var localServer = google.gears.factory.create('beta.localserver');
    var storeName = 'testBrowserCacheStore';
    var store = localServer.createStore(storeName);
    var responses = 0;
    startAsync();
    store.capture(urls, function(url, success, captureId) {
      ++responses;
      assert(success, 'Capture for ' + url + ' failed.');
      if (responses == urls.length) {
        // Check that these resources are in the browser cache.
        internalTests.testEntriesPresentInBrowserCache(urls, true, true);
        // Remove a URL.
        store.remove(urls[0]);
        internalTests.testEntriesPresentInBrowserCache([urls[0]], false);
        internalTests.testEntriesPresentInBrowserCache([urls[1]], true, true);
        // Remove the store.
        localServer.removeStore(storeName);
        internalTests.testEntriesPresentInBrowserCache(urls, false);
        completeAsync();
      }
    });
  }
}

// Tests the parsing of the options passed to Geolocation.GetCurrentPosition and
// Geolocation.WatchPosition.
function testParseGeolocationOptions() {
  // TODO(steveblock): Some of these tests do not require the test object so
  // should be moved to geolocation_tests.js.
  if (isUsingCCTests) {
    var dummyFunction = function() {};

    // All good.
    internalTests.testParseGeolocationOptions(dummyFunction);
    internalTests.testParseGeolocationOptions(dummyFunction, dummyFunction);
    internalTests.testParseGeolocationOptions(dummyFunction, null);
    internalTests.testParseGeolocationOptions(dummyFunction, null, {});
    internalTests.testParseGeolocationOptions(dummyFunction, dummyFunction, {});

    // Test correct types.
    // Missing success callback.
    assertError(function() { internalTests.testParseGeolocationOptions() });
    // Wrong type for success callback.
    assertError(function() { internalTests.testParseGeolocationOptions(42) });
    // Wrong type for error callback.
    assertError(function() {
      internalTests.testParseGeolocationOptions(dummyFunction, 42) });
    // Wrong type for options.
    assertError(function() {
      internalTests.testParseGeolocationOptions(dummyFunction,
                                                dummyFunction,
                                                42) });
    // Wrong type for enableHighAccuracy.
    assertError(function() {
      internalTests.testParseGeolocationOptions(dummyFunction,
                                                dummyFunction,
                                                { enableHighAccuracy: 42 }) },
      'options.enableHighAccuracy should be a boolean.');
    // Wrong type for gearsRequestAddress.
    assertError(function() {
      internalTests.testParseGeolocationOptions(dummyFunction,
                                                dummyFunction,
                                                { gearsRequestAddress: 42 }) },
      'options.gearsRequestAddress should be a boolean.');
    // Wrong type for gearsAddressLanguage.
    assertError(function() {
      internalTests.testParseGeolocationOptions(dummyFunction,
                                                dummyFunction,
                                                { gearsAddressLanguage: 42 }) },
      'options.gearsAddressLanguage should be a string.');
    // Wrong type for gearsLocationProviderUrls.
    assertError(function() {
      internalTests.testParseGeolocationOptions(
        dummyFunction,
        dummyFunction,
        { gearsLocationProviderUrls: 42 }) },
      'options.gearsLocationProviderUrls should be null or an array of ' +
      'strings');
    assertError(function() {
      internalTests.testParseGeolocationOptions(
        dummyFunction,
        dummyFunction,
        { gearsLocationProviderUrls: [42] }) },
      'options.gearsLocationProviderUrls should be null or an array of ' +
      'strings');

    // Test correct parsing.
    var defaultUrlArray = ['http://www.google.com/loc/json'];
    var urls = ['url1', 'url2'];
    var parsedOptions;
    // No options.
    parsedOptions = internalTests.testParseGeolocationOptions(dummyFunction);
    assertEqual(false, parsedOptions.repeats);
    assertEqual(false, parsedOptions.enableHighAccuracy);
    assertEqual(false, parsedOptions.gearsRequestAddress);
    assertEqual("", parsedOptions.gearsAddressLanguage);
    assertArrayEqual(defaultUrlArray, parsedOptions.gearsLocationProviderUrls);
    // Empty options.
    parsedOptions = internalTests.testParseGeolocationOptions(dummyFunction,
                                                              dummyFunction,
                                                              {});
    assertEqual(false, parsedOptions.repeats);
    assertEqual(false, parsedOptions.enableHighAccuracy);
    assertEqual(false, parsedOptions.gearsRequestAddress);
    assertEqual("", parsedOptions.gearsAddressLanguage);
    assertArrayEqual(defaultUrlArray, parsedOptions.gearsLocationProviderUrls);
    // Empty provider URLs.
    parsedOptions = internalTests.testParseGeolocationOptions(
        dummyFunction,
        dummyFunction,
        { gearsLocationProviderUrls: [] });
    assertEqual(false, parsedOptions.repeats);
    assertEqual(false, parsedOptions.enableHighAccuracy);
    assertEqual(false, parsedOptions.gearsRequestAddress);
    assertEqual("", parsedOptions.gearsAddressLanguage);
    assertArrayEqual([], parsedOptions.gearsLocationProviderUrls);
    // Null provider URLs.
    parsedOptions = internalTests.testParseGeolocationOptions(
        dummyFunction,
        dummyFunction,
        { gearsLocationProviderUrls: null });
    assertEqual(false, parsedOptions.repeats);
    assertEqual(false, parsedOptions.enableHighAccuracy);
    assertEqual(false, parsedOptions.gearsRequestAddress);
    assertEqual("", parsedOptions.gearsAddressLanguage);
    assertArrayEqual([], parsedOptions.gearsLocationProviderUrls);
    // All properties false, provider URLs set.
    parsedOptions = internalTests.testParseGeolocationOptions(
        dummyFunction,
        dummyFunction,
        { enableHighAccuracy: false,
          gearsRequestAddress: false,
          gearsAddressLanguage: "",
          gearsLocationProviderUrls: urls
        });
    assertEqual(false, parsedOptions.repeats);
    assertEqual(false, parsedOptions.enableHighAccuracy);
    assertEqual(false, parsedOptions.gearsRequestAddress);
    assertEqual("", parsedOptions.gearsAddressLanguage);
    assertArrayEqual(urls, parsedOptions.gearsLocationProviderUrls);
    // All properties true, provider URLs set.
    parsedOptions = internalTests.testParseGeolocationOptions(
      dummyFunction,
      dummyFunction,
      { enableHighAccuracy: true,
        gearsRequestAddress: true,
        gearsAddressLanguage: "test",
        gearsLocationProviderUrls: urls
      });
    assertEqual(false, parsedOptions.repeats);
    assertEqual(true, parsedOptions.enableHighAccuracy);
    assertEqual(true, parsedOptions.gearsRequestAddress);
    assertEqual("test", parsedOptions.gearsAddressLanguage);
    assertArrayEqual(urls, parsedOptions.gearsLocationProviderUrls);
  }
}

// Tests forming the JSON request body for a network location provider. Note
// that the inputs to the conversion are set on the C++ side.
function testGeolocationFormRequestBody() {
  if (isUsingCCTests) {
    var body = internalTests.testGeolocationFormRequestBody();
    var correctBody = '{ ' +
                      '"address_language" : "en-GB", ' +
                      '"cell_towers" : [ { ' +
                      '"cell_id" : 23874, ' +
                      '"location_area_code" : 98, ' +
                      '"mobile_country_code" : 234, ' +
                      '"mobile_network_code" : 15, ' +
                      '"signal_strength" : -65 ' +
                      '} ], ' +
                      '"host" : "www.google.com", ' +
                      '"location" : { ' +
                      '"latitude" : 53.1, ' +
                      '"longitude" : -0.1 ' +
                      '}, ' +
                      '"radio_type" : "gsm", ' +
                      '"request_address" : true, ' +
                      '"version" : "1.0.1", ' +
                      '"wifi_towers" : [ { ' +
                      '"age" : 15, ' +
                      '"channel" : 19, ' +
                      '"mac_address" : "00-0b-86-d7-6a-42", ' +
                      '"signal_strength" : -50, ' +
                      '"signal_to_noise" : 10, ' +
                      '"ssid" : "Test SSID" ' +
                      '} ] ' +
                      '}\n';  // Note trailing line break.
    assertEqual(correctBody, body);
  }
}

// Tests extracting a position object from the JSON reposnse from a network
// location provider.
function testGeolocationGetLocationFromResponse() {
  if (isUsingCCTests) {
    var dummy_server = 'http://test.server.com';
    var position;
    var correctPosition;

    var locationAcquisitionErrorCode = 2;
    var locationNotFoundErrorCode = 3;

    // Test good response with valid position.
    var responseBody = '{ ' +
                       '"location" : { ' +
                       '"latitude" : 53.1, ' +
                       '"longitude" : -0.1, ' +
                       '"altitude" : 30.1, ' +
                       '"horizontal_accuracy" : 1200.1, ' +
                       '"vertical_accuracy" : 10.1, ' +
                       '"address" : { ' +
                       '"street_number": "100", ' +
                       '"street": "Amphibian Walkway", ' +
                       '"city": "Mountain View", ' +
                       '"county": "Mountain View County", ' +
                       '"region": "California", ' +
                       '"country": "United States of America", ' +
                       '"country_code": "US", ' +
                       '"postal_code": "94043" ' +
                       '} ' +
                       '} ' +
                       '}';
    position = internalTests.testGeolocationGetLocationFromResponse(
        true,  // HttpPost result
        200,   // status code
        responseBody,
        42,    // timestamp
        '');   // server URL
    correctPosition = new Object();
    correctPosition.latitude = 53.1;
    correctPosition.longitude = -0.1;
    correctPosition.altitude = 30.1;
    correctPosition.accuracy = 1200.1;
    correctPosition.altitudeAccuracy = 10.1;
    correctPosition.gearsAddress = new Object();
    correctPosition.gearsAddress.streetNumber = '100';
    correctPosition.gearsAddress.street = 'Amphibian Walkway';
    correctPosition.gearsAddress.city = 'Mountain View';
    correctPosition.gearsAddress.county = 'Mountain View County';
    correctPosition.gearsAddress.region = 'California';
    correctPosition.gearsAddress.country = 'United States of America';
    correctPosition.gearsAddress.countryCode = 'US';
    correctPosition.gearsAddress.postalCode = '94043';
    correctPosition.timestamp = new Date(42);
    assertObjectEqual(correctPosition, position);

    // We should also accept integer values for floating point fields.
    var responseBody = '{ ' +
                       '"location" : { ' +
                       '"latitude" : 53, ' +
                       '"longitude" : 0, ' +
                       '"altitude" : 30, ' +
                       '"horizontal_accuracy" : 1200, ' +
                       '"vertical_accuracy" : 10, ' +
                       '"address" : { ' +
                       '"street_number": "100", ' +
                       '"street": "Amphibian Walkway", ' +
                       '"city": "Mountain View", ' +
                       '"county": "Mountain View County", ' +
                       '"region": "California", ' +
                       '"country": "United States of America", ' +
                       '"country_code": "US", ' +
                       '"postal_code": "94043" ' +
                       '} ' +
                       '} ' +
                       '}';
    position = internalTests.testGeolocationGetLocationFromResponse(
        true,  // HttpPost result
        200,   // status code
        responseBody,
        42,    // timestamp
        '');   // server URL
    correctPosition.latitude = 53;
    correctPosition.longitude = 0;
    correctPosition.altitude = 30;
    correctPosition.accuracy = 1200;
    correctPosition.altitudeAccuracy = 10;
    assertObjectEqual(correctPosition, position);

    // Test no response.
    position = internalTests.testGeolocationGetLocationFromResponse(
        false,  // HttpPost result
        0,      // status code
        '',     // response body
        0,      // timestamp
        dummy_server);
    correctPosition = new Object();
    correctPosition.code = locationAcquisitionErrorCode;
    correctPosition.message = 'No response from network provider at ' +
                              dummy_server +
                              '.';
    assertObjectEqual(correctPosition, position);

    // Test bad response.
    position = internalTests.testGeolocationGetLocationFromResponse(
        true,   // HttpPost result
        400,    // status code
        '',     // response body
        0,      // timestamp
        dummy_server);
    correctPosition = new Object();
    correctPosition.code = locationAcquisitionErrorCode;
    correctPosition.message = 'Network provider at ' +
                              dummy_server +
                              ' returned error code 400.';
    assertObjectEqual(correctPosition, position);

    // Test good response with malformed body.
    position = internalTests.testGeolocationGetLocationFromResponse(
        true,   // HttpPost result
        200,    // status code
        'malformed reposnse body',
        0,      // timestamp
        dummy_server);
    correctPosition = new Object();
    correctPosition.code = locationAcquisitionErrorCode;
    correctPosition.message = 'Response from network provider at ' +
                              dummy_server +
                              ' was malformed.';
    assertObjectEqual(correctPosition, position);

    // Test good response with empty body.
    position = internalTests.testGeolocationGetLocationFromResponse(
        true,   // HttpPost result
        200,    // status code
        '',     // response body
        0,      // timestamp
        dummy_server);
    correctPosition = new Object();
    correctPosition.code = locationAcquisitionErrorCode;
    correctPosition.message = 'Response from network provider at ' +
                              dummy_server +
                              ' was malformed.';
    assertObjectEqual(correctPosition, position);

    // Test good response with unknown position.
    position = internalTests.testGeolocationGetLocationFromResponse(
        true,   // HttpPost result
        200,    // status code
        '{}',   // response body
        0,      // timestamp
        dummy_server);
    correctPosition = new Object();
    correctPosition.code = locationNotFoundErrorCode;
    correctPosition.message = 'Network provider at ' +
                              dummy_server +
                              ' did not provide a good position fix.'
    assertObjectEqual(correctPosition, position);

    // Test good response with explicit unknown position.
    position = internalTests.testGeolocationGetLocationFromResponse(
        true,   // HttpPost result
        200,    // status code
        '{"position": null}',
        0,      // timestamp
        dummy_server);
    correctPosition = new Object();
    correctPosition.code = locationNotFoundErrorCode;
    correctPosition.message = 'Network provider at ' +
                              dummy_server +
                              ' did not provide a good position fix.'
    assertObjectEqual(correctPosition, position);
  }
}

function testGeolocationGetCurrentPosition() {
  if (isUsingCCTests) {
 
    // Use mock radio and WiFi device data providers.
    internalTests.configureGeolocationRadioDataProviderForTest(
      { cell_id: 23874,
        location_area_code: 98,
        mobile_network_code: 15,
        mobile_country_code: 234,
        radio_signal_strength: -65
      });
    internalTests.configureGeolocationWifiDataProviderForTest(
      { mac_address: "00-0b-86-ca-bb-c8" });
    // TODO(steveblock): Complete this test once we have a working, public
    // network location provider.
    //var geolocation = google.gears.factory.create('beta.geolocation');
    //var locationAvailable = function(position) {
    //  completeAsync();
    //};
    //startAsync();
    //geolocation.getCurrentPosition(locationAvailable,
    //                               function() {},
    //                               { gearsLocationProviderUrls: ['TODO'] });

    // Use mock location provider.
    var mockPosition = {
      latitude: 51.0,
      longitude: -0.1,
      accuracy: 100.1
    };
    internalTests.configureGeolocationMockLocationProviderForTest(mockPosition);
    var locationAvailable2 = function(position) {
      assertEqualAnyType(mockPosition.latitude, position.latitude);
      assertEqualAnyType(mockPosition.longitude, position.longitude);
      assertEqualAnyType(mockPosition.accuracy, position.accuracy);
      internalTests.removeGeolocationMockLocationProvider();
      completeAsync();
    };
    var geolocation2 = google.gears.factory.create('beta.geolocation');
    startAsync();
    geolocation2.getCurrentPosition(locationAvailable2,
                                    function() {},
                                    { gearsLocationProviderUrls: [] });
  }
}

function testNotifier() {
  if (isUsingCCTests && !isOfficial) {
    internalTests.testNotifier();
  }
}

// Helper functions

function createTestArray(length) {
  var array = [];
  for (var i = 0; i < length; ++i) {
    array.push(i);
  }
  return array;
}

function ChildTestObject() {
  this.bool_true = true;
  this.int_0 = 0;
  this.double_0 = 0.01;
  this.string_0 = '';
  this.array_0 = createTestArray(0);
}

function createTestFunction() {
  return function() { return 'i am a function'; };
}

function TestObject() {
  // boolean
  this.bool_true = true;
  this.bool_false = false;
  this.bool_array = [this.bool_true, this.bool_false];

  // integer (assumed to be tagged 32 bit signed integer,
  //          30 bits magnitude, 1 bit sign, 1 bit tag)
  this.int_0 = 0;
  this.int_1 = 1;
  this.int_large = 1073741823; // 2**30 - 1
  this.int_negative_1 = -1;
  this.int_negative_large = -1073741823; // -(2**30 - 1)
  this.int_array = [this.int_0, this.int_1, this.int_large,
    this.int_negative_1, this.int_negative_large];

  // double (assumed to be 64-bit IEEE double precision float point)
  this.double_0 = 0.01;  // 0.0 becomes an int
  this.double_1 = 1.01;
  this.double_large = Number.MAX_VALUE;
  this.double_negative_1 = -1.01;
  this.double_negative_large = Number.MIN_VALUE;
  this.double_array = [this.double_0, this.double_1, this.double_large,
    this.double_negative_1, this.double_negative_large];

  // string
  this.string_0 = '';
  this.string_1 = 'a';
  this.string_many = 'asdjh1)!(@#*h38ind89!03234bnmd831%%%*&*jdlwnd8893asd1233';
  this.string_array = [this.string_0, this.string_1, this.string_many];

  // array
  this.array_0 = createTestArray(0);
  this.array_1 = createTestArray(1);
  this.array_8 = createTestArray(8);
  this.array_10000 = createTestArray(10000);
  this.array_many_types = [false, true, 2, '3', 4.01,
    [0, 1, 2, 3, 4],
    createTestFunction()];

  // object
  this.obj = new ChildTestObject();

  // Date
  this.date_object = new Date(10);

  // function
  this.func = createTestFunction();
}

function testGetSystemTime() {
  if (isUsingCCTests) {
    // Test system time increases.
    var start = internalTests.getSystemTime();
    var callback = function() {
      var end = internalTests.getSystemTime();
      assert(start < end, 'System time should increase.');
      completeAsync();
    };
    var timer = google.gears.factory.create('beta.timer');
    startAsync();
    // getSystemTime resolution is only 1s on WinCE.
    timer.setTimeout(callback, 2000);
  }
}

function testPerfTimer() {
  if (isUsingCCTests) {
    // Test uninitialised use.
    assertError(
        function() { internalTests.stopPerfTimer(); },
        'Perf timer has not been started.',
        'Calling stopPerfTimer() should fail if the timer has not been ' +
        'started.');

    // Test repeated starts.
    internalTests.startPerfTimer();
    assertError(
        function() { internalTests.startPerfTimer(); },
        'Perf timer is already running.',
        'Calling startPerfTimer() should fail if the timer is already ' +
        'running.');

    // Test non-zero elapsed time.
    var callback = function() {
      elapsed = internalTests.stopPerfTimer();
      assert(elapsed > 0, 'Elapsed time should be greater than zero.');
      completeAsync();
    };
    var timer = google.gears.factory.create('beta.timer');
    startAsync();
    // Perf timer resolution should be 1us at worst.
    timer.setTimeout(callback, 1);
  }
}

// SAFARI-TEMP - Disable tests that don't currently work on Safari.
testCreateObject._disable_in_safari = true;
testObjectProperties._disable_in_safari = true;
