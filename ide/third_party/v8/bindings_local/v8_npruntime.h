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

#ifndef GEARS_V8_BINDINGS_V8_NPRUNTIME_H__
#define GEARS_V8_BINDINGS_V8_NPRUNTIME_H__

#include "npapi.h"
#include "npruntime.h"

// These are the NPAPI runtime functions that are normally exposed by the
// browser to be used by a plugin.  In this case, they are exposed by the V8
// binding (in our plugin) to be used by our plugin.

void *V8_NPN_MemAlloc(uint32 size);
void V8_NPN_MemFree(void *ptr);
void V8_NPN_ReleaseVariantValue(NPVariant *variant);
NPIdentifier V8_NPN_GetStringIdentifier(const NPUTF8 *name);
void V8_NPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount, NPIdentifier *identifiers);
NPIdentifier V8_NPN_GetIntIdentifier(int32_t intid);
bool V8_NPN_IdentifierIsString(NPIdentifier identifier);
NPUTF8 *V8_NPN_UTF8FromIdentifier(NPIdentifier identifier);
int32_t V8_NPN_IntFromIdentifier(NPIdentifier identifier);    
NPObject *V8_NPN_CreateObject(NPP npp, NPClass *aClass);
NPObject *V8_NPN_RetainObject(NPObject *obj);
void V8_NPN_ReleaseObject(NPObject *obj);
void V8_NPN_DeallocateObject(NPObject *obj);
bool V8_NPN_Invoke(NPP npp, NPObject *npobj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool V8_NPN_InvokeDefault(NPP npp, NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool V8_NPN_Evaluate(NPP npp, NPObject *npobj, NPString *script, NPVariant *result);
bool V8_NPN_GetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName, NPVariant *result);
bool V8_NPN_SetProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName, const NPVariant *value);
bool V8_NPN_RemoveProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName);
bool V8_NPN_HasProperty(NPP npp, NPObject *npobj, NPIdentifier propertyName);
bool V8_NPN_HasMethod(NPP npp, NPObject *npobj, NPIdentifier methodName);
void V8_NPN_SetException(NPObject *obj, const NPUTF8 *message);
bool V8_NPN_Enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
                      uint32_t *count);
NPError V8_NPN_GetValue(NPP npp, NPNVariable variable, void *value);


// The following routines allow the browser to aggressively cleanup NPObjects
// on a per plugin basis.  All NPObjects used through the NPRuntime API should
// be "registered" while they are alive.  After an object has been
// deleted, it is possible for Javascript to have a reference to that object
// which has not yet been garbage collected.  Javascript access to NPObjects
// will reference this registry to determine if the object is accessible or 
// not.

// Windows introduces an additional complication for objects created by the
// plugin.  Plugins load inside of a DLL.  Each DLL has it's own heap.  If
// the browser unloads the plugin DLL, all objects created within the DLL's
// heap instantly become invalid.  Normally, when WebKit drops the reference
// on the top-level plugin object, it tells the plugin manager that the
// plugin can be destroyed, which can unload the DLL.  So, we must eliminate
// all pointers to any object ever created by the plugin.

// We generally associate NPObjects with an owner.  The owner of an NPObject
// is an NPObject which, when destroyed, also destroys all objects it owns.
// For example, if an NPAPI plugin creates 10 sub-NPObjects, all 11 objects
// (the NPAPI plugin + its 10 sub-objects) should become inaccessible 
// simultaneously.

// The ownership hierarchy is flat, and not a tree.  Imagine the following
// object creation:
//     PluginObject
//          |
//          +-- Creates -----> Object1
//                                |
//                                +-- Creates -----> Object2
//
// PluginObject will be the "owner" for both Object1 and Object2.

// Register an NPObject with the runtime.  If the owner is NULL, the
// object is treated as an owning object.  If owner is not NULL,
// this object will be registered as owned by owner's top-level owner.
void _NPN_RegisterObject(NPObject* obj, NPObject* owner);

// Unregister an NPObject with the runtime.  If obj is an owning
// object, this call will also unregister all of the owned objects.
void _NPN_UnregisterObject(NPObject* obj);

// Check to see if an object is registered with the runtime.
// Return true if registered, false otherwise.
bool _NPN_IsAlive(NPObject* obj);

#endif
