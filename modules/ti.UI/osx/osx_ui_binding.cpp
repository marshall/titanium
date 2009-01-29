/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"

namespace ti
{

	/*
	* NOTES:
	* Dynamic Setting Dock Menu / Image on OSX
	* http://developer.apple.com/samplecode/DeskPictAppDockMenu/index.html
	*/

	OSXUIBinding::OSXUIBinding() : UIBinding()
	{

	}

	OSXUIBinding::~OSXUIBinding()
	{

	}

	SharedPtr<MenuItem> OSXUIBinding::CreateMenu()
	{
		//SharedPtr<MenuItem> menu = new OSXMenuItem(true);
		SharedPtr<MenuItem> menu = NULL;
		return menu;
	}

	void OSXUIBinding::SetMenu(SharedPtr<MenuItem>)
	{
	}

	void OSXUIBinding::SetIcon(SharedString icon_path)
	{
	}

	SharedPtr<TrayItem> OSXUIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		SharedPtr<TrayItem> item = NULL;
		return item;
	}

	void OSXUIBinding::SetDockIcon(SharedString icon_path)
	{

	}

	void OSXUIBinding::SetDockMenu(SharedPtr<MenuItem>)
	{

	}

	void OSXUIBinding::SetBadge(SharedString badge_path)
	{

	}

}
