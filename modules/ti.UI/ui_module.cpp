/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ui_module.h"

namespace ti
{
	KROLL_MODULE(UIModule)

	void UIModule::Initialize()
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
		SharedBoundObject win_binding = new WindowBinding(this->host, global);
		SharedValue win_binding_val = Value::NewObject(win_binding);
		global->Set("Window", win_binding_val);

		// add the Titanium.Menu module
		SharedBoundObject menu_binding = new MenuBinding(host->GetGlobalObject());
		SharedValue menu_binding_val = Value::NewObject(menu_binding);
		host->GetGlobalObject()->Set("Menu", menu_binding_val);

		// version
		SharedValue version = Value::NewDouble(0.2); // FIXME: for now this is hardcoded
		global->Set("version", version);

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

	void UIModule::Destroy()
	{
	}
}
