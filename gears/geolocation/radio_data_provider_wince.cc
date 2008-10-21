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

// TODO(cprince): remove platform-specific #ifdef guards when OS-specific
// sources (e.g. WIN32_CPPSRCS) are implemented
#ifdef WINCE

#include "gears/geolocation/radio_data_provider_wince.h"

#include "gears/base/common/basictypes.h"  // For kint32min
#include "gears/base/common/stopwatch.h"  // For GetCurrentTimeMillis
#include "gears/base/common/string_utils.h"  // For UTF8ToString16

// The time period, in milliseconds, between successive sets of RIL requests.
static const int kRILPollingInterval = 1000;

// Local functions.

// Handlers for result and notify callbacks from RIL.
static void RILNotifyCallback(DWORD dwCode,
                              const void* lpData,
                              DWORD cbData,
                              DWORD dwParam);
static void RILResultCallback(DWORD dwCode,
                              HRESULT hrCmdID,
                              const void* lpData,
                              DWORD cbData,
                              DWORD dwParam);
// We use kint32min to represent unknown values. The device implementation uses
// zero. This method converts to our format.
static int FromDeviceDataFormat(const int &value);

// static
template<>
RadioDataProviderImplBase *RadioDataProvider::DefaultFactoryFunction() {
  return new WinceRadioDataProvider();
}


WinceRadioDataProvider::WinceRadioDataProvider() {
  if (!Init()) {
    assert(false);
  }
  Start();
}

WinceRadioDataProvider::~WinceRadioDataProvider() {
  stop_event_.Set();
  Join();
}

bool WinceRadioDataProvider::GetData(RadioData *data) {
  assert(data);
  MutexLock lock(&data_mutex_);
  *data = radio_data_;
  return IsAllDataAvailable();
}

// Thread implementation.
void WinceRadioDataProvider::Run() {
  // Get the function pointers from the DLL.
  HINSTANCE ril_library = LoadLibrary(L"ril");
  if (!ril_library) {
    LOG16((L"WinceRadioDataProvider::Run() : Failed to load ril library.\n"));
    return;
  }
  ril_initialize_function_ = reinterpret_cast<RILInitializeFunction*>(
      GetProcAddress(ril_library, L"RIL_Initialize"));
  ril_deinitialize_function_ = reinterpret_cast<RILDeinitializeFunction*>(
      GetProcAddress(ril_library, L"RIL_Deinitialize"));
  ril_get_cell_tower_info_function_ =
      reinterpret_cast<RILGetCellTowerInfoFunction*>(
          GetProcAddress(ril_library, L"RIL_GetCellTowerInfo"));
  ril_get_signal_quality_function_ =
      reinterpret_cast<RILGetSignalQualityFunction*>(
          GetProcAddress(ril_library, L"RIL_GetSignalQuality"));
  ril_get_current_system_type_function_ =
      reinterpret_cast<RILGetCurrentSystemTypeFunction*>(
          GetProcAddress(ril_library, L"RIL_GetCurrentSystemType"));
  ril_get_current_operator_function_ =
      reinterpret_cast<RILGetCurrentOperatorFunction*>(
          GetProcAddress(ril_library, L"RIL_GetCurrentOperator"));

  if (!ril_initialize_function_ ||
      !ril_deinitialize_function_ ||
      !ril_get_cell_tower_info_function_ ||
      !ril_get_signal_quality_function_ ||
      !ril_get_current_system_type_function_ ||
      !ril_get_current_operator_function_) {
    LOG16((L"WinceRadioDataProvider::Run() : Failed to get RIL functions.\n"));
    // If we succesfully loaded the library, this should never fail.
    assert(false);
    return;
  }

  // Positive values are used to store the command id, negative values indicate
  // an error. We also use zero to indicate that the last request of this type
  // has finished processing.
  ril_get_cell_tower_info_command_id_     = 0;
  ril_get_current_system_type_command_id_ = 0;
  ril_get_signal_quality_command_id_      = 0;
  ril_get_current_operator_command_id_    = 0;

  if (FAILED(ril_initialize_function_(
                 1,                              // RIL port index
                 RILResultCallback,              // Result callback
                 RILNotifyCallback,              // Notify callback
                 RIL_NCLASS_FUNCRESULT |
                 RIL_NCLASS_NETWORK |
                 RIL_NCLASS_MISC |
                 RIL_NCLASS_RADIOSTATE,          // Notification classes
                 reinterpret_cast<DWORD>(this),  // Callback param
                 &ril_handle_))) {               // Returned handle
    LOG16((L"WinceRadioDataProvider::Run() : Failed to initialize RIL.\n"));
    // If we succesfully loaded the library, this should never fail.
    assert(false);
    return;
  }

  HANDLE events[] = {stop_event_, result_event_};

  while (true) {
    // Fire commands.
    FireRilCommands();

    // Wait for a RIL callback. Note that if calls in FireRilCommands fail, we
    // may not get a callback. If we get a stop_event_, we quit the loop and
    // deinitalize the RIL library, which cancels any pending callbacks.
    if (WaitForMultipleObjects(ARRAYSIZE(events),
                               events,
                               false,
                               INFINITE) == WAIT_OBJECT_0) {
      break;
    }
    // Pause for a set time period, listening for the stop event.
    if (WaitForSingleObject(stop_event_, kRILPollingInterval) ==
        WAIT_OBJECT_0) {
      break;
    }
  }

  // Deinitialise.
  ril_deinitialize_function_(ril_handle_);
  FreeLibrary(ril_library);
}

