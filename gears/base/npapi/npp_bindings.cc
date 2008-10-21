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

#include "gears/base/safari/messagebox.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/base_class.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/npapi/browser_utils.h"
#include "gears/base/common/js_runner_utils.h"

#include <fstream>

std::string16 g_user_agent;  // Access externally via BrowserUtils class.

#ifdef OS_ANDROID
const ThreadLocals::Slot kNPPInstance = ThreadLocals::Alloc();
#endif


//MAC: a simple javascript callback that calls the function "debugConsole" (defined in test.html for now)
ModuleWrapper *wrapper = NULL;
void debugConsole (const char *message) {
	JsRunnerInterface *jsRunner = wrapper->GetModuleImplBaseClass()->GetJsRunner();
	
	std::string16 script =
		std::string16(STRING16(L"debugConsole('")) +
		EscapeMessage(UTF8ToString16(message).c_str()) +
		std::string16(STRING16(L"');"));
	
	jsRunner->Eval(script);
}


std::string filepath;

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
  
//  MessageBox(std::string16(STRING16(L"foo")).c_str(),std::string16(STRING16(L"foo")).c_str());

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
//JGH:  return NPN_SetValue(instance, NPPVpluginWindowBool, NULL);
  bool returnValue = false;
  
  
	for (int i = 0; i < argc; i++) {
		if (strcmp(argn[i], "width") == 0 && strcmp(argv[i],"0") != 0) {
			returnValue = true;
			//break;
		} else if (strcmp(argn[i], "src") == 0) {
			filepath = argv[i];
		}
	}
	if (returnValue)
	{
		instance->pdata = NULL;
	}
	else
	{
	  NPObject* obj = CreateGearsFactoryWrapper(instance);
		wrapper = static_cast<ModuleWrapper*>(obj);
	  instance->pdata = obj;
	}
   

  return NPN_SetValue(instance, NPPVpluginWindowBool, &returnValue);	
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

NP_Port* globalPort;
CGrafPtr globalCGrafPtr;

#include "ScintillaMacOSX.h"
#include "SciLexer.h"
#include "TCarbonEvent.h"
#include <Carbon/Carbon.h>

using namespace Scintilla;
const char keywords[]="and and_eq asm auto bitand bitor bool break "
"case catch char class compl const const_cast continue "
"default delete do double dynamic_cast else enum explicit export extern false float for "
"friend goto if inline int long mutable namespace new not not_eq "
"operator or or_eq private protected public "
"register reinterpret_cast return short signed sizeof static static_cast struct switch "
"template this throw true try typedef typeid typename union unsigned using "
"virtual void volatile wchar_t while xor xor_eq";

ScintillaMacOSX *scintilla = NULL;

void init_scintilla (HIViewRef sciView, int x, int y)
{
	GetControlProperty(sciView, scintillaMacOSType, 0, sizeof(scintilla), NULL, &scintilla);
	
	scintilla->WndProc( SCI_SETLEXER, SCLEX_XML, 0);
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
	//scintilla->WndProc( SCI_SETMARGINTYPEN, 2, (long int)SC_MARGIN_SYMBOL);
	//scintilla->WndProc( SCI_SETMARGINWIDTHN, 2, (long int)16);
	
	std::ifstream filestream;
	filestream.open(filepath.c_str());
	if (filestream.is_open()) {
		std::string str, contents;
		std::getline(filestream, str);
		while (filestream) {
			contents += str + "\n";
			std::getline(filestream, str);
		}
		scintilla->WndProc( SCI_SETTEXT, 0, (sptr_t)contents.c_str());
	}

//std::string contents = "hello,world\ni am some multi-lined text\n\nyo!";

	
	//HIRect boundsRect;
	//boundsRect.origin.x = 5;
	//boundsRect.origin.y = 5;
	//boundsRect.size.width = 300;
	//boundsRect.size.height = 300;
	
	//HIViewPlaceInSuperviewAt (myCombo, 25.0, 25.0);
	//HIViewSetFrame(sciView, &boundsRect);
	
	//scintilla->WndProc( SCI_SETTEXT, 0, (sptr_t) contents.c_str());
}

