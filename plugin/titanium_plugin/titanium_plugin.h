/*
 *  titanium_plugin.h
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Appcelerator, Inc. All rights reserved.
 *
 */


#include "npapi.h"
#include "npruntime.h"
#include <iostream>
#include <string>
#include <fstream>
#include <Carbon/Carbon.h>
#include "TView.h"
#include "TCarbonEvent.h"
#include "SciLexer.h"
#include "ScintillaMacOSX.h"

CGrafPtr gPtr = NULL;
NP_Port *port = NULL;
std::string filePath;