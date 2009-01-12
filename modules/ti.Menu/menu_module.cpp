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
		Value *value = new Value(this->variables);
		host->GetGlobalObject()->Set("File",value);
		KR_DECREF(value);
	}

	void MenuModule::Destroy()
	{
		KR_DECREF(this->variables);
	}
	
	void MenuModule::Test()
	{
	  MenuUnitTestSuite test;
	  test.Run(host);
	}
}
