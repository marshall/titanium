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
#define debugf(format, args...) do { char message[512]; sprintf(message, format, args); debug(message); } while(0);

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
	OpenFileMethod, SetTextMethod, SetSelectionMethod, SetLanguageMethod,
	SetStyleForegroundMethod, SetStyleBackgroundMethod, SetStyleItalicMethod, SetStyleBoldMethod,
	RedrawMethod, NumberOfMethods
};

static NPIdentifier pluginMethodIdentifiers[NumberOfMethods];
static const NPUTF8 *pluginMethodIdentifierNames[NumberOfMethods] = {
	"openFile", "setText", "setSelection", "setLanguage",
	"setStyleForeground", "setStyleBackground", "setStyleItalic", "setStyleBold",
	"redraw"
};

static void initializeIdentifiers(void)
{
    NPN_GetStringIdentifiers(pluginPropertyIdentifierNames, NumberOfProperties, pluginPropertyIdentifiers);
    NPN_GetStringIdentifiers(pluginMethodIdentifierNames, NumberOfMethods, pluginMethodIdentifiers);
}

bool pluginHasProperty(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < NumberOfProperties; i++)
        if (name == pluginPropertyIdentifiers[i])
            return true;
    return false;
}

bool pluginHasMethod(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < NumberOfMethods; i++)
        if (name == pluginMethodIdentifiers[i])
            return true;
    return false;
}

bool pluginGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant)
{
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
	debug("open file: " + filename);
	std::ifstream filestream;
	filestream.open(filename.c_str());
	if (filestream.is_open()) {
		std::string str, contents;
		std::getline(filestream, str);
		while (filestream) {
			contents += str + "\n";
			std::getline(filestream, str);
		}
		setText(contents);
	}	
}

void EditorObject::setText (std::string text) {
	scintilla->WndProc(SCI_SETTEXT, 0, (sptr_t) text.c_str());
}

void EditorObject::setLanguage (std::string language) {
	debug("set language: " + language);
	scintilla->WndProc(SCI_SETLEXERLANGUAGE, 0, (sptr_t) language.c_str());	
}

void EditorObject::setStyleForeground(int style, int foreground) {
	debugf("set style foreground: %d, %x", style, foreground);
	
	scintilla->WndProc(SCI_STYLESETFORE, style, foreground);
}

void EditorObject::setStyleBackground(int style, int background) {
	debugf("set style background: %d, %x", style, background);
	
	scintilla->WndProc(SCI_STYLESETBACK, style, background);
}

void EditorObject::setStyleItalic(int style, bool italic) {
	debugf("set style italic: %d, %d", style, italic);
	
	scintilla->WndProc(SCI_STYLESETITALIC, style, (italic?1:0));
}

void EditorObject::setStyleBold(int style, bool bold) {
	debugf("set style bold: %d, %d", style, bold);
	
	scintilla->WndProc(SCI_STYLESETBOLD, style, (bold?1:0));
}

std::string NPStringToString (NPString string) {
	const NPUTF8 *chars = string.UTF8Characters;
	std::string newString = chars;
	
	newString.erase(string.UTF8Length);
	return newString;
}

int NPStringToColor (NPString string) {
	std::string hexString = NPStringToString(string);
	
	if (hexString[0] == '#') {
		hexString = hexString.substr(1);
	}
	
	long color = strtol(hexString.c_str(), NULL, 16);
	
	// switch red and blue.. scintilla likes GBR syntax, HTML likes RGB
	int red = ((color >> 16) & 0xFF);
	int blue = (color & 0xFF) << 16;
	
	color = blue | (color & 0xFF00) | red;
	return color;
}

