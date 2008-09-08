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

#ifndef GEARS_GEOLOCATION_GEOLOCATION_TEST_H__
#define GEARS_GEOLOCATION_GEOLOCATION_TEST_H__

class JsCallContext;
class JsRunnerInterface;

// IN: object position_options
// OUT: object parsed_options
void TestParseGeolocationOptions(JsCallContext *context,
                                 JsRunnerInterface *js_runner);

// IN: nothing
// OUT: string request_body
void TestGeolocationFormRequestBody(JsCallContext *context);

// IN: bool http_post_result, int status_code, string response_body,
//     int64 timestamp, string server_url
// OUT: object position
void TestGeolocationGetLocationFromResponse(JsCallContext *context,
                                            JsRunnerInterface *js_runner);

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
// the position that the mock provider will provide. Properties are latitude,
// longitude, altitude, accuracy, altitudeAccuracy, errorCode and errorMessage.
// Note that this a combination of some of the properties available on the
// Position and PositionError objects returned by the Geolocation module.
// IN: object position.
// OUT: nothing
void ConfigureGeolocationMockLocationProviderForTest(JsCallContext *context);

// Configures the location provider pool to not use the mock location provider.
void RemoveGeolocationMockLocationProvider();

#endif  // GEARS_GEOLOCATION_GEOLOCATION_TEST_H__
