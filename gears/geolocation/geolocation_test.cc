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

#if USING_CCTESTS

#include <assert.h>
#include "gears/base/common/common.h"
#include "gears/base/common/event.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/stopwatch.h"  // For GetCurrentTimeMillis()
#include "gears/base/common/thread.h"
#include "gears/blob/blob.h"
#include "gears/geolocation/geolocation.h"
#include "gears/geolocation/device_data_provider.h"
#include "gears/geolocation/location_provider_pool.h"
#include "gears/geolocation/network_location_request.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// TODO(steveblock): Many of the string constants for network request and
// JavaScript object properties could be merged with those in
// network_location_request.cc and geolocation.cc. Consider a header file for
// Geolocation string constants.
static const char16 *kAgeString = STRING16(L"age");
static const char16 *kCellIdString = STRING16(L"cell_id");
static const char16 *kLocationAreaCodeString =
    STRING16(L"location_area_code");
static const char16 *kMobileNetworkCodeString =
    STRING16(L"mobile_network_code");
static const char16 *kMobileCountryCodeString =
    STRING16(L"mobile_country_code");
static const char16 *kRadioSignalStrengthString =
    STRING16(L"radio_signal_strength");
static const char16 *kTimingAdvanceString = STRING16(L"timing_advance");
static const char16 *kDeviceIdString = STRING16(L"device_id");
static const char16 *kHomeMobileNetworkCodeString =
    STRING16(L"home_mobile_network_code");
static const char16 *kHomeMobileCountryCodeString =
    STRING16(L"home_mobile_country_code");
static const char16 *kRadioTypeString = STRING16(L"radio_type");
static const char16 *kCarrierString = STRING16(L"carrier");

static const char16 *kMacAddressString = STRING16(L"mac_address");
static const char16 *kChannelString = STRING16(L"channel");
static const char16 *kSignalToNoiseString = STRING16(L"signal_to_noise");
static const char16 *kSsidString = STRING16(L"ssid");

// Local functions
// Retrieves a named integer property from a JavaScript object. Does nothing if
// the property is undefined but throws an exception if the property is of any
// other non-integer type.
static void GetIntegerPropertyIfDefined(JsCallContext *context,
                                        const JsObject &javascript_object,
                                        const std::string16 &name,
                                        int *value);
// As above but for a string property.
static void GetStringPropertyIfDefined(JsCallContext *context,
                                       const JsObject &javascript_object,
                                       const std::string16 &name,
                                       std::string16 *value);
// As above but for a double property.
static void GetDoublePropertyIfDefined(JsCallContext *context,
                                       const JsObject& javascript_object,
                                       const std::string16& name,
                                       double *value);

// A mock implementation of DeviceDataProviderImplBase for testing.
template<typename DataType>
class MockDeviceDataProviderImpl
    : public DeviceDataProviderImplBase<DataType>,
      public Thread {
 public:
  // Factory method for use with DeviceDataProvider::SetFactory.
  static DeviceDataProviderImplBase<DataType> *Create() {
    return new MockDeviceDataProviderImpl<DataType>();
  }

  MockDeviceDataProviderImpl() {
    Start();
  }
  virtual ~MockDeviceDataProviderImpl() {
    stop_event_.Signal();
    Join();
  }

  // DeviceDataProviderImplBase implementation.
  virtual bool GetData(DataType *data_out) {
    assert(data_out);
    MutexLock lock(&data_mutex_);
    *data_out = data_;
    // We always have all the data we can get, so return true.
    return true;
  }

  static void SetData(const DataType &new_data) {
    MutexLock lock(&data_mutex_);
    new_data_ = new_data;
  }

 private:
  // Thread implementation.
  virtual void Run() {
    while (!stop_event_.WaitWithTimeout(1000)) {
      data_mutex_.Lock();
      bool new_data_available = !data_.Matches(new_data_);
      if (new_data_available) {
        data_ = new_data_;
      }
      data_mutex_.Unlock();
      if (new_data_available) {
        DeviceDataProviderImplBase<DataType>::NotifyListeners();
      }
    }
  }

  Event stop_event_;

  DataType data_;
  static DataType new_data_;
  static Mutex data_mutex_;

  DISALLOW_EVIL_CONSTRUCTORS(MockDeviceDataProviderImpl);
};

