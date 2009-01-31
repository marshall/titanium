/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"
#include "win32_menu_item_impl.h"

namespace ti
{
	Win32UIBinding::Win32UIBinding() : UIBinding()
	{
	}

	Win32UIBinding::~Win32UIBinding()
	{
	}

	SharedPtr<MenuItem> Win32UIBinding::CreateMenu()
	{
		SharedPtr<MenuItem> menu = new Win32MenuItemImpl(NULL);
		return menu;
	}

	void Win32UIBinding::SetMenu(SharedPtr<MenuItem>)
	{
		// Notify all windows that the app menu has changed.
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			Win32UserWindow* wuw = dynamic_cast<Win32UserWindow*>(*i);
			if (wuw != NULL)
				wuw->AppMenuChanged();

			i++;
		}
	}

	void Win32UIBinding::SetIcon(SharedString icon_path)
	{
	}

	SharedPtr<TrayItem> Win32UIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		return NULL;
	}

}
