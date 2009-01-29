/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"

namespace ti
{
	GtkUIBinding::GtkUIBinding() : UIBinding()
	{
		/* Prepare the custom curl URL handler */
		curl_register_local_handler(&Titanium_app_url_handler);
	}

	SharedPtr<MenuItem> GtkUIBinding::CreateMenu()
	{
		SharedPtr<MenuItem> menu = new GtkMenuItemImpl(true);
		return menu;
	}

	void GtkUIBinding::SetMenu(SharedPtr<MenuItem>)
	{
		// On all Windows which do not have a menu, tell them
		// the application menu changed

		// All new windows will inherit the applicaiton menu
		// if no menu is specified
	}

	void GtkUIBinding::SetIcon(SharedString icon_path)
	{
		// On all Windows which do not have an icon, tell them
		// the application icon changed

		// All new windows will inherit the applicaiton icon
		// if no menu is specified
	}

	SharedPtr<TrayItem> GtkUIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		SharedPtr<TrayItem> item = new TrayItem();
		return item;
	}

}
