// Copyright 2007, Google Inc.
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

#ifndef GEARS_BASE_NPAPI_MODULE_H__
#define GEARS_BASE_NPAPI_MODULE_H__

#include <WebKit/npapi.h>
#include <WebKit/npruntime.h>
#include <WebKit/npfunctions.h>
#ifdef BROWSER_WEBKIT
#include "npapi_patches.h" 
#endif

#ifdef BROWSER_WEBKIT
// Work around a bug in the WebKit NPAPI headers, see 
// gears/base/safari/npapi_patches.h for details.
static GearsNPNetscapeFuncs g_browser_funcs;
#else
static NPNetscapeFuncs g_browser_funcs;
#endif

void AllowNPInit(bool allow);

// Called when the given instance is being destroyed.  Implemented in
// js_runner_np.cc.
void NotifyNPInstanceDestroyed(NPP instance);

#endif  // GEARS_BASE_NPAPI_MODULE_H__
