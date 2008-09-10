/*
 *  PluginObject.h
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */

#include "npapi.h"
#include "npruntime.h"
#include "npfunctions.h"

typedef struct PluginObject {
    NPObject header;
    NPP npp;
    NPWindow* window;
	char *filename;
} PluginObject;

NPClass *getPluginClass(void);

extern NPNetscapeFuncs* browser;
