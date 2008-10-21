/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <npapi.h>
#include "plugin.h"
//#define SCIMOZ_DEBUG

// Unix needs this
#define MIME_TYPES_HANDLED  "application/x-scimoz-plugin"
#define PLUGIN_NAME         "Scintilla"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED"::"PLUGIN_NAME
#define PLUGIN_DESCRIPTION  PLUGIN_NAME " for Mozilla" 

char* NPP_GetMIMEDescription(void)
{
    return(MIME_TYPES_DESCRIPTION);
}

// get values per plugin
NPError NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
  NPError err = NPERR_NO_ERROR;
  switch (aVariable) {
    case NPPVpluginNameString:
      *((char **)aValue) = PLUGIN_NAME;
      break;
    case NPPVpluginDescriptionString:
      *((char **)aValue) = PLUGIN_DESCRIPTION;
      break;
#ifdef GTK2_XEMBED
    case NPPVpluginNeedsXEmbed:
        *((PRBool *)aValue) = PR_TRUE;
        break;
#endif
    default:
      err = NPERR_INVALID_PARAM;
      break;
  }
  return err;
}

NPError NPP_Initialize(void)
{
    return NPERR_NO_ERROR;
}

void NPP_Shutdown(void)
{
}

//////////////////////////////////////
//
// general initialization and shutdown
//
NPError NS_PluginInitialize()
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"NS_PluginInitialize\n");
#endif
  return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"NS_PluginShutdown\n");
#endif
}

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
PluginInstance * NS_NewPluginInstance(NPP instance)
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"NS_NewPluginInstance %p\n", aCreateDataStruct);
#endif 
  if(!instance)
    return NULL;
#ifdef USE_LICENSE
    if (!CheckLicense()) {
            return NULL;
    }
#endif /* USE_LICENSE */

#if USE_CARBON
    // Check if the browser supports the CoreGraphics drawing model
    NPBool supportsCoreGraphics = FALSE;
    NPError err = NPN_GetValue(instance,
                                    NPNVsupportsCoreGraphicsBool,
                                    &supportsCoreGraphics);
    if (err != NPERR_NO_ERROR || !supportsCoreGraphics) 
        return NULL;

    // Set the drawing model
    err = NPN_SetValue(instance,
                            NPPVpluginDrawingModel,
                            (void*)NPDrawingModelCoreGraphics);
    if (err != NPERR_NO_ERROR) 
        return NULL;
#endif

  PluginInstance * plugin = new PluginInstance(instance);
  return plugin;
}

void NS_DestroyPluginInstance(PluginInstance * aPlugin)
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"NS_DestroyPluginInstance %p\n", aPlugin);
#endif 
  if(aPlugin)
    delete aPlugin;
}

////////////////////////////////////////
//
// PluginInstance class implementation
//
PluginInstance::PluginInstance(NPP aInstance) :
  mInstance(aInstance),
  mInitialized(FALSE),
  mScriptablePeer(NULL)
{
  mString[0] = '\0';
  mScriptablePeer = new SciMoz(this);
  NS_ADDREF(mScriptablePeer);
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::PluginInstance %p inst %p peer %p\n", this, mInstance, mScriptablePeer);
#endif 
}

PluginInstance::~PluginInstance()
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::~PluginInstance %p inst %p peer %p\n", this, mInstance, mScriptablePeer);
#endif 
  // mScriptablePeer may be also held by the browser 
  // so releasing it here does not guarantee that it is over
  // we should take precaution in case it will be called later
  // and zero its mPlugin member
  if (mScriptablePeer) {
    mScriptablePeer->SetInstance(NULL);
#ifdef SCIDEBUG_REFS
    int rc = mScriptablePeer->getRefCount() - 1;
    if (rc > 0) {
        fprintf(stderr, "LEAK: Plugin Destroyed but SciMoz lives on!!! refcnt %d %p\n", rc, mScriptablePeer);
    }
#endif
    NS_RELEASE(mScriptablePeer);
  }
}

NPBool PluginInstance::init(NPWindow* aWindow)
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::init %p window %p\n",this,aWindow);
#endif 
  if(aWindow == NULL)
    return FALSE;

  SetWindow(aWindow);
  mInitialized = TRUE;
  return TRUE;
}

NPError
PluginInstance::SetWindow(NPWindow* window)
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::SetWindow %p\n", window);
#endif 
    // nsresult result;

    /*
    * PLUGIN DEVELOPERS:
    *	Before setting window to point to the
    *	new window, you may wish to compare the new window
    *	info to the previous window (if any) to note window
    *	size changes, etc.
    */
    return mScriptablePeer->PlatformSetWindow(window);
}

uint16 PluginInstance::HandleEvent(void* event)
{
    return mScriptablePeer->PlatformHandleEvent(event);
}

void PluginInstance::shut()
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::shut\n");
#endif 
  mInitialized = FALSE;
}

NPBool PluginInstance::isInitialized()
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::isInitialized %p %d\n", this, mInitialized);
#endif 
  return mInitialized;
}

void PluginInstance::getVersion(char* *aVersion)
{
  const char *ua = NPN_UserAgent(mInstance);
  char*& version = *aVersion;
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::getVersion %s\n",ua);
#endif   

  version = (char*)NPN_MemAlloc(strlen(ua) + 1);
  if (version)
    strcpy(version, ua);
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// here the plugin is asked by Mozilla to tell if it is scriptable
// we should return a valid interface id and a pointer to 
// SciMoz interface which we should have implemented
// and which should be defined in the corressponding *.xpt file
// in the bin/components folder
NPError	PluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
  NPError rv = NPERR_NO_ERROR;
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::GetValue\n");
#endif   

  switch (aVariable) {
    case NPPVpluginScriptableInstance: {
      // addref happens in getter, so we don't addref here
      ISciMoz * scriptablePeer = getScriptablePeer();
      if (scriptablePeer) {
        *(nsISupports **)aValue = scriptablePeer;
      } else
        rv = NPERR_OUT_OF_MEMORY_ERROR;
    }
    break;

    case NPPVpluginScriptableIID: {
      static nsIID scriptableIID = ISCIMOZ_IID;
      nsIID* ptr = (nsIID *)NPN_MemAlloc(sizeof(nsIID));
      if (ptr) {
          *ptr = scriptableIID;
          *(nsIID **)aValue = ptr;
      } else
        rv = NPERR_OUT_OF_MEMORY_ERROR;
    }
    break;

    default:
      rv = NS_PluginGetValue(aVariable, aValue);
      break;
  }

  return rv;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// this method will return the scriptable object (and create it if necessary)
SciMoz* PluginInstance::getScriptablePeer()
{
#ifdef SCIMOZ_DEBUG
    fprintf(stderr,"PluginInstance::getScriptablePeer\n");
#endif 
  if (!mScriptablePeer) {
    mScriptablePeer = new SciMoz(this);
    if(!mScriptablePeer)
      return NULL;

    NS_ADDREF(mScriptablePeer);
  }

  // add reference for the caller requesting the object
  NS_ADDREF(mScriptablePeer);
  return mScriptablePeer;
}
