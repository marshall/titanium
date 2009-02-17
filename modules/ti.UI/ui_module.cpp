/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ui_module.h"
#include <Poco/URI.h>

#ifdef OS_OSX
  #define TI_FATAL_ERROR(msg) \
  { \
	NSAlert *alert = [[NSAlert alloc] init]; \
	[alert addButtonWithTitle:@"OK"]; \
	[alert setMessageText:@"Application Error"]; \
	[alert setInformativeText:[NSString stringWithCString:msg]]; \
	[alert setAlertStyle:NSCriticalAlertStyle]; \
	[alert runModal]; \
	[alert release]; \
	[NSApp terminate:nil]; \
	 \
  }
#elif OS_WIN32
  #define TI_FATAL_ERROR(msg) \
  { \
	MessageBox(NULL,msg,"Application Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL); \
	ExitProcess(1);\
  }
#elif OS_LINUX
  #define TI_FATAL_ERROR(msg) \
  { \
	GtkWidget* dialog = gtk_message_dialog_new (NULL,  \
	                                  GTK_DIALOG_MODAL, \
					  GTK_MESSAGE_ERROR,  \
	                                  GTK_BUTTONS_OK, \
	                                  msg); \
	gtk_dialog_run (GTK_DIALOG (dialog)); \
	gtk_widget_destroy (dialog); \
	exit(1); \
  }
#endif


namespace ti
{
	KROLL_MODULE(UIModule)

	SharedBoundObject UIModule::global = SharedBoundObject(NULL);
	SharedPtr<MenuItem> UIModule::app_menu = SharedPtr<MenuItem>(NULL);
	SharedPtr<MenuItem> UIModule::app_context_menu = SharedPtr<MenuItem>(NULL);
	SharedString UIModule::icon_path = SharedString(NULL);
	std::vector<SharedPtr<TrayItem> > UIModule::tray_items;

	void UIModule::Initialize()
	{

		AppConfig *config = AppConfig::Instance();
		if (config == NULL)
		{
			TI_FATAL_ERROR("Error loading tiapp.xml. Your application is not properly configured or packaged.");
		}
		WindowConfig *main_window_config = config->GetMainWindow();
		if (main_window_config == NULL)
		{
			TI_FATAL_ERROR("Error loading tiapp.xml. Your application window is not properly configured or packaged.");
		}

		// We are keeping this object in a static variable, which means
		// that we should only ever have one copy of the UI module.
		SharedBoundObject global = this->host->GetGlobalObject();
		UIModule::global = global;

		// Add the Titanium.UI binding and initialize the main window.
#if defined(OS_LINUX)
		SharedBoundObject ui_binding = new GtkUIBinding(this->host);
		GtkUserWindow* window = new GtkUserWindow(this->host, main_window_config);

#elif defined(OS_OSX)
		SharedBoundObject ui_binding = new OSXUIBinding(this->host);
		OSXUserWindow* window = new OSXUserWindow(this->host, main_window_config);
		NativeWindow* nw = window->GetNative();
		[nw setInitialWindow:YES];

#elif defined(OS_WIN32)
		SharedBoundObject ui_binding = new Win32UIBinding(this->host);
		Win32UserWindow* window = new Win32UserWindow(this->host, main_window_config);
#endif

		SharedValue ui_binding_val = Value::NewObject(ui_binding);
		host->GetGlobalObject()->Set("UI", ui_binding_val);

		SharedBoundObject main_window = window;
		SharedValue main_window_val = Value::NewObject(main_window);
		host->GetGlobalObject()->SetNS("UI.mainWindow", main_window_val);
		window->Open();
	}

	void UIModule::Stop()
	{
		// Remove app tray icons
		UIModule::ClearTrayItems();

		// Only one copy of the UI module loaded hopefully,
		// otherwise we need to count instances and free
		// this variable when the last instance disappears
		UIModule::global = SharedBoundObject(NULL);
	}

	bool UIModule::IsResourceLocalFile(std::string string)
	{
		Poco::URI uri(string.c_str());
		std::string scheme = uri.getScheme();
		return (scheme == "app" || scheme == "ti" || scheme == "file");
	}
	
	SharedString UIModule::GetResourcePath(const char *URL)
	{
		if (URL == NULL || !strcmp(URL, ""))
			return SharedString(NULL);
		
		Poco::URI uri(URL);
		std::string scheme = uri.getScheme();

		if (scheme == "app" || scheme == "ti")
		{
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
		else
		{
			return SharedString(new std::string(URL));
		}
	}

	void UIModule::SetMenu(SharedPtr<MenuItem> menu)
	{
		UIModule::app_menu = menu;
	}

	SharedPtr<MenuItem> UIModule::GetMenu()
	{
		return UIModule::app_menu;
	}

	void UIModule::SetContextMenu(SharedPtr<MenuItem> menu)
	{
		UIModule::app_context_menu = menu;
	}

	SharedPtr<MenuItem> UIModule::GetContextMenu()
	{
		return UIModule::app_context_menu;
	}

	void UIModule::SetIcon(SharedString icon_path)
	{
		UIModule::icon_path = icon_path;
	}

	SharedString UIModule::GetIcon()
	{
		return UIModule::icon_path;
	}

	void UIModule::AddTrayItem(SharedPtr<TrayItem> item)
	{
		// One tray item at a time
		UIModule::ClearTrayItems();
		UIModule::tray_items.push_back(item);
	}

	void UIModule::ClearTrayItems()
	{
		std::vector<SharedPtr<TrayItem> >::iterator i = UIModule::tray_items.begin();
		while (i != UIModule::tray_items.end())
		{
			(*i++)->Remove();
		}
		UIModule::tray_items.clear();
	}

	void UIModule::UnregisterTrayItem(TrayItem* item)
	{
		std::vector<SharedPtr<TrayItem> >::iterator i = UIModule::tray_items.begin();
		while (i != UIModule::tray_items.end())
		{
			SharedPtr<TrayItem> c = *i;
			if (c.get() == item)
			{
				i = UIModule::tray_items.erase(i);
			}
			else
			{
				i++;
			}
		}
	}
}
