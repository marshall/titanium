/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// Utility functions for SpiderMonkey NPAPI bindings adapted from 
// firefox 3.0b5 source tarball 
// mozilla/modules/plugin/base/src/nsJSNPRuntime.cpp & ns4xPlugin.cpp
#ifndef GEARS_WORKERPOOL_SAFARI_MOZJS_NPRUNTIME_UTILS_H__
#define GEARS_WORKERPOOL_SAFARI_MOZJS_NPRUNTIME_UTILS_H__ 

#if BROWSER_WEBKIT
#include <WebKit/npapi.h>
#include <WebKit/npruntime.h>
#else
#include "third_party/npapi/npapi.h"
#include "third_party/npapi/npruntime.h"
#endif

#include "gears/base/common/common.h"
#include "third_party/spidermonkey/gears_include/mozjs_api.h"

namespace SpiderMonkeyNPAPIBindings {

inline const jschar *String16ToJschar(std::string16 &str) {
  return static_cast<const jschar *>(str.c_str());
}

// Mozilla's NPAPI bindings define an NPClass structure that is larger than
// the one defined by WebKit, however since everything is stored in pointers
// this doesn't really matter, here we define the Netscape version.
// We need to be careful to use this whenevera allocating or freeing memory
// in the internal implementation.
typedef bool (*NPEnumerationFunctionPtr)(NPObject *npobj, NPIdentifier **value,
                                         uint32_t *count);
typedef bool (*NPConstructFunctionPtr)(NPObject *npobj,
                                       const NPVariant *args,
                                       uint32_t argCount,
                                       NPVariant *result);
typedef struct
{
    uint32_t structVersion;
    NPAllocateFunctionPtr allocate;
    NPDeallocateFunctionPtr deallocate;
    NPInvalidateFunctionPtr invalidate;
    NPHasMethodFunctionPtr hasMethod;
    NPInvokeFunctionPtr invoke;
    NPInvokeDefaultFunctionPtr invokeDefault;
    NPHasPropertyFunctionPtr hasProperty;
    NPGetPropertyFunctionPtr getProperty;
    NPSetPropertyFunctionPtr setProperty;
    NPRemovePropertyFunctionPtr removeProperty;
    NPEnumerationFunctionPtr enumerate;
    NPConstructFunctionPtr construct;
} MozNPClass;

// Defines taken from Mozilla's npruntime.h
#define NP_CLASS_STRUCT_VERSION_ENUM 2
#define NP_CLASS_STRUCT_VERSION_CTOR 3

#define NP_CLASS_STRUCT_VERSION_HAS_ENUM(npclass)   \
        ((npclass)->structVersion >= NP_CLASS_STRUCT_VERSION_ENUM)

#define NP_CLASS_STRUCT_VERSION_HAS_CTOR(npclass)   \
        ((npclass)->structVersion >= NP_CLASS_STRUCT_VERSION_CTOR)

class NPPExceptionAutoHolder
{
public:
  NPPExceptionAutoHolder();
  ~NPPExceptionAutoHolder();

protected:
  char *mOldException;
};

// Taken pretty much verbatum from nsJSNPRRuntime.h
class nsJSNPRuntime
{
public:
  static void OnPluginDestroy(NPP npp);
};

class nsJSObjWrapperKey
{
public:
  nsJSObjWrapperKey(JSObject *obj, NPP npp)
    : mJSObj(obj), mNpp(npp)
  {
  }

  JSObject *mJSObj;

  const NPP mNpp;
};

class nsJSObjWrapper : public NPObject,
                       public nsJSObjWrapperKey
{
public:
  static NPObject *GetNewOrUsed(NPP npp, JSContext *cx, JSObject *obj);

protected:
  nsJSObjWrapper(NPP npp);
  ~nsJSObjWrapper();

  static NPObject * NP_Allocate(NPP npp, NPClass *aClass);
  static void NP_Deallocate(NPObject *obj);
  static void NP_Invalidate(NPObject *obj);
  static bool NP_HasMethod(NPObject *, NPIdentifier identifier);
  static bool NP_Invoke(NPObject *obj, NPIdentifier method,
                        const NPVariant *args, uint32_t argCount,
                        NPVariant *result);
  static bool NP_InvokeDefault(NPObject *obj, const NPVariant *args,
                               uint32_t argCount, NPVariant *result);
  static bool NP_HasProperty(NPObject * obj, NPIdentifier property);
  static bool NP_GetProperty(NPObject *obj, NPIdentifier property,
                             NPVariant *result);
  static bool NP_SetProperty(NPObject *obj, NPIdentifier property,
                             const NPVariant *value);
  static bool NP_RemoveProperty(NPObject *obj, NPIdentifier property);
  static bool NP_Enumerate(NPObject *npobj, NPIdentifier **identifier,
                           uint32_t *count);
  static bool NP_Construct(NPObject *obj, const NPVariant *args,
                           uint32_t argCount, NPVariant *result);

public:
  static MozNPClass sJSObjWrapperNPClass;
};

class nsNPObjWrapper
{
public:
  static void OnDestroy(NPObject *npobj);
  static JSObject *GetNewOrUsed(NPP npp, JSContext *cx, NPObject *npobj);
};

class NPPStack
{
public:
  static NPP Peek()
  {
    return sCurrentNPP;
  }

protected:
  static NPP sCurrentNPP;
};

// XXXjst: The NPPAutoPusher stack is a bit redundant now that
// PluginDestructionGuard exists, and could thus be replaced by code
// that uses the PluginDestructionGuard list of plugins on the
// stack. But they're not identical, and to minimize code changes
// we're keeping both for the moment, and making NPPAutoPusher inherit
// the PluginDestructionGuard class to avoid having to keep two
// separate objects on the stack since we always want a
// PluginDestructionGuard where we use an NPPAutoPusher.

class NPPAutoPusher : public NPPStack
//                      protected PluginDestructionGuard
{
public:
  NPPAutoPusher(NPP npp) :
//    PluginDestructionGuard(npp),
      mOldNPP(sCurrentNPP)
  {
    // NS_ASSERTION(npp, "Uh, null npp passed to NPPAutoPusher!");
    assert(npp);

    sCurrentNPP = npp;
  }

  ~NPPAutoPusher()
  {
    sCurrentNPP = mOldNPP;
  }

private:
  NPP mOldNPP;
};

// Method called directly from npapi interfaces
bool JSValToNPVariant(NPP npp, JSContext *cx, jsval val, NPVariant *variant);

NPObject* _createobject(NPP npp, MozNPClass* aClass);
NPObject* _retainobject(NPObject* npobj);
void _releaseobject(NPObject* npobj);
NPObject* _getwindowobject(NPP npp);

} // namespace SpiderMonkeyNPAPIBindings

#endif  // GEARS_WORKERPOOL_SAFARI_MOZJS_NPRUNTIME_UTILS_H__
