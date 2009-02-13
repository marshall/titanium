/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "ui_module.h"

#define GET_ARG_OR_RETURN(INDEX, TYPE, VAR) \
	if ((int) args.size() < INDEX - 1 || !args.at(INDEX)->Is##TYPE()) \
		return; \
	VAR = args.at(INDEX)->To##TYPE();

#define GET_ARG(INDEX, TYPE, VAR) \
	if ((int) args.size() > INDEX && args.at(INDEX)->Is##TYPE()) \
		VAR = args.at(INDEX)->To##TYPE();

namespace ti
{
	UIBinding::UIBinding(Host *host) : host(host)
	{
		this->SetMethod("createMenu", &UIBinding::_CreateMenu);
		this->SetMethod("createTrayMenu", &UIBinding::_CreateTrayMenu);
		this->SetMethod("setMenu", &UIBinding::_SetMenu);
		this->SetMethod("getMenu", &UIBinding::_GetMenu);
		this->SetMethod("setContextMenu", &UIBinding::_SetContextMenu);
		this->SetMethod("getContextMenu", &UIBinding::_GetContextMenu);
		this->SetMethod("setIcon", &UIBinding::_SetIcon);
		this->SetMethod("addTray", &UIBinding::_AddTray);

		this->SetMethod("setDockIcon", &UIBinding::_SetDockIcon);
		this->SetMethod("setDockMenu", &UIBinding::_SetDockMenu);
		this->SetMethod("setBadge", &UIBinding::_SetBadge);

		this->SetMethod("openFiles", &UIBinding::_OpenFiles);

		//TODO move this to platform
		this->SetMethod("getSystemIdleTime", &UIBinding::_GetSystemIdleTime);
	}

	UIBinding::~UIBinding()
	{
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

		SharedPtr<TrayItem> tray = this->AddTray(icon_path, cb);
		result->SetObject(tray);
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
		SharedString badge_path = NULL; // a NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsString())
		{
			const char *badge_url = args.at(0)->ToString();
			badge_path = UIModule::GetResourcePath(badge_url);
		}
		this->SetBadge(badge_path);
	}

	void UIBinding::_OpenFiles(const ValueList& args, SharedValue result)
	{
		// pass in a set of properties with each key being
		// the name of the property and a boolean for its setting
		// example:
		//
		// var selected = Titanium.Desktop.openFiles({
		//    multiple:true,
		//    files:false,
		//    directories:true,
		//    types:['js','html']
		// });
		//
		//
		SharedBoundMethod callback;
		if (args.size() < 1 || !args.at(0)->IsMethod())
		{
			throw ValueException::FromString("openFiles expects first argument to be a callback");
		}
		callback = args.at(0)->ToMethod();

		SharedBoundObject props;
		if (args.size() < 2 || !args.at(1)->IsObject())
		{
			props = new StaticBoundObject();
		}
		else
		{
			props = args.at(1)->ToObject();
		}

		bool files = props->GetBool("files", true);
		bool multiple = props->GetBool("multiple", false);
		bool directories = props->GetBool("directories", false);
		std::string path = props->GetString("path", "");
		std::string file = props->GetString("file", "");

		std::vector<std::string> types;
		if (props->Get("types")->IsList())
		{
			SharedBoundList l = props->Get("types")->ToList();
			for (int i = 0; i < l->Size(); i++)
			{
				if (l->At(i)->IsString())
				{
					types.push_back(l->At(i)->ToString());
				}
			}
		}


		this->OpenFiles(callback, multiple, files, directories, path, file, types);
	}


	void UIBinding::_GetSystemIdleTime(
		const ValueList& args,
		SharedValue result)
	{
		result->SetDouble(this->GetSystemIdleTime());
	}

}

