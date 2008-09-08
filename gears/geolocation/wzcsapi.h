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
// Replicates missing wzcsapi.h. Taken from
// http://msdn.microsoft.com/en-us/library/ms706587(VS.85).aspx

// TODO(steveblock): Change naming convention to follow correct style.

#ifndef GEARS_GEOLOCATION_WZCSAPI_H__
#define GEARS_GEOLOCATION_WZCSAPI_H__

// WZCEnumInterfaces

typedef struct {
  LPWSTR wszGuid;
} INTF_KEY_ENTRY, *PINTF_KEY_ENTRY;

typedef struct {
  DWORD dwNumIntfs;
  PINTF_KEY_ENTRY pIntfs;
} INTFS_KEY_TABLE, *PINTFS_KEY_TABLE;

typedef DWORD (WINAPI *WZCEnumInterfacesFunction)(LPWSTR pSrvAddr,
                                                  PINTFS_KEY_TABLE pIntfs);

// WZCQueryInterface

typedef struct {
  DWORD   dwDataLen;
  LPBYTE  pData;
} RAW_DATA, *PRAW_DATA;

// Experiment shows that padding is required in this struct.
typedef struct {
  LPWSTR wszGuid;
  LPWSTR wszDescr;
  ULONG ulMediaState;
  ULONG ulMediaType;
  ULONG ulPhysicalMediaType;
  INT nInfraMode;
  INT nAuthMode;
  INT nWepStatus;
  UCHAR padding1[14];
  DWORD dwCtlFlags;
  DWORD dwCapabilities;
  RAW_DATA rdSSID;
  RAW_DATA rdBSSID;
  RAW_DATA rdBSSIDList;
  RAW_DATA rdStSSIDList;
  RAW_DATA rdCtrlData;
  BOOL bInitialized;
} INTF_ENTRY, *PINTF_ENTRY;

typedef DWORD(WINAPI *WZCQueryInterfaceFunction)(LPWSTR pSrvAddr,
                                                 DWORD dwInFlags,
                                                 PINTF_ENTRY pIntf,
                                                 LPDWORD pdwOutFlags);

#endif  // GEARS_GEOLOCATION_WZCSAPI_H__
