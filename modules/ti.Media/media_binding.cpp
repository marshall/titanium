/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include "media_binding.h"
#include <kroll/kroll.h>

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#endif

namespace ti
{
	MediaBinding::MediaBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
		
		this->SetMethod("createSound",&MediaBinding::CreateSound);
		this->SetMethod("beep",&MediaBinding::Beep);
	}
	MediaBinding::~MediaBinding()
	{
		KR_DECREF(global);
	}
	void MediaBinding::CreateSound(const ValueList& args, Value *result)
	{
		//TODO
	}
	void MediaBinding::Beep(const ValueList& args, Value *result)
	{
#ifdef OS_OSX
		NSBeep();
#endif
	}
}
