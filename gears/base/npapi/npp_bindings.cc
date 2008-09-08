/* ***** BEGIN LICENSE BLOCK *****
* Version: NPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Netscape Public License
* Version 1.1 (the "License"); you may not use this file except in
* compliance with the License. You may obtain a copy of the License at
* http://www.mozilla.org/NPL/
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
* use your version of this file under the terms of the NPL, indicate your
* decision by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL or the LGPL. If you do not delete
* the provisions above, a recipient may use your version of this file under
* the terms of any one of the NPL, the GPL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */

////////////////////////////////////////////////////////////
//
// Implementation of plugin entry points (NPP_*), which are the functions the
// browser calls to talk to the plugin. Most are just empty stubs for this
// particular plugin.
//
#include "gears/base/common/string_utils.h"
#include "gears/base/npapi/module.h"
#include "gears/base/npapi/plugin.h"
#include "gears/factory/factory_np.h"
#include "genfiles/product_constants.h"

std::string16 g_user_agent;  // Access externally via BrowserUtils class.

#ifdef OS_ANDROID
const ThreadLocals::Slot kNPPInstance = ThreadLocals::Alloc();
#endif

// here the plugin creates an instance of our NPObject object which 
// will be associated with this newly created plugin instance and 
// will do all the neccessary job
NPError NPP_New(NPMIMEType pluginType,
                NPP instance,
                uint16 mode,
                int16 argc,
                char* argn[],
                char* argv[],
                NPSavedData* saved)
{   
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

#ifdef OS_ANDROID
  ThreadLocals::SetValue(kNPPInstance, instance, NULL);
#endif
  NPObject* obj = CreateGearsFactoryWrapper(instance);
  instance->pdata = obj;

  // Keep a copy of the user agent string.
  // Note that this will initialize exactly once on the main thread.
  // this means that this correctly stores the browsers user-agent, even when 
  // workers use a seperate JS engine which may have NPAPI bindings that return
  // a different value for the UA.
  if (g_user_agent.empty()) {
    const char *user_agent_utf8 = NPN_UserAgent(instance);
    UTF8ToString16(user_agent_utf8, &g_user_agent);
  }

  // Make this a windowless plugin.
  return NPN_SetValue(instance, NPPVpluginWindowBool, NULL);
}

// here is the place to clean up and destroy the NPObject object
NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NotifyNPInstanceDestroyed(instance);

  NPObject* obj = static_cast<NPObject*>(instance->pdata);
  if (obj)
    NPN_ReleaseObject(obj);

#ifdef OS_ANDROID
  ThreadLocals::DestroyValue(kNPPInstance);
#endif

  return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance, NPWindow* pNPWindow)
{    
  return NPERR_NO_ERROR;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// here the plugin is asked by Mozilla to tell if it is scriptable
// we should return a valid interface id and a pointer to 
// nsScriptablePeer interface which we should have implemented
// and which should be defined in the corressponding *.xpt file
// in the bin/components folder
NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;

  NPObject* plugin = static_cast<NPObject*>(instance->pdata);
  if (plugin == NULL)
    return NPERR_GENERIC_ERROR;

  switch (variable) {
  case NPPVpluginNameString:
    *((char **)value) = PRODUCT_SHORT_NAME_ASCII;
    break;
  case NPPVpluginDescriptionString:
    *((char **)value) = PRODUCT_FRIENDLY_NAME_ASCII " Plugin";
    break;
  case NPPVpluginScriptableNPObject:
    *(NPObject **)value = plugin;
    NPN_RetainObject(plugin);  // caller expects it retained.
    break;
  default:
    rv = NPERR_GENERIC_ERROR;
  }

  return rv;
}

NPError NPP_NewStream(NPP instance,
                      NPMIMEType type,
                      NPStream* stream, 
                      NPBool seekable,
                      uint16* stype)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;
  return rv;
}

int32 NPP_WriteReady(NPP instance, NPStream *stream)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  int32 rv = 0x0fffffff;
  return rv;
}

int32 NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len,
                void *buffer)
{   
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  int32 rv = len;
  return rv;
}

NPError NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;
  return rv;
}

void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname)
{
  if (instance == NULL)
    return;
}

void NPP_Print(NPP instance, NPPrint* printInfo)
{
  if (instance == NULL)
    return;
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason,
                   void* notifyData)
{
  if (instance == NULL)
    return;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;
  return rv;
}

int16 NPP_HandleEvent(NPP instance, void* event)
{
  return 0;
}

NPObject *NPP_GetScriptableInstance(NPP instance)
{
  if (!instance)
    return 0;

  NPObject* plugin = static_cast<NPObject*>(instance->pdata);
  NPN_RetainObject(plugin);  // caller expects it retained.
  return plugin;
}
