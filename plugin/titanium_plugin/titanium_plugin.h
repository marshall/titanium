/*
 *  titanium_plugin.h
 *  titanium_plugin
 *
 *  Created by Marshall on 9/9/08.
 *  Copyright 2008 Redhat. All rights reserved.
 *
 */

#ifndef titanium_plugin_
#define titanium_plugin_

/* The classes below are exported */
#pragma GCC visibility push(default)

#include "plugin.h"

class titanium_plugin
{
	public:
		void HelloWorld(const char *);
};

#pragma GCC visibility pop
#endif