// Other methods

bool WinceRadioDataProvider::Init() {
  if (!result_event_.Create(NULL, FALSE, FALSE, NULL)) {
    return false;
  }
  if (!stop_event_.Create(NULL, FALSE, FALSE, NULL)) {
    return false;
  }
  return true;
}

bool WinceRadioDataProvider::IsAllDataAvailable() {
  return radio_data_.cell_data.size() == 1 &&
      -1 != radio_data_.cell_data[0].cell_id &&
      -1 != radio_data_.cell_data[0].location_area_code &&
      -1 != radio_data_.cell_data[0].mobile_network_code &&
      -1 != radio_data_.cell_data[0].mobile_country_code &&
      -1 != radio_data_.cell_data[0].radio_signal_strength &&
      !radio_data_.carrier.empty();
}

void WinceRadioDataProvider::NotifyListenersIfUpdateAvailable() {
  bool update_available = false;
  data_mutex_.Lock();
  if (!radio_data_.Matches(last_radio_data_)) {
    last_radio_data_ = radio_data_;
    update_available = true;
  }
  // Unlock the mutex so that the callback can use methods that lock this mutex.
  data_mutex_.Unlock();
  if (update_available) {
    NotifyListeners();
  }
}

void WinceRadioDataProvider::FireRilCommands() {
  // We only fire commands that we're not currently waiting on. If a function
  // call fails, we continue working, but will not call that function again.
  // Note that all of these function calls fail on the emulator.
  if (0 == ril_get_cell_tower_info_command_id_) {
    ril_get_cell_tower_info_command_id_ =
      ril_get_cell_tower_info_function_(ril_handle_);
#ifdef DEBUG
    if (ril_get_cell_tower_info_command_id_ < 0) {
      LOG16((L"WinceDeviceDataProvider::FireRilCommands() : "
             L"Call to ril_get_cell_tower_info_function_ failed.\n"));
    }
#endif
  }
  if (0 == ril_get_current_operator_command_id_) {
    ril_get_current_operator_command_id_ =
      ril_get_current_operator_function_(ril_handle_, RIL_OPFORMAT_LONG);
#ifdef DEBUG
    if (ril_get_current_operator_command_id_ < 0) {
      LOG16((L"WinceDeviceDataProvider::FireRilCommands() : "
             L"Call to ril_get_current_operator_function_ failed.\n"));
    }
#endif
  }
  if (0 == ril_get_signal_quality_command_id_) {
    ril_get_signal_quality_command_id_ =
        ril_get_signal_quality_function_(ril_handle_);
#ifdef DEBUG
    if (ril_get_signal_quality_command_id_ < 0) {
      LOG16((L"WinceDeviceDataProvider::FireRilCommands() : "
             L"Call to ril_get_signal_quality_function_ failed.\n"));
    }
#endif
  }
  if (0 == ril_get_current_system_type_command_id_) {
    ril_get_current_system_type_command_id_ =
        ril_get_current_system_type_function_(ril_handle_);
#ifdef DEBUG
    if (ril_get_current_system_type_command_id_ < 0) {
      LOG16((L"WinceDeviceDataProvider::FireRilCommands() : "
             L"Call to ril_get_current_system_type_function_ failed.\n"));
    }
#endif
  }
}

