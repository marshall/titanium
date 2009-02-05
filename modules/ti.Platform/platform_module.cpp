/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "platform_module.h"
#include "platform_test.h"

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(PlatformModule);
	
	void PlatformModule::Initialize()
	{
		// load our variables
		this->binding = new PlatformBinding(host->GetGlobalObject());

		// set our ti.Platform
		SharedValue value = Value::NewObject(this->binding);
		host->GetGlobalObject()->Set("Platform", value);
	}

	void PlatformModule::Stop()
	{
	}
	
	void PlatformModule::Test()
	{
	  PlatformUnitTestSuite test;
	  test.Run(host);
	}
}
