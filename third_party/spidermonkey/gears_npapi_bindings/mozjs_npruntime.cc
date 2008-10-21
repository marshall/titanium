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

// Spidermonkey NPAPI bindings, adapted from firefox 3.0b5 source tarball
// mozilla/modules/plugin/base/src/nsJSNPRuntime.cpp & ns4xPlugin.cpp

#include "gears/base/common/common.h"
#include "gears/base/common/string_utils.h"
#include "third_party/spidermonkey/gears_include/mozjs_api.h"
#include "third_party/spidermonkey/gears_npapi_bindings/mozjs_npapi_storage.h"
#include "third_party/spidermonkey/gears_npapi_bindings/mozjs_npruntime.h"
#include "third_party/spidermonkey/gears_npapi_bindings/mozjs_npruntime_utils.h"

namespace SpiderMonkeyNPAPIBindings {

// -------------- Internal Functions ----------

static NPIdentifier
doGetIdentifier(JSContext *cx, const NPUTF8* name)
{
  std::string16 utf16name;
  if (!UTF8ToString16(name, strlen(name), &utf16name)) {
    assert(false);
    return NULL;
  }

  JSString *str = ::JS_InternUCStringN(cx, String16ToJschar(utf16name),
                                       utf16name.size());
  if (!str) {
    return NULL;
  }

  return (NPIdentifier)STRING_TO_JSVAL(str);
}

// -------------- public NPAPI implementation ----------

NPIdentifier NPN_GetStringIdentifier(const NPUTF8 *name) {
  assert(name);
  JSContext *cx = NPAPI_Storage::GetCurrentJSContext();
  JSAutoRequest ar(cx);
  return doGetIdentifier(cx, name);
}

bool NPN_IdentifierIsString(NPIdentifier identifier) {
  jsval v = reinterpret_cast<jsval>(identifier);
  return JSVAL_IS_STRING(v);
}

NPUTF8 *NPN_UTF8FromIdentifier(NPIdentifier identifier) {
  if (!identifier)
    return NULL;

  jsval v = (jsval)identifier;

  if (!JSVAL_IS_STRING(v)) {
    return NULL;
  }

  JSString *str = JSVAL_TO_STRING(v);
  char16 *str_utf16 = ::JS_GetStringChars(str);
  size_t len = JS_GetStringLength(str);
  
  std::string str_utf8;
  if (!String16ToUTF8(str_utf16, len, &str_utf8)) {
    return NULL;
  }
  
  size_t ret_num_bytes = str_utf8.length() + 1;
  NPUTF8 *ret = (NPUTF8 *)NPN_MemAlloc(ret_num_bytes);
  if (!ret) {
    return NULL;
  }
  memset(ret, 0, ret_num_bytes);
  str_utf8.copy(ret, ret_num_bytes, 0);

  return ret;
}

int32_t NPN_IntFromIdentifier(NPIdentifier identifier) {
  jsval v = reinterpret_cast<jsval>(identifier);
  assert(JSVAL_IS_INT(v));
  return JSVAL_TO_INT(v);
}

NPIdentifier NPN_GetIntIdentifier(int32_t intid) {
  return reinterpret_cast<NPIdentifier>(INT_TO_JSVAL(intid));
}

void *NPN_MemAlloc(uint32 size)
{
    return malloc(size);
}

void NPN_MemFree(void *ptr)
{
    free(ptr);
}

void NPN_SetException(NPObject *npobj, const NPUTF8 *message) {
  assert(npobj);
  assert(message);
  
  char **exception_msg = NPAPI_Storage::GetgNPPException();
  if (*exception_msg) {
    // If a plugin throws multiple exceptions, we'll only report the
    // last one for now.
    free(*exception_msg);
  }

  *exception_msg = strdup(message);
}

void NPN_GetStringIdentifiers(const NPUTF8 **names, 
                              int32_t nameCount, 
                              NPIdentifier *identifiers) {
 // Unused in Gears codebase.
 assert(false);
}

NPObject *NPN_CreateObject(NPP npp, NPClass *aClass) {
  assert(aClass);
  assert(npp);
  if (!npp || !aClass) {
    return NULL;
  }

  NPObject *npobj;

  if (aClass->allocate) {
    npobj = aClass->allocate(npp, aClass);
  } else {
    npobj = static_cast<NPObject *>(malloc(sizeof(NPObject)));
  }

  if (npobj) {
    npobj->_class = aClass;
    npobj->referenceCount = 1;
  }

  return npobj;
}

NPObject *NPN_RetainObject(NPObject *npobj) {
  assert(npobj);
  return _retainobject(npobj);
}

void NPN_ReleaseObject(NPObject *obj) {
  return _releaseobject(obj);
}

void NPN_DeallocateObject(NPObject *obj) {
  // Unused in Gears codebase.
  assert(false);
}

bool
NPN_Invoke(NPP npp, NPObject* npobj, NPIdentifier method, const NPVariant *args,
           uint32_t argCount, NPVariant *result)
{
  if (!npp || !npobj || !npobj->_class || !npobj->_class->invoke)
    return false;

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);
  
  return npobj->_class->invoke(npobj, method, args, argCount, result);
}

