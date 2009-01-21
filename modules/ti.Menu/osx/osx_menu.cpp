/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "osx_menu.h"

namespace ti
{
	OSXMenu::OSXMenu(NSMenu *menu) : menu(menu)
	{
		[menu retain];
	}
	OSXMenu::~OSXMenu()
	{
		[menu release];
	}
	void OSXMenu::SetTitle(std::string &title)
	{
		NSString *t = [NSString stringWithCString:title.c_str()];
		[menu setTitle:t];
	}
	std::string OSXMenu::GetTitle()
	{
		return std::string([[menu title] UTF8String]);
	}
}
