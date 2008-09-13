/*
 *  titanium_plugin.cpp
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */

#include "titanium_plugin.h"
#include "EditorObject.h"

using namespace Scintilla;

void debug (std::string message) {
	std::cout << "[titanium] " << message << std::endl;
}

NPError NPP_Initialize(void)
{
	debug("NPP_Initialize");

	
	return NPERR_NO_ERROR;
}

void NPP_Shutdown(void)
{
	debug("NPP_Shutdown");
}

std::string filePath;
NPError NPP_New(NPMIMEType pluginType, NPP instance,
								 uint16 mode, int16 argc, char* argn[],
								 char* argv[], NPSavedData* saved)
{
	debug("NPP_New");
						   
	for (int i = 0; i < argc; i++) {
		if (strcmp(argn[i], "src") == 0) {
			filePath = argv[i];
		}
	}
	
	debug("editor for " + filePath + ", creating..");
	
    instance->pdata = NPN_CreateObject(instance, EditorObject::getPluginClass());
	
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	NPError rv = NPERR_NO_ERROR;
	return rv;
}

NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
	debug("NPP_Destroy");
	
	EditorObject *obj = (EditorObject *) instance->pdata;
	NPN_ReleaseObject(obj);
	
	return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance, NPWindow* window)
{	
	debug("NPP_SetWindow");
	
	EditorObject *editor = (EditorObject *) instance->pdata;

	char message[128];
	sprintf(message, "editor address=%d, setWindow", editor);
	debug(message);
	
	editor->setWindow(window);
	
	if (filePath.length() > 0) {
		editor->openFile(filePath);
		filePath = "";
	}
	
	return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type,
									   NPStream* stream, NPBool seekable,
									   uint16* stype)
{
	debug("NPP_NewStream");
	
	return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instance, NPStream* stream,
										   NPReason reason)
{
	debug("NPP_DestroyStream");
	
	return NPERR_NO_ERROR;
}

int32 NPP_WriteReady(NPP instance, NPStream* stream)
{
	debug("NPP_WriteReady");
	
	return 0x0fffffff;
}

int32 NPP_Write(NPP instance, NPStream* stream, int32 offset,
									int32 len, void* buffer)
{
	debug("NPP_Write");
	
	return len;
}

void NPP_StreamAsFile(NPP instance, NPStream* stream,
										  const char* fname)
{
	debug("NPP_StreamAsFile");
}

void NPP_Print(NPP instance, NPPrint* platformPrint)
{
	debug("NPP_Print");
}

int16 NPP_HandleEvent(NPP instance, void* event)
{
	EditorObject *editor = (EditorObject *) instance->pdata;
	return editor->handleEvent((EventRecord*)event);
}

void NPP_URLNotify(NPP instance, const char* url,
									   NPReason reason, void* notifyData)
{
	debug("NPP_URLNotify");
}

jref NPP_GetJavaClass(void)
{
	debug("NPP_GetJavaClass");
	return NULL;
}

static bool shouldRetainReturnedNPObjects(NPP instance)
{
    // This check is necessary if you want your exposed NPObject to not leak in WebKit-based browsers (including
    // Safari) released prior to Mac OS X 10.5 (Leopard).
    //
    // Earlier versions of WebKit retained the NPObject returned from NPP_GetValue(NPPVpluginScriptableNPObject).
    // However, the NPRuntime API says NPObjects should be retained by the plug-in before they are returned.  WebKit
    // versions later than 420 do not retain returned NPObjects automatically; plug-ins are required to retain them
    // before returning from NPP_GetValue(), as in other browsers.
    static const unsigned webKitVersionNumberWithRetainFix = 420;
    static const char* const webKitVersionPrefix = " AppleWebKit/";
    const char *userAgent = NPN_UserAgent(instance);
    if (userAgent) {
        // Find " AppleWebKit/" in the user agent string
        char *webKitVersionString = strstr(userAgent, webKitVersionPrefix);
        if (!webKitVersionString)
            return true; // Not WebKit
		
        // Skip past " AppleWebKit/"
        webKitVersionString += strlen(webKitVersionPrefix);
        
        // Convert the version string into an integer.  There are some trailing junk characters after the version
        // number, but atoi() is smart enough to handle those.
        int webKitVersion = atoi(webKitVersionString);
        
        // Should not retain returned NPObjects when running in versions of WebKit earlier than 420
        if (webKitVersion && webKitVersion < webKitVersionNumberWithRetainFix)
            return false;
    }
    
    return true;
}


NPError NPP_GetValue(NPP instance, NPPVariable variable,
						 void *value)
{
	debug("NPP_GetValue");
	
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	NPError rv = NPERR_NO_ERROR;
	
	if(instance == NULL)
		return NPERR_GENERIC_ERROR;
	
	switch (variable) {
		case NPPVpluginNameString:
			*((char **)value) = "AppceleratorTitanium";
			break;
		case NPPVpluginDescriptionString:
			*((char **)value) = "Appcelerator Titanium native support plugin";
			break;
		case NPPVpluginScriptableNPObject: {
			void **v = (void **)value;
			EditorObject *obj = (EditorObject *) instance->pdata;
			
			// Returned objects are expected to be retained in most browsers, but not all.
			// See comments in shouldRetainReturnedNPObjects().
			if (obj && shouldRetainReturnedNPObjects(instance))
				NPN_RetainObject(obj);
			
			*v = obj;
			return NPERR_NO_ERROR;
		}
			break;
		default:
			rv = NPERR_GENERIC_ERROR;
	}
	
	return rv;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable,
						 void *value)
{
	debug("NPP_SetValue");
	
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	NPError rv = NPERR_NO_ERROR;
	return rv;
}