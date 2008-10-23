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
// This file contains functions which are common to both the WIN32 and WINCE
// implementations of the wifi data provider.

#ifndef GEARS_GEOLOCATION_WIFI_DATA_PROVIDER_WINDOWS_COMMON_H__
#define GEARS_GEOLOCATION_WIFI_DATA_PROVIDER_WINDOWS_COMMON_H__

#include <vector>

#ifdef WINCE

#include <ntddndis.h>

#else

#include <windows.h>

typedef UCHAR NDIS_802_11_MAC_ADDRESS[6];

#define NDIS_802_11_LENGTH_SSID 32

typedef struct _NDIS_802_11_SSID
{
  ULONG SsidLength;
  UCHAR Ssid[NDIS_802_11_LENGTH_SSID];
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;

typedef LONG NDIS_802_11_RSSI;

// This structure is not quite the same as the WinCE equivalent.
typedef struct _NDIS_WLAN_BSSID {
  UCHAR padding1[4];
  ULONG Length;
  UCHAR padding2[4];
  NDIS_802_11_MAC_ADDRESS MacAddress;
  UCHAR Reserved[2];
  NDIS_802_11_SSID Ssid;
  ULONG Privacy;
  NDIS_802_11_RSSI Rssi;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

typedef struct _NDIS_802_11_BSSID_LIST {
  ULONG NumberOfItems;
  // Following data is an array of NDIS_WLAN_BSSID objects of length
  // NumberOfItems.
  NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;

#endif

struct AccessPointData;

// Extracts access point data from the NDIS_802_11_BSSID_LIST structure and
// appends it to the data vector. Returns the number of access points for which
// data was extracted.
int GetDataFromBssIdList(const NDIS_802_11_BSSID_LIST &bss_id_list,
                         int list_size,
                         std::vector<AccessPointData> *data);

#endif  // GEARS_GEOLOCATION_WIFI_DATA_PROVIDER_WINDOWS_COMMON_H__
