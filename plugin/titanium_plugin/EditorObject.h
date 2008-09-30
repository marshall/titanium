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
	CGContextRef context;
	
	std::string filename;
	
	HIViewRef sciView;
	ScintillaMacOSX *scintilla;
	bool dragging;
	
	EditorObject() { this->window = NULL; dragging = false; }
	EditorObject(std::string filename) { this->window = NULL; dragging = false; this->filename = filename; }
	
	void setWindow(NPWindow *window);
	int16 handleEvent(EventRecord *event);
	void redraw();
	
	void setText(std::string text);
	char* getText();
	void openFile(std::string filename);
	void saveFile();
	void setLanguage(std::string language);
	void setLexer(int lexer);
	
	void setStyleForeground(int style, int foreground);
	void setStyleBackground(int style, int background);
	void setStyleItalic(int style, bool italic);
	void setStyleBold(int style, bool bold);
	void setCaretForeground(int foreground);
	void setStyleFont(int style, std::string font);
	
	static NPClass* getPluginClass();
};

#endif