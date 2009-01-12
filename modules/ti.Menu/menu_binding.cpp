/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "menu_binding.h"

namespace ti
{
	MenuBinding::MenuBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
	}
	MenuBinding::~MenuBinding()
	{
		KR_DECREF(global);
	}
}