// static
template<typename DataType>
DataType MockDeviceDataProviderImpl<DataType>::new_data_;

// static
template<typename DataType>
Mutex MockDeviceDataProviderImpl<DataType>::data_mutex_;

// Mock implementation of a location provider for testing.
class MockLocationProvider
    : public LocationProviderBase,
      public Thread {
 public:
  MockLocationProvider()
      : is_shutting_down_(false),
        is_new_listener_waiting_(false) {
    Start();
  }
  virtual ~MockLocationProvider() {
    is_shutting_down_ = true;
    worker_event_.Signal();
    Join();
  }

  void RegisterListener(ListenerInterface *listener, bool request_address) {
    MutexLock lock(&position_mutex_);
    is_new_listener_waiting_ = true;
    worker_event_.Signal();
    LocationProviderBase::RegisterListener(listener, request_address);
  }

  static void SetPosition(const Position &position) {
    MutexLock lock(&position_mutex_);
    position_ = position;
    is_new_position_available_ = true;
    worker_event_.Signal();
  }

 private:
  // LocationProviderBase implementation.
  virtual void GetPosition(Position *position) {
    assert(position);
    MutexLock lock(&position_mutex_);
    *position = position_;
  }

  // Thread implementation.
  virtual void Run() {
    while (!is_shutting_down_) {
      worker_event_.Wait();
      MutexLock lock(&position_mutex_);
      if (is_new_position_available_ || is_new_listener_waiting_) {
        UpdateListeners();
        is_new_position_available_ = false;
        is_new_listener_waiting_ = false;
      }
    }
  }

  static Position position_;
  static Mutex position_mutex_;
  static bool is_new_position_available_;
  static Event worker_event_;

  bool is_shutting_down_;
  bool is_new_listener_waiting_;

  LocationProviderBase::ListenerInterface *listener_;

  DISALLOW_EVIL_CONSTRUCTORS(MockLocationProvider);
};

// static
Position MockLocationProvider::position_;

// static
Mutex MockLocationProvider::position_mutex_;

// static
bool MockLocationProvider::is_new_position_available_ = false;

// static
Event MockLocationProvider::worker_event_;

LocationProviderBase *NewMockLocationProvider() {
  return new MockLocationProvider();
}

void TestParseGeolocationOptions(JsCallContext *context,
                                 JsRunnerInterface *js_runner) {
  std::vector<std::string16> urls;
  GearsGeolocation::FixRequestInfo info;
  if (!GearsGeolocation::ParseArguments(context, false, &urls, &info)) {
    if (!context->is_exception_set()) {
      context->SetException(
          STRING16(L"Internal error parsing geolocation options."));
    }
    return;
  }
  // Add the urls as a property of the returned object.
  scoped_ptr<JsObject> return_object(js_runner->NewObject());
  assert(return_object.get());
  scoped_ptr<JsArray> url_array(js_runner->NewArray());
  assert(url_array.get());
  for (int i = 0; i < static_cast<int>(urls.size()); ++i) {
    if (!url_array->SetElementString(i, urls[i])) {
      context->SetException(STRING16(L"Failed to set return value."));
      return;
    }
  }
  if (!return_object->SetPropertyBool(STRING16(L"repeats"), info.repeats) ||
      !return_object->SetPropertyBool(STRING16(L"enableHighAccuracy"),
                                      info.enable_high_accuracy) ||
      !return_object->SetPropertyBool(STRING16(L"gearsRequestAddress"),
                                      info.request_address) ||
      !return_object->SetPropertyString(STRING16(L"gearsAddressLanguage"),
                                        info.address_language) ||
      !return_object->SetPropertyArray(STRING16(L"gearsLocationProviderUrls"),
                                       url_array.get())) {
    context->SetException(STRING16(L"Failed to set return value."));
    return;
  }
  context->SetReturnValue(JSPARAM_OBJECT, return_object.get());
}

