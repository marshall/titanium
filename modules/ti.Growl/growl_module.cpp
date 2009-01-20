/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "growl_module.h"
#include "growl_test.h"

#if defined(OS_OSX)
#include "osx/growl_osx.h"
#elif defined(OS_WIN32)
#include "win32/snarl_win32.h"
#endif

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(GrowlModule);

	void GrowlModule::Initialize()
	{

#if defined(OS_OSX)
		// load our variables
		binding = new GrowlOSX(host->GetGlobalObject());
		binding->CopyToApp(host, this);
#elif defined(OS_WIN32)
		binding = new SnarlWin32(host->GetGlobalObject());
#endif

		// set our ti.Growl
		SharedValue value = Value::NewObject(binding);
		host->GetGlobalObject()->Set("Growl", value);
	}

	void GrowlModule::Destroy()
	{
	}

	void GrowlModule::Test()
	{
	  GrowlUnitTestSuite test;
	  test.Run(host);
	}
}
