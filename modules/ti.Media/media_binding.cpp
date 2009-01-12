/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include "media_binding.h"
#include <kroll/kroll.h>

namespace ti
{
	MediaBinding::MediaBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
	}
	MediaBinding::~MediaBinding()
	{
		KR_DECREF(global);
	}
}