void TestGeolocationFormRequestBody(JsCallContext *context) {
  // Parsing radio and wifi data from JavaScript would involve more complex code
  // than that which we're trying to test. Instead we hard-code these values.
  // The values must be kept in sync with internal_tests.js.

  RadioData radio_data;
  CellData cell_data;
  cell_data.cell_id = 23874;
  cell_data.location_area_code = 98;
  cell_data.mobile_network_code = 15;
  cell_data.mobile_country_code = 234;
  cell_data.radio_signal_strength = -65;
  radio_data.cell_data.push_back(cell_data);
  radio_data.radio_type = RADIO_TYPE_GSM;

  WifiData wifi_data;
  AccessPointData access_point_data;
  access_point_data.mac_address = STRING16(L"00-0b-86-d7-6a-42");
  access_point_data.radio_signal_strength = -50;
  access_point_data.age = 15;
  access_point_data.channel = 19;
  access_point_data.signal_to_noise = 10;
  access_point_data.ssid = STRING16(L"Test SSID");
  wifi_data.access_point_data.push_back(access_point_data);

  scoped_refptr<BlobInterface> blob;
  if (!NetworkLocationRequest::FormRequestBody(STRING16(L"www.google.com"),
                                               radio_data,
                                               wifi_data,
                                               true,
                                               STRING16(L"en-GB"),
                                               53.1,
                                               -0.1,
                                               &blob)) {
    context->SetException(STRING16(L"Failed to form request body."));
    return;
  }
  assert(blob.get());
  int64 length = blob->Length();
  // We only handle the case where the size of the blob is less than kuint32max.
  // All good request bodies should be much smaller than this.
  assert(length > 0 && length < kuint32max);
  std::vector<uint8> request_body_vector;
  request_body_vector.resize(static_cast<unsigned int>(length));
  if (!blob->Read(&request_body_vector[0], 0, length)) {
    context->SetException(STRING16(L"Failed to extract request string from "
                                   L"blob."));
    return;
  }
  std::string16 request_body;
  if (!UTF8ToString16(reinterpret_cast<char*>(&request_body_vector[0]),
                      request_body_vector.size(),
                      &request_body)) {
    context->SetException(STRING16(L"Failed to convert request string from "
                                   L"UTF8."));
    return;
  }
  context->SetReturnValue(JSPARAM_STRING16, &request_body);
}

void TestGeolocationGetLocationFromResponse(JsCallContext *context,
                                            JsRunnerInterface *js_runner) {
  bool http_post_result;
  int status_code;
  std::string16 response_body;
  int64 timestamp;
  std::string16 server_url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &http_post_result },
    { JSPARAM_REQUIRED, JSPARAM_INT, &status_code },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &response_body },
    { JSPARAM_REQUIRED, JSPARAM_INT64, &timestamp },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &server_url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }

  std::string response_body_utf8;
  if (!String16ToUTF8(response_body.c_str(), response_body.size(),
                      &response_body_utf8)) {
    context->SetException(
        STRING16(L"Failed to convert response body to UTF8."));
    return;
  }

  Position position;
  NetworkLocationRequest::GetLocationFromResponse(http_post_result,
                                                  status_code,
                                                  response_body_utf8,
                                                  timestamp,
                                                  server_url,
                                                  &position);

  scoped_ptr<JsObject> return_object(js_runner->NewObject());
  assert(return_object.get());
  if (position.IsGoodFix()) {
    if (!GearsGeolocation::CreateJavaScriptPositionObject(
        position,
        true,  // Use address if present
        js_runner,
        return_object.get())) {
      context->SetException(STRING16(L"Failed to create JavaScript Position "
                                     L"object."));
      return;
    }
  } else {
    if (!GearsGeolocation::CreateJavaScriptPositionErrorObject(
        position,
        return_object.get())) {
      context->SetException(STRING16(L"Failed to create JavaScript "
                                     L"PositionError object."));
      return;
    }
  }

  context->SetReturnValue(JSPARAM_OBJECT, return_object.get());
}

