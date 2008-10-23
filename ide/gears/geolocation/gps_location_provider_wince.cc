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
// We use the GPS Intermediate Driver to access the GPS. See
// http://msdn.microsoft.com/en-us/library/ms850332.aspx.
//
// Note that the GPS driver may not be enabled by default. To enable it on a
// WinMo 6 device, you must set the virtual port to be used by the driver.
// - Go to Start -> Settings.
// - In the System tab, go to External GPS.
// - In the Programs tab, set 'GPS Program port' to any unused port.
// See http://blogs.msdn.com/windowsmobile/archive/2006/06/07/620387.aspx
// and http://www.expansys-usa.com/ft.aspx?k=100176&page=3.
//
// TODO(steveblock): consider adding this information to the Geolocation API
// documentation so that developers can communicate it to users if appropriate.

// TODO(cprince): remove platform-specific #ifdef guards when OS-specific
// sources (e.g. WIN32_CPPSRCS) are implemented
#ifdef WINCE

#include "gears/geolocation/gps_location_provider_wince.h"

#include <service.h>
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/time_utils_win32.h"

// Maximum age of GPS data we'll accept.
static const int kGpsDataMaximumAgeMilliseconds = 5 * 1000;

// Maximum time we'll wait to connect to the GPS.
static const int kMaximumConnectionTimeMilliseconds = 45 * 1000;
// Maximum time we'll wait for the GPS to acquire a fix.
static const int kMaximumAcquistionTimeMilliseconds = 5 * 60 *1000;

static const char16 *kFailedToConnectErrorMessage =
    STRING16(L"Failed to connect to GPS.");
static const char16 *kFailedToGetFixErrorMessage =
    STRING16(L"GPS failed to get a position fix.");


LocationProviderBase *NewGpsLocationProvider() {
  return new WinceGpsLocationProvider();
}


WinceGpsLocationProvider::WinceGpsLocationProvider() {
  // Initialise member events.
  stop_event_.Create(NULL, FALSE, FALSE, NULL);
  new_listener_waiting_event_.Create(NULL, FALSE, FALSE, NULL);

  // Start the worker thread.
  Start();
}

WinceGpsLocationProvider::~WinceGpsLocationProvider() {
  // Shut down the worker thread.
  stop_event_.Set();
  Join();
}

void WinceGpsLocationProvider::RegisterListener(
    LocationProviderBase::ListenerInterface *listener,
    bool request_address) {
  LocationProviderBase::RegisterListener(listener, request_address);

  // Signal to the worker thread that there is a new listener.
  new_listener_waiting_event_.Set();
}

// LocationProviderBase implementation
void WinceGpsLocationProvider::GetPosition(Position *position) {
  assert(position);
  MutexLock lock(&position_mutex_);
  *position = position_;
}

// Thread implementation
void WinceGpsLocationProvider::Run() {
  // Initialise events.
  CEvent position_update_event;
  CEvent state_change_event;
  position_update_event.Create(NULL, FALSE, FALSE, NULL);
  state_change_event.Create(NULL, FALSE, FALSE, NULL);
  HANDLE events[] = {stop_event_,
                     new_listener_waiting_event_,
                     position_update_event,
                     state_change_event};

  // Connect to GPS.
  gps_handle_ = GPSOpenDevice(position_update_event,
                              state_change_event,
                              NULL,  // Reserved
                              0);  // Reserved
  if (gps_handle_ == NULL) {
    LOG(("WinceGpsLocationProvider::Run() : Failed to open handle to GPS "
         "device.\n"));
    // Set an error message and call back.
    position_mutex_.Lock();
    position_.error_code = kGeolocationLocationAcquisitionErrorCode;
    position_.error_message = kFailedToConnectErrorMessage;
    position_mutex_.Unlock();
    UpdateListeners();
    return;
  }

  // Wait for updates from the GPS driver. It seems that if GPS is not present
  // or the driver is disabled we still get callbacks. This isn't very useful
  // for detecting the fact that there's no GPS available, so we must rely on
  // timeouts.
  state_ = STATE_CONNECTING;
  int64 start_time = GetCurrentTimeMillis();
  bool shutting_down = false;
  while (!shutting_down) {
    DWORD wait_milliseconds;
    switch (state_) {
      case STATE_CONNECTING:
        wait_milliseconds =
            kMaximumConnectionTimeMilliseconds -
            static_cast<DWORD>(GetCurrentTimeMillis() - start_time);
        break;
      case STATE_ACQUIRING_FIX:
        wait_milliseconds =
            kMaximumConnectionTimeMilliseconds +
            kMaximumAcquistionTimeMilliseconds -
            static_cast<DWORD>(GetCurrentTimeMillis() - start_time);
        break;
      case STATE_FIX_ACQUIRED:
        wait_milliseconds = INFINITE;
        break;
      default:
        assert(false);
    }
 
    // It's possible that we were woken up just before the timer expired, in
    // which case the wait time may now be negative. In this case, set the
    // timeout to zero so that we wake up immediately and continue as normal.
    if (wait_milliseconds != INFINITE && wait_milliseconds < 0) {
      wait_milliseconds = 0;
    }

    DWORD event_index = WaitForMultipleObjects(ARRAYSIZE(events),
                                               events,
                                               false,
                                               wait_milliseconds);
    switch (event_index) {
      case WAIT_OBJECT_0:  // stop_event_
        shutting_down = true;
        break;
      case WAIT_OBJECT_0 + 1:  // new_listener_waiting_event_
        // Only call back if we've got a good fix. Otherwise, the new listener
        // should continue to wait.
        if (state_ == STATE_FIX_ACQUIRED) {
          UpdateListeners();
        }
        break;
      case WAIT_OBJECT_0 + 2:  // position_update_event
        HandlePositionUpdate();
        break;
      case WAIT_OBJECT_0 + 3:  // state_change_event
        HandleStateChange();
        break;
      default:
        // Wait timed out.
        assert(wait_milliseconds != INFINITE);
        // Set an error message and call back.
        position_mutex_.Lock();
        switch (state_) {
          case STATE_CONNECTING:
            position_.error_code = kGeolocationLocationAcquisitionErrorCode;
            position_.error_message = kFailedToConnectErrorMessage;
            break;
          case STATE_ACQUIRING_FIX:
            position_.error_code = kGeolocationLocationNotFoundErrorCode;
            position_.error_message = kFailedToGetFixErrorMessage;
            break;
          default:
            assert(false);
        }
        position_mutex_.Unlock();
        UpdateListeners();
        shutting_down = true;
        break;
    }
  }

  if (GPSCloseDevice(gps_handle_) != ERROR_SUCCESS) {
    LOG(("Failed to close connection to GPS.\n"));
  }
}