bool pluginInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    EditorObject *editor = (EditorObject *)obj;
    if (name == pluginMethodIdentifiers[OpenFileMethod]) {
		
		editor->openFile(NPStringToString(NPVARIANT_TO_STRING(args[0])));
		NULL_TO_NPVARIANT(*result);
        return true;
    }
	else if (name == pluginMethodIdentifiers[SetLanguageMethod]) {
		editor->setLanguage(NPStringToString(NPVARIANT_TO_STRING(args[0])));

		NULL_TO_NPVARIANT(*result);
        return true;
    }
	else if (name == pluginMethodIdentifiers[SetTextMethod]) {
		editor->setText(NPStringToString(NPVARIANT_TO_STRING(args[0])));
		
		NULL_TO_NPVARIANT(*result);
		return true;
	}
	else if (name == pluginMethodIdentifiers[SetStyleBackgroundMethod]) {
		// for some reason ints are being passed as doubles??
		editor->setStyleBackground((int)NPVARIANT_TO_DOUBLE(args[0]), NPStringToColor(NPVARIANT_TO_STRING(args[1])));
		
		NULL_TO_NPVARIANT(*result);
		return true;
	}
	else if (name == pluginMethodIdentifiers[SetStyleForegroundMethod]) {
		editor->setStyleForeground((int)NPVARIANT_TO_DOUBLE(args[0]), NPStringToColor(NPVARIANT_TO_STRING(args[1])));
		
		NULL_TO_NPVARIANT(*result);
		return true;
	}
	else if (name == pluginMethodIdentifiers[SetStyleBoldMethod]) {
		editor->setStyleBold(NPVARIANT_TO_DOUBLE(args[0]), NPVARIANT_TO_BOOLEAN(args[1]));
		
		NULL_TO_NPVARIANT(*result);
		return true;
	}
	else if (name == pluginMethodIdentifiers[SetStyleItalicMethod]) {
		editor->setStyleItalic(NPVARIANT_TO_DOUBLE(args[0]), NPVARIANT_TO_BOOLEAN(args[1]));
		
		NULL_TO_NPVARIANT(*result);
		return true;
	}
	else if (name == pluginMethodIdentifiers[RedrawMethod]) {
		editor->redraw();
		
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
	debugf("in set window, window addres=%d", this->window);
	
	NP_CGContext *context = (NP_CGContext *)window->window;
	WindowRef windowRef = context->window;
	HIViewRef viewRef = HIViewGetRoot(windowRef);
	
	if (this->window == NULL) {
		this->window = window;
		
		debug("creating scintilla view...");
		sciView = scintilla_new();
		
		GetControlProperty(sciView, scintillaMacOSType, 0, sizeof(this->scintilla), NULL, &(this->scintilla));
		
		debug("initializing scintilla view...");
		//scintilla->WndProc(SCI_SETLEXER, SCLEX_CPP, 0);
		/*scintilla->WndProc(SCI_SETSTYLEBITS, 5, 0);
		
		scintilla->WndProc(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x7C7C7C);
		scintilla->WndProc(SCI_STYLESETBACK, STYLE_LINENUMBER, 0x5E5E5E);
		
		scintilla->WndProc(SCI_STYLESETFORE, STYLE_DEFAULT, 0xF8F8F8);  // White space
		scintilla->WndProc(SCI_STYLESETBACK, STYLE_DEFAULT, 0x2B2B2B);
		
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
		scintilla->WndProc(SCI_STYLESETFORE,11, 0x000000);  // Identifiers*/
		
		scintilla->WndProc(SCI_SETKEYWORDS, 0, (sptr_t)(char *)keywords); // Keyword
		
		scintilla->WndProc(SCI_SETMARGINTYPEN, 0, (long int)SC_MARGIN_NUMBER);
		scintilla->WndProc(SCI_SETMARGINWIDTHN, 0, (long int)30);
		scintilla->WndProc(SCI_SETMARGINTYPEN, 1, (long int)SC_MARGIN_SYMBOL);
		scintilla->WndProc(SCI_SETMARGINMASKN, 1, (long int)SC_MASK_FOLDERS);
		scintilla->WndProc(SCI_SETMARGINWIDTHN, 1, (long int)0);
		
		//scintilla->WndProc(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x7c7c7c);
		//scintilla->WndProc(SCI_STYLESETBACK, STYLE_LINENUMBER, 0x5e5e5e);
		
		//scintilla->WndProc( SCI_SETMARGINTYPEN, 2, (long int)SC_MARGIN_SYMBOL);
		//scintilla->WndProc( SCI_SETMARGINWIDTHN, 2, (long int)16);
		scintilla->SetMouseCapture(true);
		
		debug("adding to browser window...");
		HIViewAddSubview(viewRef, sciView);
		
		if (filename.length() > 0) {
			openFile(filename);
			filename = "";
		}
	}
	
	HIRect sciRect;

	sciRect.origin.x = window->clipRect.left;
	sciRect.origin.y = window->clipRect.top;
	sciRect.size.width = window->width; //window->clipRect.right - window->clipRect.left;
	sciRect.size.height = window->height; //window->clipRect.bottom - window->clipRect.top;
	
	HIViewSetFrame(sciView, &sciRect);
	HIViewSetVisible(sciView, TRUE);
	
	ShowControl(sciView);
	SetAutomaticControlDragTrackingEnabledForWindow(windowRef, true);
	
	redraw();
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