void ConfigureGeolocationRadioDataProviderForTest(JsCallContext *context) {
  assert(context);

  // Get the arguments provided from JavaScript. 
  JsObject object;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &object },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  
  // Note that we only support providing data for a single cell.
  CellData cell_data;
  // Update the fields of cell_data, if they have been provided.
  GetIntegerPropertyIfDefined(context, object, kCellIdString,
                              &cell_data.cell_id);
  GetIntegerPropertyIfDefined(context, object, kLocationAreaCodeString,
                              &cell_data.location_area_code);
  GetIntegerPropertyIfDefined(context, object, kMobileNetworkCodeString,
                              &cell_data.mobile_network_code);
  GetIntegerPropertyIfDefined(context, object, kMobileCountryCodeString,
                              &cell_data.mobile_country_code);
  GetIntegerPropertyIfDefined(context, object, kAgeString, &cell_data.age);
  GetIntegerPropertyIfDefined(context, object, kRadioSignalStrengthString,
                              &cell_data.radio_signal_strength);
  GetIntegerPropertyIfDefined(context, object, kTimingAdvanceString,
                              &cell_data.timing_advance);

  RadioData radio_data;
  GetStringPropertyIfDefined(context, object, kDeviceIdString,
                             &radio_data.device_id);
  GetIntegerPropertyIfDefined(context, object, kHomeMobileNetworkCodeString,
                              &radio_data.home_mobile_network_code);
  GetIntegerPropertyIfDefined(context, object, kHomeMobileCountryCodeString,
                              &radio_data.home_mobile_country_code);
  int radio_type = RADIO_TYPE_UNKNOWN;
  GetIntegerPropertyIfDefined(context, object, kRadioTypeString, &radio_type);
  switch (radio_type) {
    case RADIO_TYPE_UNKNOWN:
      radio_data.radio_type = RADIO_TYPE_UNKNOWN;
      break;
    case RADIO_TYPE_GSM:
      radio_data.radio_type = RADIO_TYPE_GSM;
      break;
    case RADIO_TYPE_CDMA:
      radio_data.radio_type = RADIO_TYPE_CDMA;
      break;
    case RADIO_TYPE_WCDMA:
      radio_data.radio_type = RADIO_TYPE_WCDMA;
      break;
    default:
      LOG(("ConfigureGeolocationRadioDataProviderForTest() : Ignoring value %s "
           "for radio_type.\n", radio_type));
  }
  GetStringPropertyIfDefined(context, object, kCarrierString,
                             &radio_data.carrier);

  // The GetXXXPropertyIfDefined functions set an exception if the property has
  // the wrong type.
  if (context->is_exception_set()) {
    return;
  }

  radio_data.cell_data.push_back(cell_data);

  MockDeviceDataProviderImpl<RadioData>::SetData(radio_data);
  RadioDataProvider::SetFactory(MockDeviceDataProviderImpl<RadioData>::Create);
}

void ConfigureGeolocationWifiDataProviderForTest(JsCallContext *context) {
  assert(context);

  // Get the arguments provided from JavaScript. 
  JsObject object;
  JsArgument argv[] = { { JSPARAM_REQUIRED, JSPARAM_OBJECT, &object }, };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  
  // Note that we only support providing data for a single access point.
  AccessPointData access_point_data;
  // Update the fields of access_point_data, if they have been provided.
  GetStringPropertyIfDefined(context, object, kMacAddressString,
                             &access_point_data.mac_address);
  GetIntegerPropertyIfDefined(context, object, kRadioSignalStrengthString,
                              &access_point_data.radio_signal_strength);
  GetIntegerPropertyIfDefined(context, object, kAgeString,
                              &access_point_data.age);
  GetIntegerPropertyIfDefined(context, object, kChannelString,
                              &access_point_data.channel);
  GetIntegerPropertyIfDefined(context, object, kSignalToNoiseString,
                              &access_point_data.signal_to_noise);
  GetStringPropertyIfDefined(context, object, kSsidString,
                             &access_point_data.ssid);

  // The GetXXXPropertyIfDefined functions set an exception if the property has
  // the wrong type.
  if (context->is_exception_set()) {
    return;
  }

  WifiData wifi_data;
  wifi_data.access_point_data.push_back(access_point_data);

  MockDeviceDataProviderImpl<WifiData>::SetData(wifi_data);
  WifiDataProvider::SetFactory(MockDeviceDataProviderImpl<WifiData>::Create);
}

