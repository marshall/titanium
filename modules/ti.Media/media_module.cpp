/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "media_module.h"
#include "media_binding.h"

using namespace kroll;
using namespace ti;


namespace ti
{
	KROLL_MODULE(MediaModule);
	
	void MediaModule::Initialize()
	{
		// load our variables
		this->variables = new MediaBinding(host->GetGlobalObject());
		
		// set our ti.Media
		Value *value = new Value(this->variables);
		host->GetGlobalObject()->Set("Media",value);
	}

	void MediaModule::Destroy()
	{
		KR_DECREF(this->variables);
	}
}
