/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "ui_module.h"

namespace ti
{
	MenuBinding::MenuBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("createMenu", &MenuBinding::CreateMenu);
	}

	MenuBinding::~MenuBinding()
	{
	}

	void MenuBinding::CreateMenu(const ValueList& args, SharedValue result)
	{
#ifdef OS_LINUX
		MenuItem* item = new GtkMenuItemImpl(this->global, true);
#endif
		SharedBoundList so = SharedBoundList(item);
		result->SetList(so);
	}

}


//NOTES:
//Dynamic Setting Dock Menu / Image on OSX
//http://developer.apple.com/samplecode/DeskPictAppDockMenu/index.html