bool NPN_InvokeDefault(NPP npp, NPObject* npobj, const NPVariant *args,
                       uint32_t argCount, NPVariant *result)
{
  if (!npp || !npobj || !npobj->_class || !npobj->_class->invokeDefault)
    return false;

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);

  return npobj->_class->invokeDefault(npobj, args, argCount, result);
}

bool NPN_Evaluate(NPP npp, NPObject* npobj, NPString *script, NPVariant *result)
{
  if (!npp)
    return false;

  NPPAutoPusher nppPusher(npp);

  JSContext *cx = NPAPI_Storage::GetCurrentJSContext();
  if (!cx) {
    return false;
  }
  JSAutoRequest context_lock(cx);

  JSObject *obj =
    nsNPObjWrapper::GetNewOrUsed(npp, cx, npobj);

  if (!obj) {
    return false;
  }

  // Root obj and the rval (below).
  jsval vec[] = { OBJECT_TO_JSVAL(obj), JSVAL_NULL };
  JSAutoTempValueRooter tvr(cx, ARRAYSIZE(vec), vec);
  jsval *rval = &vec[1];

  if (result) {
    // Initialize the out param to void
    VOID_TO_NPVARIANT(*result);
  }

  if (!script || !script->UTF8Length || !script->UTF8Characters) {
    // Nothing to evaluate.

    return true;
  }
  
  std::string16 script_utf16;
  if (!UTF8ToString16(script->UTF8Characters, script->UTF8Length, 
      &script_utf16)) {
    return false;
  }
  
  JSBool rv = JS_EvaluateUCScript(cx, obj, script_utf16.c_str(),
                                  script_utf16.length(), "Gears Worker", 1,  
                                  rval);
  
  return (rv == JS_TRUE) &&
         (!result || JSValToNPVariant(npp, cx, *rval, result));
}

bool
NPN_GetProperty(NPP npp, NPObject* npobj, NPIdentifier property,
                NPVariant *result)
{
  if (!npp || !npobj || !npobj->_class || !npobj->_class->getProperty)
    return false;

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);

  return npobj->_class->getProperty(npobj, property, result);
}

bool
NPN_SetProperty(NPP npp, NPObject* npobj, NPIdentifier property,
             const NPVariant *value)
{
  if (!npp || !npobj || !npobj->_class || !npobj->_class->setProperty)
    return false;

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);

  return npobj->_class->setProperty(npobj, property, value);
}

bool
NPN_RemoveProperty(NPP npp, NPObject* npobj, NPIdentifier property)
{
  if (!npp || !npobj || !npobj->_class || !npobj->_class->removeProperty)
    return false;

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);

  return npobj->_class->removeProperty(npobj, property);
}

bool
NPN_HasProperty(NPP npp, NPObject* npobj, NPIdentifier propertyName)
{
  if (!npp || !npobj || !npobj->_class || !npobj->_class->hasProperty)
    return false;

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);

  return npobj->_class->hasProperty(npobj, propertyName);
}

bool NPN_HasMethod(NPP npp, NPObject* npobj, NPIdentifier methodName)
{
  if (!npp || !npobj || !npobj->_class || !npobj->_class->hasMethod)
    return false;

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);

  return npobj->_class->hasProperty(npobj, methodName);
}

bool NPN_Enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
                   uint32_t *count)
{
  if (!npp || !npobj || !npobj->_class)
    return false;

  if (!NP_CLASS_STRUCT_VERSION_HAS_ENUM(((MozNPClass *)npobj->_class)) ||
      !((MozNPClass *)npobj->_class)->enumerate) {
    *identifier = 0;
    *count = 0;
    return true;
  }

  NPPExceptionAutoHolder nppExceptionHolder;
  NPPAutoPusher nppPusher(npp);

  return ((MozNPClass *)npobj->_class)->enumerate(npobj, identifier, count);
}

NPError NPN_GetValue(NPP npp, NPNVariable variable, void *result)
{
  switch(variable) {
    case NPNVWindowNPObject: {
      *(NPObject **)result = _getwindowobject(npp);
      return NPERR_NO_ERROR;
    }

    // Gears only requires retreival of the window object.
    default : {
      assert(false);
      return NPERR_GENERIC_ERROR;
    }
  }
}

void NPN_ReleaseVariantValue(NPVariant *variant)
{
  switch (variant->type) {
  case NPVariantType_Void :
  case NPVariantType_Null :
  case NPVariantType_Bool :
  case NPVariantType_Int32 :
  case NPVariantType_Double :
    break;
  case NPVariantType_String :
    {
      const NPString *s = &NPVARIANT_TO_STRING(*variant);

      if (s->UTF8Characters)
        free((void *)s->UTF8Characters);

      break;
    }
  case NPVariantType_Object:
    {
      NPObject *npobj = NPVARIANT_TO_OBJECT(*variant);

      if (npobj)
        _releaseobject(npobj);

      break;
    }
  default:
    assert(false);
    // NS_ERROR("Unknown NPVariant type!");
  }

  VOID_TO_NPVARIANT(*variant);
}

} // namespace SpiderMonkeyNPAPIBindings
