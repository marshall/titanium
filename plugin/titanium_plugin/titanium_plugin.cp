/*
 *  titanium_plugin.cp
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Redhat. All rights reserved.
 *
 */

#include "npapi.h"
#include "npruntime.h"
#include <iostream>

NPError NPP_Initialize(void)
{
	return NPERR_NO_ERROR;
}

void NPP_Shutdown(void)
{
	
}

NPError NPP_New(NPMIMEType pluginType, NPP instance,
								 uint16 mode, int16 argc, char* argn[],
								 char* argv[], NPSavedData* saved)
{
	std::cout << "appcelerator:titanium:NPP_New" << std::endl;
	
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	NPError rv = NPERR_NO_ERROR;
	return rv;
}

NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
	return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance, NPWindow* window)
{	
	NP_Port *port = (NP_Port *) window->window;
	CGrafPtr gPtr = port->port;
	HIViewRef viewRef = HIViewGetRoot(gPtr);
	
	return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type,
									   NPStream* stream, NPBool seekable,
									   uint16* stype)
{
	return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instance, NPStream* stream,
										   NPReason reason)
{
	return NPERR_NO_ERROR;
}

int32 NPP_WriteReady(NPP instance, NPStream* stream)
{
	return 0x0fffffff;
}

int32 NPP_Write(NPP instance, NPStream* stream, int32 offset,
									int32 len, void* buffer)
{
	return len;
}

void NPP_StreamAsFile(NPP instance, NPStream* stream,
										  const char* fname)
{
	
}

void NPP_Print(NPP instance, NPPrint* platformPrint)
{
	
}

int16 NPP_HandleEvent(NPP instance, void* event)
{
	return 0;
}

void NPP_URLNotify(NPP instance, const char* url,
									   NPReason reason, void* notifyData)
{
	
}

jref NPP_GetJavaClass(void)
{
	return NULL;
}

NPError NPP_GetValue(NPP instance, NPPVariable variable,
						 void *value)
{
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
		/*case NPPVpluginScriptableNPObject:
			*(NPObject **)value = plugin->GetScriptableObject();
			break;*/
		default:
			rv = NPERR_GENERIC_ERROR;
	}
	
	return rv;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable,
						 void *value)
{
	if(instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	NPError rv = NPERR_NO_ERROR;
	return rv;
}


