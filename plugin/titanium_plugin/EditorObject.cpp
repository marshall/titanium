/*
 *  EditorObject.cpp
 *  titanium_plugin
 *
 *  Created by Marshall on 9/12/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */

#include "EditorObject.h"

void debug (std::string message);

static void pluginInvalidate(NPObject *obj);
static bool pluginHasProperty(NPObject *obj, NPIdentifier name);
static bool pluginHasMethod(NPObject *obj, NPIdentifier name);
static bool pluginGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant);
static bool pluginSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant);
static bool pluginInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
static bool pluginInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result);
static NPObject *pluginAllocate(NPP npp, NPClass *theClass);
static void pluginDeallocate(NPObject *obj);
static bool pluginRemoveProperty (NPObject *npobj, NPIdentifier name);
static bool pluginEnumerate (NPObject *npobj, NPIdentifier **value, uint32_t *count);

static NPClass pluginClass = { 
NP_CLASS_STRUCT_VERSION,
pluginAllocate, 
pluginDeallocate, 
pluginInvalidate,
pluginHasMethod,
pluginInvoke,
pluginInvokeDefault,
pluginHasProperty,
pluginGetProperty,
pluginSetProperty,
pluginRemoveProperty,
pluginEnumerate
};

NPClass *EditorObject::getPluginClass(void)
{
	debug("get plugin class..");
    return &pluginClass;
}


enum {
	FileNameProperty, NumberOfProperties
};


static NPIdentifier pluginPropertyIdentifiers[NumberOfProperties];
static const NPUTF8 *pluginPropertyIdentifierNames[NumberOfProperties] = {
	"fileName"
};

enum {
	OpenFileMethod, SetSelectionMethod, SetLanguageMethod, NumberOfMethods
};

static NPIdentifier pluginMethodIdentifiers[NumberOfMethods];
static const NPUTF8 *pluginMethodIdentifierNames[NumberOfMethods] = {
	"openFile", "setSelection", "setLanguage"
};

static void initializeIdentifiers(void)
{
	debug ("initializing identifiers..");
    NPN_GetStringIdentifiers(pluginPropertyIdentifierNames, NumberOfProperties, pluginPropertyIdentifiers);
    NPN_GetStringIdentifiers(pluginMethodIdentifierNames, NumberOfMethods, pluginMethodIdentifiers);
	debug ("done");
}

bool pluginHasProperty(NPObject *obj, NPIdentifier name)
{
	debug("has property?..");
    int i;
    for (i = 0; i < NumberOfProperties; i++)
        if (name == pluginPropertyIdentifiers[i])
            return true;
    return false;
}

bool pluginHasMethod(NPObject *obj, NPIdentifier name)
{
	debug("has method?..");
    int i;
    for (i = 0; i < NumberOfMethods; i++)
        if (name == pluginMethodIdentifiers[i])
            return true;
    return false;
}

bool pluginGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant)
{
	debug("get property..");
    EditorObject *editor = (EditorObject *)obj;
    if (name == pluginPropertyIdentifiers[FileNameProperty]) {
		STRINGZ_TO_NPVARIANT(editor->filename.c_str(), *variant);
        return true;
    }
    return false;
}

bool pluginSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant)
{
    return false;
}

void EditorObject::openFile (std::string filename) {
	std::ifstream filestream;
	filestream.open(filename.c_str());
	if (filestream.is_open()) {
		std::string str, contents;
		std::getline(filestream, str);
		while (filestream) {
			contents += str + "\n";
			std::getline(filestream, str);
		}
		scintilla->WndProc(SCI_SETTEXT, 0, (sptr_t)contents.c_str());
	}	
}

void EditorObject::setLanguage (std::string language) {
	scintilla->WndProc(SCI_SETLEXERLANGUAGE, 0, (sptr_t) language.c_str());	
}

bool pluginInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    EditorObject *editor = (EditorObject *)obj;
    if (name == pluginMethodIdentifiers[OpenFileMethod]) {
		
		editor->openFile(NPVARIANT_TO_STRING(args[0]).UTF8Characters);
		NULL_TO_NPVARIANT(*result);
        return true;
    }
	else if (name == pluginMethodIdentifiers[SetLanguageMethod]) {
		editor->setLanguage(NPVARIANT_TO_STRING(args[0]).UTF8Characters);

		NULL_TO_NPVARIANT(*result);
        return true;
    }
    return false;
}

bool pluginInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return false;
}

void pluginInvalidate(NPObject *obj)
{
    // Release any remaining references to JavaScript objects.
}

extern "C" HIViewRef scintilla_new(void);


const char keywords[]="and and_eq asm auto bitand bitor bool break "
"case catch char class compl const const_cast continue "
"default delete do double dynamic_cast else enum explicit export extern false float for "
"friend goto if inline int long mutable namespace new not not_eq "
"operator or or_eq private protected public "
"register reinterpret_cast return short signed sizeof static static_cast struct switch "
"template this throw true try typedef typeid typename union unsigned using "
"virtual void volatile wchar_t while xor xor_eq";

