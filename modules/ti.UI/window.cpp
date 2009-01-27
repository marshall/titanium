/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "window.h"

namespace ti
{
	Window::Window(SharedBoundObject window) : kroll::StaticBoundObject() 
	{
		SharedValue w = Value::NewObject(window);
		this->Set("window",w);
	}
}

