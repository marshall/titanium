/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "database_module.h"
#include "database_binding.h"

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(DatabaseModule);
	
	void DatabaseModule::Initialize()
	{
		// load our variables
		this->binding = new DatabaseBinding(host);

		// set our ti.Database
		SharedValue value = Value::NewObject(this->binding);
		host->GetGlobalObject()->Set("Database", value);
	}

	void DatabaseModule::Stop()
	{
	}
	
}
