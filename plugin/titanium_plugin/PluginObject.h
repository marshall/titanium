/*
 *  PluginObject.h
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */
#ifndef PLUGIN_OBJECT_H
#define PLUGIN_OBJECT_H 1

#import <WebKit/npapi.h>
#import <WebKit/npfunctions.h>
#import <WebKit/npruntime.h>

typedef struct PluginObject {
    NPObject header;
    NPP npp;
    NPWindow* window;
	char *filename;
} PluginObject;

NPClass *getPluginClass(void);

extern NPNetscapeFuncs* browser;

#endif
