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

#ifndef GEARS_BASE_COMMON_COMMON_WIN32_H__
#define GEARS_BASE_COMMON_COMMON_WIN32_H__

#include <windows.h>

#ifdef WINCE
// WinCE does not support the Appartment threading model (the use of multiple
// Single Threaded Apartments). Instead, we must use the Free threading model
// and run all threads in the Multi Threaded Appartment. To do this, we define
// the preprocessor symbol _ATL_FREE_THREADED and initialise new threads with
// COINIT_MULTITHREADED.
#define GEARS_COINIT_THREAD_MODEL COINIT_MULTITHREADED
#else
// For Win32 we use the Apartment threading model. To do this, we define the
// preprocessor symbol _ATL_APARTMENT_THREADED and initialise new threads with
// COINIT_APARTMENTTHREADED.
#define GEARS_COINIT_THREAD_MODEL COINIT_APARTMENTTHREADED
#endif

#ifdef WINCE
// WinCE doesn't allow message-only windows (HWND_MESSAGE). Instead, create a
// pop-up window (doesn't require a parent) and don't make visible (default).
const HWND  kMessageOnlyWindowParent = NULL;
const DWORD kMessageOnlyWindowStyle  = WS_POPUP;
#else
const HWND  kMessageOnlyWindowParent = HWND_MESSAGE;
const DWORD kMessageOnlyWindowStyle  = NULL;
#endif

#endif  // GEARS_BASE_COMMON_COMMON_WIN32_H__
