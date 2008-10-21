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
// Windows Vista uses the Native Wifi (WLAN) API for accessing WiFi cards. See
// http://msdn.microsoft.com/en-us/library/ms705945(VS.85).aspx. Windows XP
// Service Pack 3 (and Windows XP Service Pack 2, if upgraded with a hot fix)
// also support a limited version of the WLAN API. See
// http://msdn.microsoft.com/en-us/library/bb204766.aspx. The WLAN API uses
// wlanapi.h, which is not part of the SDK used by Gears, so is replicated
// locally using data from the MSDN.
//
// Windows XP from Service Pack 2 onwards supports the Wireless Zero
// Configuration (WZC) programming interface. See
// http://msdn.microsoft.com/en-us/library/ms706587(VS.85).aspx. This uses the
// header wzcsapi.h, which is not part of the SDK, so is replicated locally with
// data obtained from the MSDN. See
// http://hostap.epitest.fi/wpa_supplicant/devel/driver__ndis_8c-source.html for
// an example of using the WZC. Note that Windows Windows Vista does not support
// WZC.
//
// It appears that it is not possible to access WiFi cards on versions of
// Windows prior to XP Service Pack 2.
//
// As recommended by the MSDN, we use the WLAN API where available, and WZC
// otherwise.

// TODO(cprince): remove platform-specific #ifdef guards when OS-specific
// sources (e.g. WIN32_CPPSRCS) are implemented
#if defined(WIN32) && !defined(WINCE)

#include "gears/geolocation/wifi_data_provider_win32.h"

#include <windows.h>
#include "gears/base/common/string_utils.h"
#include "gears/geolocation/wifi_data_provider_common.h"
#include "gears/geolocation/wifi_data_provider_windows_common.h"

// The time period, in milliseconds, between successive polls of the wifi data.
static const int kPollingInterval = 1000;

// Local function
// Extracts data for an access point and converts to Gears format.
static bool GetNetworkData(const WLAN_BSS_ENTRY &bss_entry,
                           AccessPointData *access_point_data);

// static
template<>
WifiDataProviderImplBase *WifiDataProvider::DefaultFactoryFunction() {
  return new Win32WifiDataProvider();
}


Win32WifiDataProvider::Win32WifiDataProvider()
    : is_first_scan_complete_(false) {
  // Start the polling thread.
  Start();
}

Win32WifiDataProvider::~Win32WifiDataProvider() {
  stop_event_.Signal();
  Join();
}

bool Win32WifiDataProvider::GetData(WifiData *data) {
  assert(data);
  MutexLock lock(&data_mutex_);
  *data = wifi_data_;
  // If we've successfully completed a scan, indicate that we have all of the
  // data we can get.
  return is_first_scan_complete_;
}