void ConfigureGeolocationMockLocationProviderForTest(JsCallContext *context) {
  assert(context);

  // Get the arguments provided from JavaScript. 
  JsObject object;
  JsArgument argv[] = { { JSPARAM_REQUIRED, JSPARAM_OBJECT, &object }, };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  
  Position position;
  GetDoublePropertyIfDefined(context, object, STRING16(L"latitude"),
                             &position.latitude);
  GetDoublePropertyIfDefined(context, object, STRING16(L"longitude"),
                             &position.longitude);
  GetDoublePropertyIfDefined(context, object, STRING16(L"altitude"),
                             &position.altitude);
  GetDoublePropertyIfDefined(context, object, STRING16(L"accuracy"),
                             &position.accuracy);
  GetDoublePropertyIfDefined(context, object, STRING16(L"altitudeAccuracy"),
                             &position.altitude_accuracy);
  GetIntegerPropertyIfDefined(context, object, STRING16(L"errorCode"),
                             &position.error_code);
  GetStringPropertyIfDefined(context, object, STRING16(L"errorMessage"),
                             &position.error_message);

  // The GetXXXPropertyIfDefined functions set an exception if the property has
  // the wrong type.
  if (context->is_exception_set()) {
    return;
  }

  position.timestamp = GetCurrentTimeMillis();
  if (!position.IsInitialized()) {
    context->SetException(STRING16(L"Specified position is not valid.\n"));
    return;
  }

  // Set the position used by the mock location provider.
  MockLocationProvider::SetPosition(position);

  // Configure the location provider pool to use the mock location provider.
  LocationProviderPool::GetInstance()->UseMockLocationProvider(true);
}

void RemoveGeolocationMockLocationProvider() {
  LocationProviderPool::GetInstance()->UseMockLocationProvider(false);
}

// Local functions
static bool GetPropertyIfDefined(JsCallContext *context,
                                 const JsObject& javascript_object,
                                 const std::string16& name,
                                 JsScopedToken *token) {
  assert(context);
  assert(token);
  // GetProperty should always succeed, but will get a token of type
  // JSPARAM_UNDEFINED if the requested property is not present.
  if (!javascript_object.GetProperty(name, token)) {
    assert(false);
    return false;
  }
  if (JsTokenGetType(*token, context->js_context()) == JSPARAM_UNDEFINED) {
    return false;
  }
  return true;
}

static void GetIntegerPropertyIfDefined(JsCallContext *context,
                                        const JsObject& javascript_object,
                                        const std::string16& name,
                                        int *value) {
  assert(context);
  assert(value);
  JsScopedToken token;
  if (!GetPropertyIfDefined(context, javascript_object, name, &token)) {
    return;
  }
  if (!JsTokenToInt_NoCoerce(token, context->js_context(), value)) {
    std::string16 error = STRING16(L"property ");
    error += name;
    error += STRING16(L" should be an integer.");
    context->SetException(error);
  }
}

static void GetStringPropertyIfDefined(JsCallContext *context,
                                       const JsObject& javascript_object,
                                       const std::string16& name,
                                       std::string16 *value) {
  assert(context);
  assert(value);
  JsScopedToken token;
  if (!GetPropertyIfDefined(context, javascript_object, name, &token)) {
    return;
  }
  if (!JsTokenToString_NoCoerce(token, context->js_context(), value)) {
    std::string16 error = STRING16(L"property ");
    error += name;
    error += STRING16(L" should be a string.");
    context->SetException(error);
  }
}

static void GetDoublePropertyIfDefined(JsCallContext *context,
                                       const JsObject& javascript_object,
                                       const std::string16& name,
                                       double *value) {
  assert(context);
  assert(value);
  JsScopedToken token;
  if (!GetPropertyIfDefined(context, javascript_object, name, &token)) {
    return;
  }
  if (!JsTokenToDouble_NoCoerce(token, context->js_context(), value)) {
    std::string16 error = STRING16(L"property ");
    error += name;
    error += STRING16(L" should be a double.");
    context->SetException(error);
  }
}

#endif  // USING_CCTESTS
