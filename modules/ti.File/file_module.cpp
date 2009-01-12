/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "file_module.h"
#include <kroll/kroll.h>

using namespace kroll;
using namespace ti;


namespace ti
{
	KROLL_MODULE(FileModule);
	
	void FileModule::Initialize()
	{
		// load our variables
		this->variables = new FileBinding(host->GetGlobalObject());

		// set our ti.File
		Value *value = new Value(this->variables);
		host->GetGlobalObject()->Set("File",value);
		KR_DECREF(value);
	}

	void FileModule::Destroy()
	{
		KR_DECREF(this->variables);
	}
}
