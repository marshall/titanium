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

#ifndef GEARS_GEOLOCATION_GPS_LOCATION_PROVIDER_WINCE_H__
#define GEARS_GEOLOCATION_GPS_LOCATION_PROVIDER_WINCE_H__

#include <atlsync.h>  // For CEvent.
#include <gpsapi.h>
#include "gears/base/common/common.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/thread.h"
#include "gears/geolocation/location_provider.h"
#include "gears/geolocation/geolocation.h"

class WinceGpsLocationProvider
    : public LocationProviderBase,
      public Thread {
 public:
  WinceGpsLocationProvider();
  ~WinceGpsLocationProvider();

  // Override LocationProviderBase implementation.
  virtual void RegisterListener(
      LocationProviderBase::ListenerInterface *listener,
      bool request_address);

  // LocationProviderBase implementation.
  virtual void GetPosition(Position *position);

 private:
  // Thread implementation
  virtual void Run();

  // Callbacks used to handle updates from the GPS Intermediate Driver library.
  void HandlePositionUpdate();
  void HandleStateChange();

  // The current best position estimate and its mutex.
  Position position_;
  Mutex position_mutex_;

  // Hanlde to the GPS Intermediate Driver library.
  HANDLE gps_handle_;

  // Events signalled to the thread that waits for events from the GPS API.
  CEvent stop_event_;
  CEvent new_listener_waiting_event_;

  // The state of the attempt to get a fix from the GPS. Use for determining
  // timeouts.
  typedef enum {
    STATE_CONNECTING,
    STATE_ACQUIRING_FIX,
    STATE_FIX_ACQUIRED,
  } State;
  State state_;

  DISALLOW_EVIL_CONSTRUCTORS(WinceGpsLocationProvider);
};

#endif  // GEARS_GEOLOCATION_GPS_LOCATION_PROVIDER_WINCE_H__
