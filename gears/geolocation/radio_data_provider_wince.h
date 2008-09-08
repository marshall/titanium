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

#ifndef GEARS_GEOLOCATION_RADIO_DATA_PROVIDER_WINCE_H__
#define GEARS_GEOLOCATION_RADIO_DATA_PROVIDER_WINCE_H__

#include <atlsync.h>  // For CEvent.
#include <set>
#include "gears/base/common/common.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/thread.h"
#include "gears/geolocation/device_data_provider.h"
#include "gears/geolocation/radio_interface_layer_wince.h"

class WinceRadioDataProvider
    : public RadioDataProviderImplBase,
      public Thread {
 public:
  WinceRadioDataProvider();
  virtual ~WinceRadioDataProvider();

  // RadioDataProviderImplBase implementation
  virtual bool GetData(RadioData *data);

  // Used to handle result callbacks from RIL requests.
  void RILResult(DWORD dwCode,
                 HRESULT hrCmdID,
                 const void* lpData,
                 DWORD cbData,
                 DWORD dwParam);

 private:
  // Thread implementation
  virtual void Run();

  bool Init();
  bool IsAllDataAvailable();
  void NotifyListenersIfUpdateAvailable();
  void FireRilCommands();

  RadioData radio_data_;
  RadioData last_radio_data_;
  Mutex data_mutex_;

  // RIL function pointers.
  RILInitializeFunction* ril_initialize_function_;
  RILDeinitializeFunction* ril_deinitialize_function_;
  RILGetCellTowerInfoFunction* ril_get_cell_tower_info_function_;
  RILGetSignalQualityFunction* ril_get_signal_quality_function_;
  RILGetCurrentSystemTypeFunction* ril_get_current_system_type_function_;
  RILGetCurrentOperatorFunction* ril_get_current_operator_function_;

  // Data
  HRIL ril_handle_;
  HRESULT ril_get_cell_tower_info_command_id_;
  HRESULT ril_get_signal_quality_command_id_;
  HRESULT ril_get_current_system_type_command_id_;
  HRESULT ril_get_current_operator_command_id_;

  // Event signalled in response to a RILResult callback.
  CEvent result_event_;

  // Event signalled to shut down the thread that polls the RIL.
  CEvent stop_event_;

  DISALLOW_EVIL_CONSTRUCTORS(WinceRadioDataProvider);
};

#endif  // GEARS_GEOLOCATION_RADIO_DATA_PROVIDER_WINCE_H__