extern "C" HIViewRef scintilla_new(void);

const HILayoutInfo kBindToParentLayout = {
  kHILayoutInfoVersionZero,
  { { NULL, kHILayoutBindTop }, { NULL, kHILayoutBindLeft }, { NULL, kHILayoutBindBottom }, { NULL, kHILayoutBindRight } },
  { { NULL, kHILayoutScaleAbsolute, 0 }, { NULL, kHILayoutScaleAbsolute, 0 } },
  { { NULL, kHILayoutPositionTop, 0 }, { NULL, kHILayoutPositionLeft, 0 } }
};


//MAC: these two functions were copied from https://wiki.mozilla.org/Mac:NPAPI_Drawing_Models
// basically sets up a context ref that can be drawn into from a widget etc
static CGContextRef beginQDPluginUpdate(NPWindow *window)
{
    // window->window is an NPPort* since the browser is using QuickDraw
    NP_Port *npPort = ((NP_Port *)window->window);
	
    // Get the CGContext for the port
    CGContextRef cgContext;
    QDBeginCGContext(npPort->port, &cgContext);
    CGContextSaveGState(cgContext);
	
    // Set the CG clip path to the port's clip region -- QDBeginCGContext()
    // does not automatically respect the QuickDraw port's clip region.
    RgnHandle clipRegion = NewRgn();
    GetPortClipRegion(npPort->port, clipRegion);
    Rect portBounds;
    GetPortBounds(npPort->port, &portBounds);
    ClipCGContextToRegion(cgContext, &portBounds, clipRegion);
    DisposeRgn(clipRegion);
	
    // Flip the CG context vertically -- its origin is at the lower left,
    // but QuickDraw's origin is at the upper left.
    CGContextTranslateCTM(cgContext, 0.0, portBounds.bottom - portBounds.top);
    CGContextScaleCTM(cgContext, 1.0, -1.0);
	
    return cgContext;
}

static void endQDPluginUpdate(NPWindow *window, CGContextRef cgContext)
{
    // Restore state (it was saved in beginQDPluginUpdate())
    CGContextRestoreGState(cgContext);
	
    // If we had to prepare the CGContext for use in a QuickDraw-only browser,
    // restore its state and notify QD that the CG drawing sequence is over.
    CGContextFlush(cgContext);
    QDEndCGContext(((NP_Port *)window->window)->port, &cgContext);
}


bool initialized = FALSE;
HIViewRef sciView;
NPWindow *window;

static void drawEditor ()
{
	/*CGContextRef context = beginQDPluginUpdate(pNPWindow);
	
	HIRect frame;
	CGImageRef image;
	HIViewCreateOffscreenImage(sciView, 0, &frame, &image);
	HIViewDrawCGImage(context, &frame, image);
	
	endQDPluginUpdate(pNPWindow, context);*/
	HIViewSetNeedsDisplay(sciView, TRUE);
}

pascal OSStatus WindowEventHandler(EventHandlerCallRef  inCallRef,
								   EventRef   inEvent,
								   void*          inUserData )
{
	HIViewRef sciView = *reinterpret_cast<HIViewRef*>( inUserData );
	WindowRef window = GetControlOwner(sciView);
	ScintillaMacOSX* scintilla;
	GetControlProperty( sciView, scintillaMacOSType, 0, sizeof( scintilla ), NULL, &scintilla );
	TCarbonEvent event( inEvent );
	
	// If the window is not active, let the standard window handler execute.
	if ( ! IsWindowActive( window ) ) return eventNotHandledErr;
	
	const HIViewRef rootView = HIViewGetRoot( window );
	assert( rootView != NULL );
	
	if ( event.GetKind() == kEventMouseDown )
    {
		UInt32 inKeyModifiers;
		event.GetParameter( kEventParamKeyModifiers, &inKeyModifiers );
		
		EventMouseButton inMouseButton;
		event.GetParameter<EventMouseButton>( kEventParamMouseButton, typeMouseButton, &inMouseButton );
		if (inMouseButton == kEventMouseButtonTertiary) {
			if (inKeyModifiers & optionKey) {
				const char *test = "\001This is a test calltip This is a test calltip This is a test calltip";
				scintilla->WndProc( SCI_CALLTIPSHOW, 0, (long int)test );
			} else {
				char *list = "test_1?0 test_2 test_3 test_4 test_5 test_6 test_7 test_8 test_9 test_10 test_11 test_12";
				scintilla->WndProc( SCI_AUTOCSHOW, 0, (long int)list );
			}
			return noErr;
		}
    }
	
	return eventNotHandledErr;
}

