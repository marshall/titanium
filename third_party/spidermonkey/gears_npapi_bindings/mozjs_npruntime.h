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

#ifndef GEARS_WORKERPOOL_SAFARI_MOZJS_NPRUNTIME_H__
#define GEARS_WORKERPOOL_SAFARI_MOZJS_NPRUNTIME_H__ 

#if BROWSER_WEBKIT
#include <WebKit/npapi.h>
#include <WebKit/npruntime.h>
#else
#include "third_party/npapi/npapi.h"
#include "third_party/npapi/npruntime.h"
#endif

// These are the NPAPI runtime functions that are normally exposed by the
// browser to be used by a plugin.  Here we expose Spidermonkey versions for
// use by Gears.

namespace SpiderMonkeyNPAPIBindings {
void *NPN_MemAlloc(uint32 size);
void NPN_MemFree(void *ptr);
void NPN_ReleaseVariantValue(NPVariant *variant);
NPIdentifier NPN_GetStringIdentifier(const NPUTF8 *name);
void NPN_GetStringIdentifiers(const NPUTF8 **names, 
                              int32_t nameCount, 
                              NPIdentifier *identifiers);
NPIdentifier NPN_GetIntIdentifier(int32_t intid);
bool NPN_IdentifierIsString(NPIdentifier identifier);
NPUTF8 *NPN_UTF8FromIdentifier(NPIdentifier identifier);
int32_t NPN_IntFromIdentifier(NPIdentifier identifier);    
NPObject *NPN_CreateObject(NPP npp, NPClass *aClass);
NPObject *NPN_RetainObject(NPObject *obj);
void NPN_ReleaseObject(NPObject *obj);
void NPN_DeallocateObject(NPObject *obj);
bool NPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName, 
                const NPVariant *args, uint32_t argCount, NPVariant *result);
bool NPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args, 
                       uint32_t argCount, NPVariant *result);
bool NPN_Evaluate(NPP npp, NPObject *npobj, NPString *script, 
                  NPVariant *result);
bool NPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName, 
                     NPVariant *result);
bool NPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName, 
                     const NPVariant *value);
bool NPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName);
bool NPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName);
bool NPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName);
void NPN_SetException(NPObject *obj, const NPUTF8 *message);
NPError NPN_GetValue(NPP npp, NPNVariable variable, void *value);
bool NPN_Enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
                   uint32_t *count);
} // namespace SpiderMonkeyNPAPIBindings

#endif // GEARS_WORKERPOOL_SAFARI_MOZJS_NPRUNTIME_H__