void EditorObject::setWindow (NPWindow *window)
{
	if (this->window == NULL) {
		this->window = window;
		
		NP_Port *port = (NP_Port *)window->window;
		WindowPtr windowRef = GetWindowFromPort(port->port);
		HIViewRef root = HIViewGetRoot(windowRef);
		sciView = scintilla_new();
		
		GetControlProperty(sciView, scintillaMacOSType, 0, sizeof(this->scintilla), NULL, &(this->scintilla));
		
		//scintilla->WndProc(SCI_SETLEXER, SCLEX_CPP, 0);
		scintilla->WndProc(SCI_SETSTYLEBITS, 5, 0);
		
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
		
		scintilla->WndProc(SCI_SETMARGINTYPEN, 0, (long int)SC_MARGIN_NUMBER);
		scintilla->WndProc(SCI_SETMARGINWIDTHN, 0, (long int)30);
		scintilla->WndProc(SCI_SETMARGINTYPEN, 1, (long int)SC_MARGIN_SYMBOL);
		scintilla->WndProc(SCI_SETMARGINMASKN, 1, (long int)SC_MASK_FOLDERS);
		scintilla->WndProc(SCI_SETMARGINWIDTHN, 1, (long int)20);
		//scintilla->WndProc( SCI_SETMARGINTYPEN, 2, (long int)SC_MARGIN_SYMBOL);
		//scintilla->WndProc( SCI_SETMARGINWIDTHN, 2, (long int)16);
		
		HIViewAddSubview(root, sciView);
		
		HIRect sciRect, rootRect;
		HIViewGetFrame(root, &rootRect);
		sciRect.origin.x = window->x + window->clipRect.left;
		sciRect.origin.y = window->y + window->clipRect.top;
		sciRect.size.width = window->clipRect.right - window->clipRect.left;
		sciRect.size.height = window->clipRect.bottom - window->clipRect.top;
		HIViewSetFrame(sciView, &sciRect);
		HIViewSetVisible(sciView, TRUE);
		
		ShowControl(sciView);
		SetAutomaticControlDragTrackingEnabledForWindow(windowRef, true);
		
		if (filename.length() > 0) {
			openFile(filename);
		}
	} else {
		redraw();
	}
}

void EditorObject::redraw ()
{
	HIViewSetNeedsDisplay(sciView, TRUE);
}

int16 EditorObject::handleEvent (EventRecord *event)
{
	char *eventType;
	
	int16 handled = 0;
	
	switch (event->what) {
		case updateEvt: eventType = "update"; break;
		case nullEvent: eventType = "null"; break;
		case mouseDown: eventType = "mouse down"; break;
		case mouseUp: eventType = "mouse up"; break;
		case keyDown: eventType = "key down"; break;
		case keyUp: eventType = "key up"; break;
		case autoKey: eventType = "auto key"; break;
		case diskEvt: eventType = "disk"; break;
		case activateEvt: eventType = "activate"; break;
		case osEvt: eventType = "operating system"; break;
		case kHighLevelEvent: eventType = "high level"; break;
	}
	
	if (event->what == updateEvt) {
		redraw();
		handled = 1;
	} else if (event->what == mouseDown) {
		scintilla->MouseDown(event);
		handled = 1;
	} else if (event->what == mouseUp) {
		scintilla->MouseUp(event);
		handled = 1;
	} else if (event->what == keyUp || event->what == autoKey) {
		
		scintilla->NotifyKey(event->message & keyCodeMask, event->modifiers);
		handled = 1;
	}
	
	/*if (handled==1) {
		char message[512];
		sprintf(message, "handled %s event message: %ld", eventType, eventRecord->mess
				age);
		debugConsole(message);
	}*/
	
	return handled;
}

bool identifiersInitialized = false;

NPObject *pluginAllocate(NPP npp, NPClass *theClass)
{
	debug("in plugin allocate.. constructing..");
    EditorObject *newInstance = new EditorObject();
    
    if (!identifiersInitialized) {
        identifiersInitialized = true;
        initializeIdentifiers();
    }
	
    newInstance->npp = npp;
	
    return newInstance;
}

void pluginDeallocate(NPObject *obj) 
{
    free(obj);
}

bool pluginRemoveProperty (NPObject *npobj, NPIdentifier name) {
	return false;
}

bool pluginEnumerate (NPObject *npobj, NPIdentifier **value, uint32_t *count) {
	debug("enumerating..");
	
	*count = NumberOfProperties + NumberOfMethods;
	
	value = (NPIdentifier**) malloc(sizeof(NPIdentifier*) * (*count));
	int i = 0, index = 0;
	
	for (i = 0; i < NumberOfProperties; i++, index++) value[index] = &pluginPropertyIdentifiers[i];
	for (i = 0; i < NumberOfMethods; i++, index++) value[index] = &pluginMethodIdentifiers[i];
	
	return true;
}
