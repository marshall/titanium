/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "window_module.h"
#include "window_binding.h"

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
		
		// add the Titanium.Window module
		SharedBoundObject win = new WindowBinding(this->host,global);
		SharedValue winmodule = Value::NewObject(win);
		global->Set("Window",winmodule);
		
		// version
		SharedValue version = Value::NewDouble(0.2); // FIXME: for now this is hardcoded
		global->Set("version",version);

		// platform
#if defined(OS_LINUX)
		curl_register_local_handler(&Titanium_app_url_handler);
		GtkUserWindow* window = new GtkUserWindow(this->host, main_window_config);
		SharedValue platform = Value::NewString("linux");
#elif defined(OS_OSX)
		OSXUserWindow* window = new OSXUserWindow(this->host, main_window_config);
		SharedValue platform = Value::NewString("osx");
		NativeWindow* nw = window->GetNative();
		[nw setInitialWindow:YES];
#elif defined(OS_WIN32)
		Win32UserWindow* window = new Win32UserWindow(this->host, main_window_config);
		SharedValue platform = Value::NewString("win32");
#endif
		global->Set("platform",platform);

		window->Open();
	}

	void WindowModule::Destroy()
	{
	}
}
