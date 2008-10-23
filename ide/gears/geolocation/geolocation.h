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
//
// This file declares GearsGeolocation, the main class of the Gears Geolocation
// API. The GearsGeolocation object provides the API methods to JavaScript. It
// uses a set of location providers to gather position information.
//
// This file also declares the Position structure, which is used to represent a
// position fix.

#ifndef GEARS_GEOLOCATION_GEOLOCATION_H__
#define GEARS_GEOLOCATION_GEOLOCATION_H__

#include <map>
#include <vector>
#include "gears/base/common/base_class.h"
#include "gears/base/common/message_service.h"
#ifdef USING_CCTESTS
#include "gears/geolocation/geolocation_test.h"
#endif
#include "gears/geolocation/location_provider.h"
#include "gears/geolocation/timed_callback.h"
#include "third_party/linked_ptr/linked_ptr.h"

static const double kBadLatLng = 200;
// Lowest point on land is at approximately -400 metres.
static const int kBadAltitude = -1000;
static const int kBadAccuracy = -1;  // Accuracy must be non-negative.


// Error codes for returning to JavaScript.
const int kGeolocationLocationAcquisitionErrorCode = 2;
const int kGeolocationLocationNotFoundErrorCode = 3;

// The internal representation of an address.
struct Address {
  std::string16 street_number; // street number
  std::string16 street;        // street address
  std::string16 premises;      // premises, e.g. building name
  std::string16 city;          // city name
  std::string16 county;        // county name
  std::string16 region;        // region, e.g. a state in the US
  std::string16 country;       // country
  std::string16 country_code;  // country code (ISO 3166-1)
  std::string16 postal_code;   // postal code
};

// The internal representation of a position. Some properties use different
// types when passed to JavaScript.
struct Position {
 public:
  Position()
      : latitude(kBadLatLng),
        longitude(kBadLatLng),
        altitude(kBadAltitude),
        accuracy(kBadAccuracy),
        altitude_accuracy(kBadAccuracy),
        timestamp(-1),
        error_code(kint32min) {}
  bool IsGoodFix() const {
    // A good fix has a valid latitude, longitude, accuracy and timestamp.
    return latitude >= -90.0 && latitude <= 90.0 &&
           longitude >= -180.0 && longitude <= 180.0 &&
           accuracy >= 0.0 &&
           timestamp != -1;
  }
  bool IsInitialized() const {
    return IsGoodFix() || error_code != kint32min;
  }

  // These properties are returned to JavaScript as a Position object.
  double latitude;          // In degrees
  double longitude;         // In degrees
  double altitude;          // In metres
  double accuracy;          // In metres
  double altitude_accuracy; // In metres
  int64 timestamp;          // Milliseconds since 1st Jan 1970
  // Note that the corresponding JavaScript Position property is 'gearsAddress'.
  Address address;

  // These properties are returned to JavaScript as a PositionError object.
  int error_code;
  std::string16 error_message;  // Human-readable error message
};

