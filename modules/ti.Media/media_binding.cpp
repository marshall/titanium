/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "media_binding.h"
#include <kroll/kroll.h>

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#elif defined(OS_WIN32)
#include <windows.h>
#endif

namespace ti
{
	MediaBinding::MediaBinding(BoundObject *global) : global(global)
	{
		//KR_ADDREF(global);

		this->SetMethod("createSound",&MediaBinding::CreateSound);
		this->SetMethod("beep",&MediaBinding::Beep);
	}
	MediaBinding::~MediaBinding()
	{
		//KR_DECREF(global);
	}
	void MediaBinding::CreateSound(const ValueList& args, SharedValue result)
	{
		//TODO
	}
	void MediaBinding::Beep(const ValueList& args, SharedValue result)
	{
#ifdef OS_OSX
		NSBeep();
#elif defined(OS_WIN32)
		MessageBeep(MB_OK);
#endif
	}
}
