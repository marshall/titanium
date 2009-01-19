/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */


#ifdef OS_WIN32
// A little disorganization; header include order is very sensitive in win32,
// and the build breaks if this below the other OS_ defines
#include "win32/win32_user_window.h"

#endif

#include "window_module.h"
#include <iostream>

#ifdef OS_LINUX
#include "linux/window_module_linux.h"
#include "url/app_url.h"
#endif

#ifdef OS_OSX
#include "osx/preinclude.h"
#include <WebKit/WebKit.h>
#include "osx/window_module_osx.h"
#include "osx/ti_app.h"
#endif

namespace ti
{
	KROLL_MODULE(WindowModule)

	void WindowModule::Initialize()
	{
		std::cout << "Initializing ti.Window..." << std::endl;

		#ifdef OS_OSX
		OSXInitialize();
		#endif

		AppConfig *config = AppConfig::Instance();
		if (config == NULL)
		{
			std::cerr << "Error loading tiapp.xml. Your application is not properly configured or packaged." << std::endl;
			return;
		}
		WindowConfig *main_window_config = config->GetMainWindow();
		if (main_window_config == NULL)
		{
			std::cerr << "Error loading tiapp.xml. Your application window is not properly configured or packaged." << std::endl;
			return;
		}

		// add some titanium specific global info here
		SharedBoundObject global = this->host->GetGlobalObject();

		// version
		SharedValue version = new Value(0.2); // FIXME: for now this is hardcoded
		global->Set("version",version);

		// platform
#if defined(OS_LINUX)
		curl_register_local_handler(&Titanium_app_url_handler);
		GtkUserWindow* window = new GtkUserWindow(this->host, main_window_config);
		SharedValue platform = new Value("linux");
#elif defined(OS_OSX)
		OSXUserWindow* window = new OSXUserWindow(this->host, main_window_config);
		SharedValue platform = new Value("osx");
#elif defined(OS_WIN32)
		Win32UserWindow* window = new Win32UserWindow(this->host, main_window_config);
		SharedValue platform = new Value("win32");
#endif
		global->Set("platform",platform);
		//KR_DECREF(global);

		window->Open();
	}

	void WindowModule::Destroy()
	{
		//KR_DECREF(this->runtime);
	}
}