// The principal class of the Geolocation API.
class GearsGeolocation
    : public ModuleImplBaseClass,
      public LocationProviderBase::ListenerInterface,
      public MessageObserverInterface,
      public TimedCallback::ListenerInterface,
      public JsEventHandlerInterface {
 public:
#ifdef USING_CCTESTS
  // Uses ParseArguments for testing.
  friend void TestParseGeolocationOptions(JsCallContext *context,
                                          JsRunnerInterface *js_runner);
  // Uses CreateJavaScriptPositionObject and CreateJavaScriptPositionErrorObject
  // for testing.
  friend void TestGeolocationGetLocationFromResponse(
      JsCallContext *context,
      JsRunnerInterface *js_runner);
#endif

  GearsGeolocation();
  virtual ~GearsGeolocation();

  // API methods. See the Geolocation API design document at
  // http://code.google.com/p/google-gears/wiki/LocationAPI for a more complete
  // description of these methods.

  // Gets the last, cached, position fix. This method does not cause Gears to
  // actively seek a position update.
  // IN: nothing
  // OUT: object position
  void GetLastPosition(JsCallContext *context);

  // Instructs Gears to get a new position fix. The supplied callback function
  // is called with a valid position as soon as it is available, or with NULL on
  // failure.
  // IN: function successCallback, optional function errorCallback,
  //     optional object options
  // OUT: nothing
  void GetCurrentPosition(JsCallContext *context);

  // Instructs Gears to get a new position fix. The supplied callback function
  // is called repeatedly with position updates as they become available. The
  // return value is a unique ID for this watch which can be used to cancel it.
  // IN: function successCallback, optional function errorCallback,
  //     optional object options
  // OUT: int watch_id
  void WatchPosition(JsCallContext *context);

  // Cancels the position watch specified by the supplied ID.
  // IN: int watchId
  // OUT: nothing
  void ClearWatch(JsCallContext *context);

  // Triggers the geolocation-specific permissions dialog.
  // IN:  string siteName, string imageUrl, string extraMessage
  // OUT: boolean permission
  void GetPermission(JsCallContext *context);

  // Maintains all the data for a position fix.
  typedef std::vector<LocationProviderBase*> ProviderVector;
  struct FixRequestInfo {
    FixRequestInfo() : last_success_callback_time(0) {}
    ProviderVector providers;
    bool enable_high_accuracy;
    bool request_address;
    std::string16 address_language;
    bool repeats;
    // Linked_ptr so we can use FixRequestInfo in STL containers.
    linked_ptr<JsRootedCallback> success_callback;
    linked_ptr<JsRootedCallback> error_callback;
    // The last position sent back to JavaScript. Used by repeating requests
    // only.
    Position last_position;
    // The time at which we last made a success callback to JavaScript, in ms
    // since the epoch.
    int64 last_success_callback_time;
    // The timer used for a pending future success callback in a watch.
    linked_ptr<TimedCallback> success_callback_timer;
    // The position that will be used used for a pending future success callback
    // in a watch.
    Position pending_position;
  };

 private:
  // LocationProviderBase::ListenerInterface implementation.
  virtual bool LocationUpdateAvailable(LocationProviderBase *provider);

  // MessageObserverInterface implementation.
  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data);

  // TimedCallback::ListenerInterface implementation.
  virtual void OnTimeout(TimedCallback *caller, void *user_data);

  // JsEventHandlerInterface implementation used to handle the 'JSEVENT_UNLOAD'
  // event.
  void HandleEvent(JsEventType event_type);

  // Internal method used by OnNotify.
  void LocationUpdateAvailableImpl(LocationProviderBase *provider);

  // Internal method used by GetCurrentPosition and WatchPosition to get a
  // position fix.
  void GetPositionFix(JsCallContext *context, bool repeats);

  // Cancels an ongoing watch.
  bool CancelWatch(const int &watch_id);

  // In the two following methods, we use an ID to identify the fix request,
  // rather than a raw pointer. However, both methods require that the ID is
  // valid. See the implementation of LocationUpdateAvailableImpl.

  // Internal method used by LocationUpdateAvailable to handle an update for a
  // repeating fix request. 
  void HandleRepeatingRequestUpdate(int id,
                                    const Position &position);

  // Internal method used by LocationUpdateAvailable to handle an update for a
  // non-repeating fix request.
  void HandleSingleRequestUpdate(LocationProviderBase *provider,
                                 int id,
                                 const Position &position);

  // Internal method to make the callback to JavaScript once we have a postion
  // fix.
  bool MakeSuccessCallback(FixRequestInfo *fix_info, const Position &position);
  bool MakeErrorCallback(FixRequestInfo *fix_info, const Position &position);

  // Parses the JavaScript arguments passed to the GetCurrentPosition and
  // WatchPosition methods.
  static bool ParseArguments(JsCallContext *context,
                             bool repeats,
                             std::vector<std::string16> *urls,
                             GearsGeolocation::FixRequestInfo *info);
  // Parses a JsObject representing the options parameter. The output is a
  // vector of URLs and the fix request info. Return value indicates success.
  static bool ParseOptions(JsCallContext *context,
                           const JsObject &options,
                           std::vector<std::string16> *urls,
                           GearsGeolocation::FixRequestInfo *info);
  // Parses a JsScopedToken representing the gearsLocationProviderUrls field.
  // The output is a vector of URLs. Return value indicates success.
  static bool ParseLocationProviderUrls(JsCallContext *context,
                                        const JsScopedToken &token,
                                        std::vector<std::string16> *urls);

  // Converts a Gears position object to a JavaScript object. static for use in
  // unit tests.
  static bool CreateJavaScriptPositionObject(const Position &position,
                                             bool use_address,
                                             JsRunnerInterface *js_runner,
                                             JsObject *js_object);
  static bool CreateJavaScriptPositionErrorObject(const Position &position,
                                                  JsObject *js_object);

  // Gets the fix request for a given ID. The supplied ID must be valid.
  FixRequestInfo *GetFixRequest(int id);

  // Takes a pointer to a new fix request and records it in our map. Returns
  // false if the maximum number of fix requests has been reached. Otherwise
  // return true.
  bool RecordNewFixRequest(FixRequestInfo *fix_request);

  // Removes a fix request. Cancels any pending requests to the location
  // providers it uses. Note that this does not delete the FixRequestInfo
  // object.
  void RemoveFixRequest(int id);

  // Deletes a fix request and decrements our ref count.
  void DeleteFixRequest(FixRequestInfo *fix_request);

  // Removes a location provider from a fix request.
  void RemoveProvider(LocationProviderBase *provider, int id);

  // Causes a callback to JavaScript to be made at the specified number of
  // milliseconds in the future.
  void MakeFutureSuccessCallback(int timeout_milliseconds,
                                 FixRequestInfo *fix_info,
                                 const Position &position);

  // TODO(steveblock): Refactor the logic used to maintain the fix request maps
  // into a separate class.

  // A map from providers to fix request IDs. We use this map when looking up
  // the single fix requests in which a provider is involved and to make sure
  // that the provider from which a location update is received is still valid
  // once the callback has been marshalled. See the comment in the
  // implementation of LocationUpdateAvailableImpl for a discussion of how and
  // why request IDs are used.
  typedef std::vector<int> IdList;
  typedef std::map<LocationProviderBase*, IdList> ProviderMap;
  ProviderMap providers_;

  // Map from fix request ID to fix request.
  typedef std::map<int, FixRequestInfo*> FixRequestInfoMap;
  FixRequestInfoMap fix_requests_;
  int next_single_request_id_;  // Always negative
  int next_watch_id_;  // Always positive

  // The current best estimate for our position. This is the position returned
  // by GetLastPosition and is shared by all watches.
  Position last_position_;

  ThreadId java_script_thread_id_;

  scoped_ptr<JsEventMonitor> unload_monitor_;

  DECL_SINGLE_THREAD
  DISALLOW_EVIL_CONSTRUCTORS(GearsGeolocation);
};

#endif  // GEARS_GEOLOCATION_GEOLOCATION_H__
