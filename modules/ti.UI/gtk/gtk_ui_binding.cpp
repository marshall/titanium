/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
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

		/* Register the script evaluator */
		evaluator = new ScriptEvaluator();
		addScriptEvaluator(evaluator);
	}

	SharedPtr<MenuItem> GtkUIBinding::CreateMenu()
	{
		SharedPtr<MenuItem> menu = new GtkMenuItemImpl();
		return menu;
	}

	void GtkUIBinding::SetMenu(SharedPtr<MenuItem> new_menu)
	{
		// Notify all windows that the app menu has changed.
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			GtkUserWindow* guw = dynamic_cast<GtkUserWindow*>(*i);
			if (guw != NULL)
				guw->AppMenuChanged();

			i++;
		}
	}

	void GtkUIBinding::SetContextMenu(SharedPtr<MenuItem> new_menu)
	{
	}

	void GtkUIBinding::SetIcon(SharedString icon_path)
	{

		// Notify all windows that the app icon has changed.
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			GtkUserWindow* guw = dynamic_cast<GtkUserWindow*>(*i);
			if (guw != NULL)
				guw->AppIconChanged();

			i++;
		}
	}

	SharedPtr<TrayItem> GtkUIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		SharedPtr<TrayItem> item = new GtkTrayItem(icon_path, cb);
		return item;
	}

}
