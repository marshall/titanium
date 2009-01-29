/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ui_module.h"

namespace ti
{
	KROLL_MODULE(UIModule)

	SharedBoundObject UIModule::global = SharedBoundObject(NULL);
	SharedPtr<MenuItem> UIModule::application_menu = SharedPtr<MenuItem>(NULL);
	SharedString UIModule::icon_path = SharedString(NULL);

	void UIModule::Initialize()
	{
		std::cout << "Initializing ti.Window..." << std::endl;

#ifdef OS_OSX
		OSXInitialize();
#endif

		AppConfig *config = AppConfig::Instance();
		if (config == NULL)
		{
			std::cerr << "Error loading tiapp.xml. Your application "
			          << " is not properly configured or packaged."
			          << std::endl;
			return;
		}
		WindowConfig *main_window_config = config->GetMainWindow();
		if (main_window_config == NULL)
		{
			std::cerr << "Error loading tiapp.xml. Your application "
			          << "window is not properly configured or packaged."
			          << std::endl;
			return;
		}

		// We are keeping this object in a static variable, which means 
		// that we should only ever have one copy of the UI module.
		SharedBoundObject global = this->host->GetGlobalObject();
		UIModule::global = global;

		// Add the Titanium.UI binding and initialize the main window.
#if defined(OS_LINUX)
		SharedBoundObject ui_binding = new GtkUIBinding();
		GtkUserWindow* window = new GtkUserWindow(this->host, main_window_config);

#elif defined(OS_OSX)
		SharedBoundObject ui_binding = new OSXUIBinding();
		OSXUserWindow* window = new OSXUserWindow(this->host, main_window_config);
		NativeWindow* nw = window->GetNative();
		[nw setInitialWindow:YES];

#elif defined(OS_WIN32)
		SharedBoundObject ui_binding = new Win32UIBinding();
		Win32UserWindow* window = new Win32UserWindow(this->host, main_window_config);
#endif

		SharedValue ui_binding_val = Value::NewObject(ui_binding);
		host->GetGlobalObject()->Set("UI", ui_binding_val);

		window->Open();
	}

	void UIModule::Destroy()
	{
		// Only one copy of the UI module loaded hopefully,
		// otherwise we need to count instances and free
		// this variable when the last instance disappears
		UIModule::global = SharedBoundObject(NULL);
	}

	SharedString UIModule::GetResourcePath(const char *URL)
	{
		if (URL == NULL || !strcmp(URL, ""))
			return SharedString(NULL);

		SharedValue meth_val = UIModule::global->GetNS("App.appURLToPath");
		if (!meth_val->IsMethod())
			return SharedString(NULL);

		SharedBoundMethod meth = meth_val->ToMethod();
		ValueList args;
		args.push_back(Value::NewString(URL));
		SharedValue out_val = meth->Call(args);

		if (out_val->IsString())
		{
			return SharedString(new std::string(out_val->ToString()));
		}
		else
		{
			return SharedString(NULL);
		}
	}

	void UIModule::SetMenu(SharedPtr<MenuItem> menu)
	{
		UIModule::application_menu = menu;
	}

	SharedPtr<MenuItem> UIModule::GetMenu()
	{
		return UIModule::application_menu;
	}

	void UIModule::SetIcon(SharedString icon_path)
	{
		UIModule::icon_path = icon_path;
	}

	SharedString UIModule::GetIcon()
	{
		return UIModule::icon_path;
	}


}
