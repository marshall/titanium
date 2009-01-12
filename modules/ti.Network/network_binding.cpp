/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include "network_binding.h"
#include <kroll/kroll.h>

namespace ti
{
	NetworkBinding::NetworkBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
	}
	NetworkBinding::~NetworkBinding()
	{
		KR_DECREF(global);
	}
}