// Thread implementation
void Win32WifiDataProvider::Run() {
  // Use the WLAN interface if possible, otherwise use WZC.
  typedef bool (Win32WifiDataProvider::*GetAccessPointDataFunction)(
      std::vector<AccessPointData> *data);
  GetAccessPointDataFunction get_access_point_data_function = NULL;
  HINSTANCE library = LoadLibrary(L"wlanapi");
  if (library) {
    GetWLANFunctions(library);
    get_access_point_data_function =
        &Win32WifiDataProvider::GetAccessPointDataWLAN;
  } else {
    library = LoadLibrary(L"wzcsapi");
    if (library) {
      GetWZCFunctions(library);
      get_access_point_data_function =
          &Win32WifiDataProvider::GetAccessPointDataWZC;
    } else {
      is_first_scan_complete_ = true;
      return;
    }
  }

  // Regularly get the access point data.
  do {
    WifiData new_data;
    if ((this->*get_access_point_data_function)(&new_data.access_point_data)) {
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

  FreeLibrary(library);
}

void Win32WifiDataProvider::GetWZCFunctions(HINSTANCE wzc_library) {
  assert(wzc_library);
  WZCEnumInterfaces_function_ = reinterpret_cast<WZCEnumInterfacesFunction>(
      GetProcAddress(wzc_library, "WZCEnumInterfaces"));
  WZCQueryInterface_function_ = reinterpret_cast<WZCQueryInterfaceFunction>(
      GetProcAddress(wzc_library, "WZCQueryInterface"));
  assert(WZCEnumInterfaces_function_ && WZCQueryInterface_function_);
}

bool Win32WifiDataProvider::GetAccessPointDataWZC(
    std::vector<AccessPointData> *data) {
  assert(data);
  assert(WZCEnumInterfaces_function_);

  // Get the list of interfaces. WZCEnumInterfaces allocates
  // INTFS_KEY_TABLE::pIntfs.
  INTFS_KEY_TABLE interface_list = {0};
  DWORD result = (*WZCEnumInterfaces_function_)(NULL, &interface_list);
  if (result != ERROR_SUCCESS) {
    // This fails if WZC is not running.
    LocalFree(interface_list.pIntfs);
    return false;
  }

  // Go through the list of interfaces and get the data for each.
  for (int i = 0; i < static_cast<int>(interface_list.dwNumIntfs); ++i) {
    GetInterfaceDataWZC(interface_list.pIntfs[i].wszGuid, data);
  }

  LocalFree(interface_list.pIntfs);
  return true;
}

// This value was taken from the WinCE equivalent at
// http://msdn.microsoft.com/en-us/library/aa448300.aspx
#define INTF_BSSIDLIST (0x04000000)

void Win32WifiDataProvider::GetInterfaceDataWZC(
    const char16 *interface_guid,
    std::vector<AccessPointData> *data) {
  assert(data);
  assert(WZCQueryInterface_function_);

  INTF_ENTRY interface_data = {0};
  interface_data.wszGuid = const_cast<char16*>(interface_guid);
  DWORD retrieved_fields;
  DWORD result = (*WZCQueryInterface_function_)(NULL,
                                                INTF_BSSIDLIST,
                                                &interface_data,
                                                &retrieved_fields);
  if (result != ERROR_SUCCESS ||
      !(retrieved_fields & INTF_BSSIDLIST) ||
      interface_data.rdBSSIDList.dwDataLen == 0) {
    return;
  }

  // Based on the WinCE equivalent at
  // http://msdn.microsoft.com/en-us/library/aa448306.aspx,
  // INTF_ENTRY::rdBSSIDList::pData points to a NDIS_802_11_BSSID_LIST object.
  // The first member is of type ULONG, and must be valid.
  assert(interface_data.rdBSSIDList.dwDataLen >= sizeof(ULONG));

  // Cast the data to a list of BSS IDs.
  NDIS_802_11_BSSID_LIST *bss_id_list =
      reinterpret_cast<NDIS_802_11_BSSID_LIST*>(
          interface_data.rdBSSIDList.pData);

  GetDataFromBssIdList(*bss_id_list,
                       interface_data.rdBSSIDList.dwDataLen,
                       data);

  LocalFree(interface_data.rdBSSIDList.pData);
}

void Win32WifiDataProvider::GetWLANFunctions(HINSTANCE wlan_library) {
  assert(wlan_library);
  WlanOpenHandle_function_ = reinterpret_cast<WlanOpenHandleFunction>(
      GetProcAddress(wlan_library, "WlanOpenHandle"));
  WlanEnumInterfaces_function_ = reinterpret_cast<WlanEnumInterfacesFunction>(
      GetProcAddress(wlan_library, "WlanEnumInterfaces"));
  WlanGetNetworkBssList_function_ =
      reinterpret_cast<WlanGetNetworkBssListFunction>(
      GetProcAddress(wlan_library, "WlanGetNetworkBssList"));
  WlanFreeMemory_function_ = reinterpret_cast<WlanFreeMemoryFunction>(
      GetProcAddress(wlan_library, "WlanFreeMemory"));
  WlanCloseHandle_function_ = reinterpret_cast<WlanCloseHandleFunction>(
      GetProcAddress(wlan_library, "WlanCloseHandle"));
  assert(WlanOpenHandle_function_ &&
         WlanEnumInterfaces_function_ &&
         WlanGetNetworkBssList_function_ &&
         WlanFreeMemory_function_ &&
         WlanCloseHandle_function_);
}

bool Win32WifiDataProvider::GetAccessPointDataWLAN(
    std::vector<AccessPointData> *data) {
  assert(data);

  // Get the handle to the WLAN API.
  DWORD negotiated_version;
  HANDLE wlan_handle;
  // We could be executing on either Windows XP or Windows Vista, so use the
  // lower version of the client WLAN API. It seems that the negotiated version
  // is the Vista version irrespective of what we pass!
  static const int kXpWlanClientVersion = 1;
  if ((*WlanOpenHandle_function_)(kXpWlanClientVersion,
                                  NULL,
                                  &negotiated_version,
                                  &wlan_handle) != ERROR_SUCCESS) {
    return false;
  }

  // Get the list of interfaces. WlanEnumInterfaces allocates interface_list.
  WLAN_INTERFACE_INFO_LIST *interface_list;
  if ((*WlanEnumInterfaces_function_)(wlan_handle, NULL, &interface_list) !=
      ERROR_SUCCESS) {
    return false;
  }

  // Go through the list of interfaces and get the data for each.
  for (int i = 0; i < static_cast<int>(interface_list->dwNumberOfItems); ++i) {
    if (!GetInterfaceDataWLAN(wlan_handle,
                              interface_list->InterfaceInfo[i].InterfaceGuid,
                              data)) {
      return false;
    }
  }

  // Free interface_list.
  (*WlanFreeMemory_function_)(interface_list);

  // Close the handle.
  if ((*WlanCloseHandle_function_)(wlan_handle, NULL) != ERROR_SUCCESS) {
    return false;
  }

  return true;
}

// Appends the data for a single interface to the data vector. Returns the
// number of access points found, or -1 on error.
int Win32WifiDataProvider::GetInterfaceDataWLAN(
    const HANDLE wlan_handle,
    const GUID &interface_id,
    std::vector<AccessPointData> *data) {
  assert(data);
  // WlanGetNetworkBssList allocates bss_list.
  WLAN_BSS_LIST *bss_list;
  if ((*WlanGetNetworkBssList_function_)(wlan_handle,
                                         &interface_id,
                                         NULL,   // Use all SSIDs.
                                         DOT11_BSS_TYPE_UNUSED,
                                         false,  // bSecurityEnabled - unused
                                         NULL,   // reserved
                                         &bss_list) != ERROR_SUCCESS) {
    return -1;
  }

  int found = 0;
  for (int i = 0; i < static_cast<int>(bss_list->dwNumberOfItems); ++i) {
    AccessPointData access_point_data;
    if (GetNetworkData(bss_list->wlanBssEntries[i], &access_point_data)) {
      ++found;
      data->push_back(access_point_data);
    }
  }

  (*WlanFreeMemory_function_)(bss_list);

  return found;
}

// Local function

static bool GetNetworkData(const WLAN_BSS_ENTRY &bss_entry,
                           AccessPointData *access_point_data) {
  assert(access_point_data);
  access_point_data->mac_address = MacAddressAsString16(bss_entry.dot11Bssid);
  access_point_data->radio_signal_strength = bss_entry.lRssi;
  // bss_entry.dot11Ssid.ucSSID is not null-terminated.
  UTF8ToString16(reinterpret_cast<const char*>(bss_entry.dot11Ssid.ucSSID),
                 static_cast<ULONG>(bss_entry.dot11Ssid.uSSIDLength),
                 &access_point_data->ssid);
  // TODO(steveblock): Is it possible to get the following?
  //access_point_data->signal_to_noise
  //access_point_data->age
  //access_point_data->channel
  return true;
}

#endif  // WIN32 && !WINCE
