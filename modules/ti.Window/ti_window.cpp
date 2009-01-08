/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "windowing_plugin.h"

TiWindow::TiWindow(TiUserWindow* window) : TiStaticBoundObject() {
    this->SetObject("currentWindow", window);
}

