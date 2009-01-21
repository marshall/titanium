/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "menu.h"

namespace ti
{
	Menu::Menu()
	{
	}
	Menu::~Menu()
	{
	}
	void Menu::SetTitle(const ValueList& args, SharedValue result)
	{
		std::string title(args.at(0)->ToString());
		this->SetTitle(title);
	}
	void Menu::GetTitle(const ValueList& args, SharedValue result)
	{
		std::string title = this->GetTitle();
		result->SetString(title.c_str());
	}
}
