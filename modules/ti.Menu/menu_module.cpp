/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "menu_module.h"
#include "menu_test.h"

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(MenuModule);

	void MenuModule::Initialize()
	{
		// load our variables
		this->variables = new MenuBinding(host->GetGlobalObject());

		// set our ti.Menu
		SharedValue value = Value::NewObject(this->variables);
		host->GetGlobalObject()->Set("Menu",value);
	}

	void MenuModule::Destroy()
	{
	}

	void MenuModule::Test()
	{
	  MenuUnitTestSuite test;
	  test.Run(host);
	}
}
