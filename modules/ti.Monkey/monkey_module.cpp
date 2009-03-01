/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "monkey_module.h"
#include "monkey_test.h"

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(MonkeyModule);
	
	void MonkeyModule::Initialize()
	{
		// load our variables
		this->binding = new MonkeyBinding(host,host->GetGlobalObject());

		// set our ti.Monkey
		SharedValue value = Value::NewObject(this->binding);
		host->GetGlobalObject()->Set("Monkey", value);
	}

	void MonkeyModule::Stop()
	{
	}
	
	void MonkeyModule::Test()
	{
	  MonkeyUnitTestSuite test;
	  test.Run(host);
	}
}
