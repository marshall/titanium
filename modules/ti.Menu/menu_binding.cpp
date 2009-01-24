/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "menu_binding.h"

namespace ti
{
	MenuBinding::MenuBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("createAppMenu",&MenuBinding::CreateAppMenu);
		this->SetMethod("createTrayMenu",&MenuBinding::CreateAppMenu);
		this->SetMethod("createDockMenu",&MenuBinding::CreateDockMenu);
	}
	MenuBinding::~MenuBinding()
	{
	}
	void MenuBinding::CreateAppMenu(const ValueList& args, SharedValue result)
	{
		MenuItem* item = new MenuItem();
		printf("root menu: %x\n", (int) item);
		SharedBoundList so = SharedBoundList(item);
		result->SetList(so);
	}
	void MenuBinding::CreateTrayMenu(const ValueList& args, SharedValue result)
	{
	}
	void MenuBinding::CreateDockMenu(const ValueList& args, SharedValue result)
	{
	}
}


//NOTES:
//Dynamic Setting Dock Menu / Image on OSX
//http://developer.apple.com/samplecode/DeskPictAppDockMenu/index.html
