// Copyright 2006, Google Inc.
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
// Exposes XPCOM functions that do not exist in static libs and must be
// dynamically loaded at runtime.

#ifndef GEARS_BASE_FIREFOX_XPCOM_DYNAMIC_LOAD_H__
#define GEARS_BASE_FIREFOX_XPCOM_DYNAMIC_LOAD_H__
#if BROWSER_FF3
#include <gecko_internal/nscore.h>
#endif
#include <gecko_internal/xptcall.h>

// global function pointers that get initialized at load time
#if BROWSER_FF2
typedef XPTC_PUBLIC_API(nsresult) (*XPTC_InvokeByIndex_Type)(
#else
typedef nsresult (*XPTC_InvokeByIndex_Type)(
#endif
    nsISupports* that, PRUint32 methodIndex,
    PRUint32 paramCount, nsXPTCVariant* params);
extern XPTC_InvokeByIndex_Type XPTC_InvokeByIndex_DynLoad;


#endif // GEARS_BASE_FIREFOX_XPCOM_DYNAMIC_LOAD_H__
