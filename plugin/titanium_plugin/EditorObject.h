/*
 *  EditorObject.h
 *  titanium_plugin
 *
 *  Created by Marshall on 9/12/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */

#ifndef EDITOR_OBJECT_H
#define EDITOR_OBJECT_H 1

#import <WebKit/npapi.h>
#import <WebKit/npfunctions.h>
#import <WebKit/npruntime.h>
#include <iostream>
#include <string>
#include <fstream>

#include <Carbon/Carbon.h>
#include <TView.h>
#include "TCarbonEvent.h"
#include "SciLexer.h"
#include "ScintillaMacOSX.h"

using namespace Scintilla;

class EditorObject : public NPObject {
public:
    NPP npp;
    NPWindow* window;
	
	std::string filename;
	
	HIViewRef sciView;
	ScintillaMacOSX *scintilla;

	EditorObject() { this->window = NULL; }
	EditorObject(std::string filename) { this->window = NULL; this->filename = filename; }
	
	void setWindow(NPWindow *window);
	int16 handleEvent(EventRecord *event);
	void redraw();
	
	void setText(std::string text);
	void openFile(std::string filename);
	void setLanguage(std::string language);
	
	static NPClass* getPluginClass();
};

#endif