void WinceGpsLocationProvider::HandlePositionUpdate() {
  bool update_available = false;
  position_mutex_.Lock();

  GPS_POSITION gps_position = {0};
  gps_position.dwSize = sizeof(gps_position);
  gps_position.dwVersion = GPS_VERSION_1;  // The only valid value
  if (GPSGetPosition(gps_handle_,
                     &gps_position,
                     kGpsDataMaximumAgeMilliseconds,
                     0) == ERROR_SUCCESS) {  // Reserved
    if ((gps_position.dwValidFields & GPS_VALID_LATITUDE) &&
        (gps_position.dwValidFields & GPS_VALID_LONGITUDE)) {
      if (position_.latitude != gps_position.dblLatitude ||
          position_.longitude != gps_position.dblLongitude) {
        position_.latitude = gps_position.dblLatitude;
        position_.longitude = gps_position.dblLongitude;
        update_available = true;
      }
    }

    if (gps_position.dwValidFields & GPS_VALID_ALTITUDE_WRT_ELLIPSOID) {
      double altitude = gps_position.flAltitudeWRTEllipsoid;
      if (position_.altitude != altitude) {
        position_.altitude = altitude;
        update_available = true;
      }
    }

    // The GPS Intermediate Driver API does not provide a numerical value for
    // the postition accuracy. However, we can estimate the accuracy from the
    // Position Dilution of Precision (PDOP), which describes how well the
    // geometric configuration of the satellites currently in view lends itself
    // to getting a high accuracy fix (see
    // http://en.wikipedia.org/wiki/Dilution_of_precision_(GPS) and
    // http://www.cgrer.uiowa.edu/cgrer_lab/gps/gpsdefs.html).
    if (gps_position.dwValidFields & GPS_VALID_POSITION_DILUTION_OF_PRECISION) {
      int accuracy;
      if (gps_position.flPositionDilutionOfPrecision <= 3) {
        accuracy = 1;
      } else if (gps_position.flPositionDilutionOfPrecision <= 6) {
        accuracy = 10;
      } else if (gps_position.flPositionDilutionOfPrecision <= 8) {
        accuracy = 20;
      } else {
        accuracy = 50;
      }
      if (position_.accuracy != accuracy) {
        position_.accuracy = accuracy;
        update_available = true;
      }
    }
  }

  GPS_DEVICE device_state = {0};
  device_state.dwSize = sizeof(device_state);
  device_state.dwVersion = GPS_VERSION_1;  // The only valid value
  if (GPSGetDeviceState(&device_state) == ERROR_SUCCESS) {
    if (device_state.ftLastDataReceived.dwHighDateTime != 0 ||
        device_state.ftLastDataReceived.dwLowDateTime != 0) {
      int64 timestamp =
          FiletimeToMilliseconds(device_state.ftLastDataReceived);
      if (position_.timestamp != timestamp) {
        position_.timestamp = timestamp;
      }
    }
  }

  position_mutex_.Unlock();

  // Call back only if the position has changed and is good.
  if (update_available && position_.IsGoodFix()) {
    state_ = STATE_FIX_ACQUIRED;
    UpdateListeners();
  }
}

void WinceGpsLocationProvider::HandleStateChange() {
  // It seems that if GPS is not present, or the GPS driver is disabled, we get
  // this callback exactly once, with hardware and driver states
  // SERVICE_STATE_ON.
  GPS_DEVICE device_state = {0};
  device_state.dwSize = sizeof(device_state);
  device_state.dwVersion = GPS_VERSION_1;  // The only valid value
  if (GPSGetDeviceState(&device_state) == ERROR_SUCCESS) {
    if (state_ == STATE_CONNECTING &&
        (device_state.dwDeviceState == SERVICE_STATE_STARTING_UP ||
         device_state.dwDeviceState == SERVICE_STATE_ON)) {
      state_ = STATE_ACQUIRING_FIX;
    }
  }
}

#endif  // WINCE
