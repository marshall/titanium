/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include "desktop_binding.h"
#include <kroll/kroll.h>

namespace ti
{
	DesktopBinding::DesktopBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
	}
	DesktopBinding::~DesktopBinding()
	{
		KR_DECREF(global);
	}
	void DesktopBinding::CreateShortcut(const ValueList& args, Value *result)
	{
	}
	void DesktopBinding::OpenFiles(const ValueList& args, Value *result)
	{
	}
}
