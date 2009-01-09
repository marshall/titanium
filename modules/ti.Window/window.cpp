/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "window_module.h"

using namespace ti;

Window::Window(UserWindow* window) : kroll::StaticBoundObject() {
    this->SetObject("currentWindow", window);
}