NPError NPP_SetWindow(NPP instance, NPWindow* pNPWindow)
{
   // NULL will be set if it's not a gears bootstrap and a actual window
   if (instance->pdata == NULL)
   {
	   window = pNPWindow;
	   
	   NP_Port *port = (NP_Port *)window->window;
	   WindowPtr windowRef = GetWindowFromPort(port->port);
	   HIViewRef root = HIViewGetRoot(windowRef);
	   
	   
	   if (!initialized) {
		  // NOTE: The browser calls NPP_SetWindow after creating the instance to allow drawing to begin. 
		  // Subsequent calls to NPP_SetWindow indicate changes in size or position; 
		  
		   sciView = scintilla_new();
		   init_scintilla(sciView, 0, 0);
		   HIViewAddSubview(root, sciView);
		   
		   
		   HIRect sciRect, rootRect;
		   HIViewGetFrame(root, &rootRect);
		   sciRect.origin.x = window->x + window->clipRect.left;
		   sciRect.origin.y = window->y + window->clipRect.top;
		   sciRect.size.width = window->clipRect.right - window->clipRect.left;
		   sciRect.size.height = window->clipRect.bottom - window->clipRect.top;
		   HIViewSetFrame(sciView, &sciRect);
		   
		   char message[512];
		   sprintf(message, "x=%f,y=%f,width=%f,height=%f", sciRect.origin.x, sciRect.origin.y, sciRect.size.width, sciRect.size.height);
		   debugConsole(message);
		   
		   //HIViewPlaceInSuperviewAt(sciView, window->clipRect.top, window->clipRect.left)
		   HIViewSetVisible(sciView, TRUE);
		   
		   static const EventTypeSpec kWindowMouseEvents[] =
		   {
			   { kEventClassMouse, kEventMouseDown },
		   };
		   
		   InstallEventHandler( GetWindowEventTarget( windowRef ), WindowEventHandler,
							   GetEventTypeCount( kWindowMouseEvents ), kWindowMouseEvents, &sciView, NULL );
		   
		   ShowControl(sciView);
		   SetAutomaticControlDragTrackingEnabledForWindow(windowRef, true);
		   debugConsole("after set window");
		   initialized = TRUE;
	   }

	   drawEditor();
   }

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
	EventRecord *eventRecord = (EventRecord *)event;
	char *eventType;
	
	int16 handled = 0;
	
	switch (eventRecord->what) {
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
	
	if (eventRecord->what == updateEvt) {
		drawEditor();
		handled = 1;
	} else if (eventRecord->what == mouseDown) {
		scintilla->MouseDown(eventRecord);
		handled = 1;
	} else if (eventRecord->what == mouseUp) {
		scintilla->MouseUp(eventRecord);
		handled = 1;
	} else if (eventRecord->what == keyUp || eventRecord->what == autoKey) {
		
		scintilla->NotifyKey(eventRecord->message & keyCodeMask, eventRecord->modifiers);
		handled = 1;
	}
	
	if (handled==1) {
		char message[512];
		sprintf(message, "handled %s event message: %ld", eventType, eventRecord->message);
		debugConsole(message);
	}
	
    return 1;
}

NPObject *NPP_GetScriptableInstance(NPP instance)
{
  if (!instance)
    return 0;

  NPObject* plugin = static_cast<NPObject*>(instance->pdata);
  NPN_RetainObject(plugin);  // caller expects it retained.
  return plugin;
}
