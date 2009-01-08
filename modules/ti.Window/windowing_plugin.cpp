/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */


#ifdef OS_WIN32
// A little disorganization; header include order is very sensitive in win32,
// and the build breaks if this below the other OS_ defines
#include "win32/ti_win32_user_window.h"

#endif

#include "windowing_plugin.h"
#include <iostream>

#ifdef OS_LINUX
#include "linux/windowing_plugin_linux.h"
#endif

#ifdef OS_OSX
#include "osx/preinclude.h"
#include <WebKit/WebKit.h>
#include "osx/windowing_plugin_osx.h"
#include "osx/ti_app.h"
#endif

TIPLUGIN(WindowingPlugin)

void WindowingPlugin::Initialize()
{
	std::cout << "Initializing ti.Window..." << std::endl;

#ifdef OS_OSX
	OSXInitialize();
#endif

	this->runtime = new TiStaticBoundObject();
	this->host->GetGlobalObject()->SetObject("tiRuntime", this->runtime);

	TiAppConfig *config = TiAppConfig::Instance();
	TiWindowConfig *main_window_config = config->GetMainWindow();

	std::cout << "Have config and main_window_config " << std::endl;

#if defined(OS_LINUX)
	TiGtkUserWindow* window = new TiGtkUserWindow(this->host, main_window_config);
#elif defined(OS_OSX)
	TiOSXUserWindow* window = new TiOSXUserWindow(this->host, main_window_config);
#elif defined(OS_WIN32)
	TiWin32UserWindow* window = new TiWin32UserWindow(this->host, main_window_config);
#endif

	window->Open();
}

void WindowingPlugin::Destroy()
{
	TI_DECREF(this->runtime);
}
