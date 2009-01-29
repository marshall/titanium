/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"

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
		SharedPtr<MenuItem> menu = new Win32MenuItemImpl(true);
		return menu;
	}

	void Win32UIBinding::SetMenu(SharedPtr<MenuItem>)
	{
	}

	void Win32UIBinding::SetIcon(SharedString icon_path)
	{
	}

	SharedPtr<TrayItem> Win32UIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{

	}

}
