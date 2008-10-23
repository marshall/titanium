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

#include "gears/geolocation/wifi_data_provider_wince.h"

#include <Iphlpapi.h>  // For GetAdaptersInfo()
#include <ntddndis.h>  // Must be included before nuiouser.h
#include <nuiouser.h>  // For NDISUIO stuff
#include <windows.h>
#include <winioctl.h>  // For IOCTL_NDISUIO_QUERY_OID_VALUE
#include "gears/base/common/string_utils.h"  // For UTF8ToString16()
#include "gears/geolocation/wifi_data_provider_windows_common.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// The time period, in milliseconds, between successive polls of the wifi data.
static const int kPollingInterval = 1000;

// Local function.
static bool GetAccessPointData(const HANDLE &ndis_handle,
                               std::vector<AccessPointData> *access_points);

// static
template<>
WifiDataProviderImplBase *WifiDataProvider::DefaultFactoryFunction() {
  return new WinceWifiDataProvider();
}


WinceWifiDataProvider::WinceWifiDataProvider()
    : is_first_scan_complete_(false) {
  // Start the polling thread.
  Start();
}

WinceWifiDataProvider::~WinceWifiDataProvider() {
  stop_event_.Signal();
  Join();
}

bool WinceWifiDataProvider::GetData(WifiData *data) {
  assert(data);
  MutexLock lock(&data_mutex_);
  *data = wifi_data_;
  // If we've successfully completed a scan, indicate that we have all of the
  // data we can get.
  return is_first_scan_complete_;
}

// Thread implementation

void WinceWifiDataProvider::Run() {
  // Get the Network Driver Interface Specification (NDIS) handle for wireless
  // devices. NDIS defines a standard API for Network Interface Cards (NICs).
  // NDIS User Mode I/O (NDISUIO) is an NDIS protocol driver which offers
  // support for wireless devices.
  HANDLE ndis_handle = CreateFile(NDISUIO_DEVICE_NAME, GENERIC_READ,
                                  FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_READONLY, NULL);
  if (INVALID_HANDLE_VALUE == ndis_handle) {
    is_first_scan_complete_ = true;
    return;
  }

  // Regularly get the access point data.
  do {
    WifiData new_data;
    if (GetAccessPointData(ndis_handle, &new_data.access_point_data)) {
      bool update_available;
      data_mutex_.Lock();
      if (update_available = !wifi_data_.Matches(new_data)) {
        wifi_data_ = new_data;
        is_first_scan_complete_ = true;
      }
      data_mutex_.Unlock();
      if (update_available) {
        NotifyListeners();
      }
    }
  } while (!stop_event_.WaitWithTimeout(kPollingInterval));

  CloseHandle(ndis_handle);
}

// Local functions.

// Issues the specified OID query for the specified device name. The returned
// data, which is query-specific, is stored in oid_data.
static bool QueryOid(const HANDLE &ndis_handle,
                     const std::string16 &device_name,
                     DWORD oid,
                     std::vector<uint8> *oid_data) {
  assert(oid_data);

  // Allocate a buffer for the query. 8192 seems to be enough to hold 50 access
  // points.
  static const int kOidBufferSize = 8192;
  UCHAR oid_buffer[kOidBufferSize];

  // Form the query parameters.
  NDISUIO_QUERY_OID *query = reinterpret_cast<NDISUIO_QUERY_OID*>(oid_buffer);
  query->ptcDeviceName = const_cast<char16*>(device_name.c_str());
  query->Oid = oid;

  // Make the query. This will fail if the adapter is not a wireless adapter.
  // This is not a failure for our purposes. We simply don't return any data.
  DWORD result_size = 0;
  BOOL result = DeviceIoControl(ndis_handle,
                                IOCTL_NDISUIO_QUERY_OID_VALUE,
                                query,
                                sizeof(NDISUIO_QUERY_OID),
                                query,
                                sizeof(oid_buffer),
                                &result_size,
                                NULL);
  if (!result) {
#ifdef DEBUG
    std::string16 error =
        STRING16(L"QueryOid(): Call to DeviceIoControl() failed: ");
    error += GetLastErrorString();
    error += STRING16(L".\n");
    std::string error_utf8;
    if (String16ToUTF8(error.c_str(), error.size(), &error_utf8)) {
      LOG((error_utf8.c_str()));
    } else {
      LOG(("QueryOid(): Call to DeviceIoControl() failed.\n"));
    }
#endif
    return true;
  }
  oid_data->assign(&query->Data[0], &query->Data[0] + result_size);
  return true;
}

// Gets the data for all access points for the specified card and appends to the
// supplied vector. Returns the number of access points found, or -1 on failure.
static int GetCardAccessPointData(const HANDLE &ndis_handle,
                                  const std::string16 &card_name,
                                  std::vector<AccessPointData> *access_points) {
  assert(access_points);
  // Make an OID query to get the list of wifi Basic Service Set (BSS) IDs.
  //
  // Note that we rely on the connection manager to perform regular scans to
  // keep the list fresh. (OID_802_11_BSSID_LIST_SCAN, which refreshes
  // the list, requires a "Set OID" to the device, which can only be done from
  // kernel mode.) For general information on scanning 802.11 networks, see
  // http://msdn.microsoft.com/en-us/library/aa504190.aspx.
  std::vector<uint8> oid_response_data;
  if (!QueryOid(ndis_handle, card_name, OID_802_11_BSSID_LIST,
                &oid_response_data)) {
    return -1;
  }
  // The data may be empty, even if QueryOid succeeds.
  if (oid_response_data.size() == 0) {
    return 0;
  }
  // Cast the data to a list of BSS IDs.
  NDIS_802_11_BSSID_LIST *bss_id_list =
      reinterpret_cast<NDIS_802_11_BSSID_LIST*>(&oid_response_data[0]);

  return GetDataFromBssIdList(*bss_id_list,
                              oid_response_data.size(),
                              access_points);
}

static bool GetAccessPointData(const HANDLE &ndis_handle,
                               std::vector<AccessPointData> *access_points) {
  assert(access_points);
  // Get the list of adapters. First determine the buffer size.
  ULONG buffer_size = 0;
  DWORD result = GetAdaptersInfo(NULL, &buffer_size);
  assert(ERROR_BUFFER_OVERFLOW == result);
  // Allocate buffer with correct size.
  scoped_array<uint8> buffer(new uint8[buffer_size]);
  IP_ADAPTER_INFO *adapter_info =
      reinterpret_cast<IP_ADAPTER_INFO*>(buffer.get());
  if (GetAdaptersInfo(adapter_info, &buffer_size) != ERROR_SUCCESS) {
    return false;
  }
  // Walk through the list of adapters.
  while (adapter_info) {
    std::string16 adapter_name;
    if (UTF8ToString16(adapter_info->AdapterName, &adapter_name)) {
      GetCardAccessPointData(ndis_handle, adapter_name, access_points);
    }
    adapter_info = adapter_info->Next;
  }
  return true;
}

#endif  // WINCE