void WinceRadioDataProvider::RILResult(DWORD dwCode,
                                       HRESULT hrCmdID,
                                       const void* lpData,
                                       DWORD cbData,
                                       DWORD dwParam) {
  data_mutex_.Lock();
  // WinCE provides only one set of cell ID data.
  if (radio_data_.cell_data.empty()) {
    CellData cell_data;
    radio_data_.cell_data.push_back(cell_data);
  }
  if (hrCmdID == ril_get_cell_tower_info_command_id_) {
    ril_get_cell_tower_info_command_id_ = 0;
    const RILCELLTOWERINFO* cell_tower_info =
        static_cast<const RILCELLTOWERINFO*>(lpData);
    if (cell_tower_info->dwParams & RIL_PARAM_CTI_CELLID) {
      assert(radio_data_.cell_data.size() == 1);
      radio_data_.cell_data[0].cell_id =
          FromDeviceDataFormat(cell_tower_info->dwCellID);
      radio_data_.cell_data[0].location_area_code =
          FromDeviceDataFormat(cell_tower_info->dwLocationAreaCode);
      radio_data_.cell_data[0].mobile_network_code =
          FromDeviceDataFormat(cell_tower_info->dwMobileNetworkCode);
      radio_data_.cell_data[0].mobile_country_code =
          FromDeviceDataFormat(cell_tower_info->dwMobileCountryCode);
    }
  } else if (hrCmdID == ril_get_signal_quality_command_id_) {
    ril_get_signal_quality_command_id_ = 0;
    const RILSIGNALQUALITY* ril_signal_quality =
        static_cast<const RILSIGNALQUALITY*>(lpData);
    assert(radio_data_.cell_data.size() == 1);
    radio_data_.cell_data[0].radio_signal_strength =
        FromDeviceDataFormat(ril_signal_quality->nSignalStrength);
  } else if (hrCmdID == ril_get_current_system_type_command_id_) {
    ril_get_current_system_type_command_id_ = 0;
    const int32 ril_system_type = *reinterpret_cast<const int32*>(lpData);
    if (ril_system_type & (RIL_SYSTEMTYPE_GSM |
                           RIL_SYSTEMTYPE_GPRS |
                           RIL_SYSTEMTYPE_EDGE)) {
      radio_data_.radio_type = RADIO_TYPE_GSM;
    } else if (ril_system_type & (RIL_SYSTEMTYPE_UMTS |
                                  RIL_SYSTEMTYPE_HSDPA)) {
      radio_data_.radio_type = RADIO_TYPE_WCDMA;
    } else if (ril_system_type & (RIL_SYSTEMTYPE_1XEVDOPACKET |
                                  RIL_SYSTEMTYPE_1XEVDVPACKET)) {
      radio_data_.radio_type = RADIO_TYPE_CDMA;
    } else {
      radio_data_.radio_type = RADIO_TYPE_UNKNOWN;
    }
  } else if (hrCmdID == ril_get_current_operator_command_id_) {
    ril_get_current_operator_command_id_ = 0;
    const RILOPERATORNAMES* ril_operator_names =
        reinterpret_cast<const RILOPERATORNAMES*>(lpData);
    std::string carrier = ril_operator_names->szLongName;
    UTF8ToString16(carrier.c_str(), carrier.size(), &radio_data_.carrier);
  }

  // It's possible that the cell data entry is uninitialized, in which case we
  // should remove it from the radio data object to keep the network request
  // clean.
  CellData empty_cell_data;
  if (radio_data_.cell_data[0].Matches(empty_cell_data)) {
    radio_data_.cell_data.clear();
  }
    
  data_mutex_.Unlock();
  SetEvent(result_event_);
  NotifyListenersIfUpdateAvailable();
}

// Local functions.

static void RILNotifyCallback(DWORD dwCode, const void* lpData, DWORD cbData,
                              DWORD dwParam) {
  // We don't use these callbacks, but we have to register for them.
}

static void RILResultCallback(DWORD dwCode, HRESULT hrCmdID, const void* lpData,
                              DWORD cbData, DWORD dwParam) {
  WinceRadioDataProvider *self =
      reinterpret_cast<WinceRadioDataProvider*>(dwParam);
  self->RILResult(dwCode, hrCmdID, lpData, cbData, dwParam);
}

static int FromDeviceDataFormat(const int &value) {
  return value ? value : kint32min;
}

#endif  // WINCE
