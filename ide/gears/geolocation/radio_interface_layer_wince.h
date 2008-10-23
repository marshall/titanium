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
// This file provides declarations of the constants and functions used by the
// WinCE Radio Interface Layer (RIL) library. Header files for the RIL are not
// available in public WinCE SDKs. However, information about the RIL is
// on the MSDN at http://msdn.microsoft.com/en-us/library/aa920441.aspx.
// Furthermore, the MSDN states that for Windows Embedded, the appropriate
// header file for the RIL is ril.h. This is publicly available online at
// http://nah6.com/~itsme/cvs-xdadevtools/itsutils/libril/ril.h. The data in
// this file was compiled from these two sources.

#ifndef GEARS_GEOLOCATION_RADIO_INTERFACE_LAYER_WINCE_H__
#define GEARS_GEOLOCATION_RADIO_INTERFACE_LAYER_WINCE_H__

typedef HANDLE HRIL;

// This callback function is called when the radio sends an unsolicited
// notification.
typedef void (CALLBACK *RILNOTIFYCALLBACK)(DWORD dwCode,
                                           const void* lpData,
                                           DWORD cbData,
                                           DWORD dwParam);
// This callback function is called to send a return value after an
// asynchronous RIL function call.
typedef void (CALLBACK *RILRESULTCALLBACK)(DWORD dwCode,
                                           HRESULT hrCmdID,
                                           const void* lpData,
                                           DWORD cbData,
                                           DWORD dwParam);
typedef HRESULT (RILInitializeFunction)(DWORD dwIndex,
                                        RILRESULTCALLBACK pfnResult,
                                        RILNOTIFYCALLBACK pfnNotify,
                                        DWORD dwNotificationClasses,
                                        DWORD dwParam,
                                        HRIL* lphRil);
typedef HRESULT (RILDeinitializeFunction)(HRIL hRil);
typedef HRESULT (RILGetCellTowerInfoFunction)(HRIL hRil);
typedef HRESULT (RILGetSignalQualityFunction)(HRIL hRil);
typedef HRESULT (RILGetCurrentSystemTypeFunction)(HRIL hRil);
typedef HRESULT (RILGetCurrentOperatorFunction)(HRIL hRil, DWORD dwFormat);

#define MAXLENGTH_BCCH (48)
#define MAXLENGTH_NMR (16)
#define MAXLENGTH_OPERATOR_LONG (16)
#define MAXLENGTH_OPERATOR_SHORT (8)
#define MAXLENGTH_OPERATOR_NUMERIC (5)
#define MAXLENGTH_OPERATOR_COUNTRY_CODE (2)

#define RIL_NCLASS_FUNCRESULT (0x00000000)
#define RIL_NCLASS_NETWORK (0x00040000)
#define RIL_NCLASS_MISC (0x00400000)
#define RIL_NCLASS_RADIOSTATE (0x00800000)
#define RIL_PARAM_CTI_CELLID (0x00000008)
#define RIL_OPFORMAT_LONG (0x00000001)

struct RILCELLTOWERINFO {
  DWORD cbSize;
  DWORD dwParams;
  DWORD dwMobileCountryCode;
  DWORD dwMobileNetworkCode;
  DWORD dwLocationAreaCode;
  DWORD dwCellID;
  DWORD dwBaseStationID;
  DWORD dwBroadcastControlChannel;
  DWORD dwRxLevel;
  DWORD dwRxLevelFull;
  DWORD dwRxLevelSub;
  DWORD dwRxQuality;
  DWORD dwRxQualityFull;
  DWORD dwRxQualitySub;
  DWORD dwIdleTimeSlot;
  DWORD dwTimingAdvance;
  DWORD dwGPRSCellID;
  DWORD dwGPRSBaseStationID;
  DWORD dwNumBCCH;
  BYTE rgbBCCH[MAXLENGTH_BCCH];
  BYTE rgbNMR[MAXLENGTH_NMR];
};

struct RILSIGNALQUALITY {
  DWORD cbSize;
  DWORD dwParams;
  int nSignalStrength;
  int nMinSignalStrength;
  int nMaxSignalStrength;
  DWORD dwBitErrorRate;
  int nLowSignalStrength;
  int nHighSignalStrength;
};

struct RILOPERATORNAMES {
  DWORD cbSize;
  DWORD dwParams;
  char szLongName[MAXLENGTH_OPERATOR_LONG];
  char szShortName[MAXLENGTH_OPERATOR_SHORT];
  char szNumName[MAXLENGTH_OPERATOR_NUMERIC];
  char szCountryCode[MAXLENGTH_OPERATOR_COUNTRY_CODE];
};

enum RILSYSTEMTYPES {
  RIL_SYSTEMTYPE_NONE = 0,
  RIL_SYSTEMTYPE_IS95A = 0x001,
  RIL_SYSTEMTYPE_IS95B = 0x002,
  RIL_SYSTEMTYPE_1XRTTPACKET = 0x004,
  RIL_SYSTEMTYPE_GSM = 0x008,
  RIL_SYSTEMTYPE_GPRS = 0x010,
  RIL_SYSTEMTYPE_EDGE = 0x020,
  RIL_SYSTEMTYPE_1XEVDOPACKET = 0x040,
  RIL_SYSTEMTYPE_1XEVDVPACKET = 0x080,
  RIL_SYSTEMTYPE_UMTS = 0x100,
  RIL_SYSTEMTYPE_HSDPA = 0x200,
};

#endif  // GEARS_GEOLOCATION_RADIO_INTERFACE_LAYER_WINCE_H__
