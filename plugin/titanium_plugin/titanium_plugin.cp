/*
 *  titanium_plugin.cp
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */

#include "npapi.h"
#include "npruntime.h"
#include <iostream>
#include <Carbon/Carbon.h>
#include "TView.h"
#include "TCarbonEvent.h"
#include "SciLexer.h"
#include "ScintillaMacOSX.h"

using namespace Scintilla;

const char keywords[]="and and_eq asm auto bitand bitor bool break "
"case catch char class compl const const_cast continue "
"default delete do double dynamic_cast else enum explicit export extern false float for "
"friend goto if inline int long mutable namespace new not not_eq "
"operator or or_eq private protected public "
"register reinterpret_cast return short signed sizeof static static_cast struct switch "
"template this throw true try typedef typeid typename union unsigned using "
"virtual void volatile wchar_t while xor xor_eq";

NPError NPP_Initialize(void)
{
	return NPERR_NO_ERROR;
}

void NPP_Shutdown(void)
{
	
}

CGrafPtr gPtr = NULL;
NP_Port *port = NULL;

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
	port = (NP_Port *) window->window;
	gPtr = port->port;
	
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

void init_scintilla (HIViewRef sciView, int x, int y)
{
	ScintillaMacOSX *scintilla;
	GetControlProperty(sciView, scintillaMacOSType, 0, sizeof(scintilla), NULL, &scintilla);
	
	scintilla->WndProc( SCI_SETLEXER, SCLEX_CPP, 0);
	scintilla->WndProc( SCI_SETSTYLEBITS, 5, 0);
	
	scintilla->WndProc(SCI_STYLESETFORE, 0, 0x808080);  // White space
	scintilla->WndProc(SCI_STYLESETFORE, 1, 0x007F00);  // Comment
	scintilla->WndProc(SCI_STYLESETITALIC, 1, 1); // Comment
	scintilla->WndProc(SCI_STYLESETFORE, 2, 0x007F00);  // Line comment
	scintilla->WndProc(SCI_STYLESETITALIC, 2, 1); // Line comment
	scintilla->WndProc(SCI_STYLESETFORE, 3, 0x3F703F);  // Doc comment
	scintilla->WndProc(SCI_STYLESETITALIC, 3, 1); // Doc comment
	scintilla->WndProc(SCI_STYLESETFORE, 4, 0x7F7F00);  // Number
	scintilla->WndProc(SCI_STYLESETFORE, 5, 0x7F0000);  // Keyword
	scintilla->WndProc(SCI_STYLESETBOLD, 5, 1); // Keyword
	scintilla->WndProc(SCI_STYLESETFORE, 6, 0x7F007F);  // String
	scintilla->WndProc(SCI_STYLESETFORE, 7, 0x7F007F);  // Character
	scintilla->WndProc(SCI_STYLESETFORE, 8, 0x804080);  // UUID
	scintilla->WndProc(SCI_STYLESETFORE, 9, 0x007F7F);  // Preprocessor
	scintilla->WndProc(SCI_STYLESETFORE,10, 0x000000);  // Operators
	scintilla->WndProc(SCI_STYLESETBOLD,10, 1); // Operators
	scintilla->WndProc(SCI_STYLESETFORE,11, 0x000000);  // Identifiers
	
	scintilla->WndProc(SCI_SETKEYWORDS, 0, (sptr_t)(char *)keywords); // Keyword
	
	scintilla->WndProc( SCI_SETMARGINTYPEN, 0, (long int)SC_MARGIN_NUMBER);
	scintilla->WndProc( SCI_SETMARGINWIDTHN, 0, (long int)30);
	scintilla->WndProc( SCI_SETMARGINTYPEN, 1, (long int)SC_MARGIN_SYMBOL);
	scintilla->WndProc( SCI_SETMARGINMASKN, 1, (long int)SC_MASK_FOLDERS);
	scintilla->WndProc( SCI_SETMARGINWIDTHN, 1, (long int)20);
	scintilla->WndProc( SCI_SETMARGINTYPEN, 2, (long int)SC_MARGIN_SYMBOL);
	scintilla->WndProc( SCI_SETMARGINWIDTHN, 2, (long int)16);
	
	HIRect boundsRect;
	boundsRect.origin.x = x;
	boundsRect.origin.y = y;
	boundsRect.size.width = 300;
	boundsRect.size.height = 300;
	
	HIViewSetFrame(sciView, &boundsRect);
}

extern "C" HIViewRef scintilla_new(void);

void NPP_Print(NPP instance, NPPrint* platformPrint)
{
	
	CGContextRef cgContext;
	HIViewRef sciView = scintilla_new();
	init_scintilla(sciView, port->portx, port->porty);
	
	QDBeginCGContext(gPtr, &cgContext);
	HIRect frame;
	CGImageRef image;
	
	HIViewCreateOffscreenImage(sciView, 0, &frame, &image);
	HIViewDrawCGImage(cgContext, &frame, image);
	
	QDEndCGContext(gPtr, &cgContext);
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


