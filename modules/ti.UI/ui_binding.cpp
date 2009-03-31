/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "ui_module.h"
#include <string>

#define GET_ARG_OR_RETURN(INDEX, TYPE, VAR) \
	if ((int) args.size() < INDEX - 1 || !args.at(INDEX)->Is##TYPE()) \
		return; \
	VAR = args.at(INDEX)->To##TYPE();

#define GET_ARG(INDEX, TYPE, VAR) \
	if ((int) args.size() > INDEX && args.at(INDEX)->Is##TYPE()) \
		VAR = args.at(INDEX)->To##TYPE();

namespace ti
{
	UIBinding* UIBinding::instance = NULL;

	UIBinding::UIBinding(Host *host) : host(host)
	{
		instance = this;

		/**
		 * @tiapi(method=True,returns=object,name=UI.createMenu) creates a Menu object
		 */
		this->SetMethod("createMenu", &UIBinding::_CreateMenu);
		/**
		 * @tiapi(method=True,returns=object,name=UI.createTrayMenu) creates a tray Menu object
		 */
		this->SetMethod("createTrayMenu", &UIBinding::_CreateTrayMenu);
		/**
		 * @tiapi(method=True,returns=void,name=UI.setMenu) sets a menu for the application
		 */
		this->SetMethod("setMenu", &UIBinding::_SetMenu);
		/**
		 * @tiapi(method=True,returns=object,name=UI.getMeu) returns the application menu
		 */
		this->SetMethod("getMenu", &UIBinding::_GetMenu);
		/**
		 * @tiapi(method=True,returns=void,name=UI.setContextMenu) sets the application context menu
		 */
		this->SetMethod("setContextMenu", &UIBinding::_SetContextMenu);
		/**
		 * @tiapi(method=True,returns=object,name=UI.getContextMenu) gets the application context menu
		 */
		this->SetMethod("getContextMenu", &UIBinding::_GetContextMenu);
		/**
		 * @tiapi(method=True,returns=void,name=UI.setIcon) sets the application icon
		 */
		this->SetMethod("setIcon", &UIBinding::_SetIcon);
		/**
		 * @tiapi(method=True,returns=void,name=UI.addTray) adds a tray menu
		 */
		this->SetMethod("addTray", &UIBinding::_AddTray);
		/**
		 * @tiapi(method=True,returns=void,name=UI.clearTray) removes a tray menu
		 */
		this->SetMethod("clearTray", &UIBinding::_ClearTray);
		/**
		 * @tiapi(method=True,returns=void,name=UI.setDockIcon) set the dock icon
		 */
		this->SetMethod("setDockIcon", &UIBinding::_SetDockIcon);
		/**
		 * @tiapi(method=True,returns=void,name=UI.setDockMenu) set the dock menu
		 */
		this->SetMethod("setDockMenu", &UIBinding::_SetDockMenu);
		/**
		 * @tiapi(method=True,returns=void,name=UI.setBadge) set the application badge value
		 */
		this->SetMethod("setBadge", &UIBinding::_SetBadge);
		/**
		 * @tiapi(method=True,returns=void,name=UI.setBadgeImage) set the application badge image
		 */
		this->SetMethod("setBadgeImage", &UIBinding::_SetBadgeImage);

		/**
		 * @tiapi(method=True,returns=long,name=UI.createMenu) get the user's idle time (for the machine, not just the application)
		 */
		this->SetMethod("getIdleTime", &UIBinding::_GetIdleTime);

		this->open_window_list = new StaticBoundList();
		/**
		 * @tiapi(property=True,returns=list,name=UI.windows) gets a list of user created windows
		 */
		this->Set("windows", Value::NewList(this->open_window_list));

		SharedBoundObject global = host->GetGlobalObject();
		SharedValue ui_binding_val = Value::NewObject(this);
		global->Set("UI", ui_binding_val);

	}

	void UIBinding::CreateMainWindow(WindowConfig* config)
	{
		SharedPtr<UserWindow> no_parent = NULL;
		SharedUserWindow main_window = this->CreateWindow(config, no_parent);

		SharedBoundObject global = host->GetGlobalObject();
		/**
		 * @tiapi(property=True,returns=object,name=UI.mainWindow) get the main window 
		 */
		global->SetNS("UI.mainWindow", Value::NewObject(main_window));

		main_window->Open();
	}

	void UIBinding::ErrorDialog(std::string msg)
	{
		std::cerr << msg << std::endl;
	}

	UIBinding::~UIBinding()
	{
	}

	Host* UIBinding::GetHost()
	{
		return host;
	}

	std::vector<SharedUserWindow>& UIBinding::GetOpenWindows()
	{
		return this->open_windows;
	}

	void UIBinding::AddToOpenWindows(SharedUserWindow window)
	{
		this->open_window_list->Append(Value::NewObject(window));
		this->open_windows.push_back(window);
	}

	void UIBinding::RemoveFromOpenWindows(SharedUserWindow window)
	{
		std::vector<SharedUserWindow>::iterator w = open_windows.begin();
		while (w != open_windows.end())
		{
			if ((*w).get() == window.get())
				w = this->open_windows.erase(w);
			else
				w++;
		}
	}

	void UIBinding::_CreateMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<MenuItem> menu = this->CreateMenu(false);
		result->SetList(menu);
	}

	void UIBinding::_CreateTrayMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<MenuItem> menu = this->CreateMenu(true);
		result->SetList(menu);
	}

	void UIBinding::_SetMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<MenuItem> menu = NULL; // A NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsList())
		{
			menu = args.at(0)->ToList().cast<MenuItem>();
		}
		UIModule::SetMenu(menu);
		this->SetMenu(menu);
	}

	void UIBinding::_GetMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<MenuItem> menu = UIModule::GetMenu();
		if (menu.get() != NULL)
		{
			SharedBoundList list = menu;
			result->SetList(list);
		}
		else
		{
			result->SetUndefined();
		}
	}

	void UIBinding::_SetContextMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<MenuItem> menu = NULL; // A NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsList())
		{
			menu = args.at(0)->ToList().cast<MenuItem>();
		}
		UIModule::SetContextMenu(menu);
		this->SetContextMenu(menu);
	}

	void UIBinding::_GetContextMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<MenuItem> menu = UIModule::GetContextMenu();
		if (menu.get() != NULL)
		{
			SharedBoundList list = menu;
			result->SetList(list);
		}
		else
		{
			result->SetUndefined();
		}
	}

	void UIBinding::_SetIcon(const ValueList& args, SharedValue result)
	{
		SharedString icon_path = NULL; // a NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsString())
		{
			const char *icon_url = args.at(0)->ToString();
			icon_path = UIModule::GetResourcePath(icon_url);
		}
		UIModule::SetIcon(icon_path);
		this->SetIcon(icon_path);
	}

	void UIBinding::_AddTray(const ValueList& args, SharedValue result)
	{
		const char *icon_url;
		GET_ARG_OR_RETURN(0, String, icon_url);
		SharedString icon_path = UIModule::GetResourcePath(icon_url);
		if (icon_path.isNull())
			return;

		SharedBoundMethod cb = SharedBoundMethod(NULL);
		GET_ARG(1, Method, cb);

		SharedPtr<TrayItem> item = this->AddTray(icon_path, cb);

		UIModule::AddTrayItem(item);
		result->SetObject(item);
	}

	void UIBinding::_ClearTray(const ValueList& args, SharedValue result)
	{
		UIModule::ClearTrayItems();
	}

	void UIBinding::_SetDockIcon(const ValueList& args, SharedValue result)
	{
		SharedString icon_path = NULL; // a NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsString())
		{
			const char *icon_url = args.at(0)->ToString();
			icon_path = UIModule::GetResourcePath(icon_url);
		}
		this->SetDockIcon(icon_path);
	}

	void UIBinding::_SetDockMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<MenuItem> menu = NULL; // A NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsList())
		{
			menu = args.at(0)->ToList().cast<MenuItem>();
		}
		this->SetDockMenu(menu);
	}

	void UIBinding::_SetBadge(const ValueList& args, SharedValue result)
	{
		// badges are just labels right now
		// we might want to support custom images too
		SharedString badge_path = NULL; // a NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsString())
		{
			const char *badge_url = args.at(0)->ToString();
			if (badge_url!=NULL)
			{
				badge_path = SharedString(new std::string(badge_url));
			}
		}
		this->SetBadge(badge_path);
	}

	void UIBinding::_SetBadgeImage(const ValueList& args, SharedValue result)
	{
		SharedString image_path = NULL; // a NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsString())
		{
			const char *image_url = args.at(0)->ToString();
			if (image_url!=NULL)
			{
				image_path = UIModule::GetResourcePath(image_url);
			}
		}

		this->SetBadgeImage(image_path);
	}

	void UIBinding::_GetIdleTime(
		const ValueList& args,
		SharedValue result)
	{
		result->SetDouble(this->GetIdleTime());
	}

}

