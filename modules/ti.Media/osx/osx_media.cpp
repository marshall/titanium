/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "osx_media.h"

namespace ti
{
	OSXMedia::OSXMedia()
	{
	}
	OSXMedia::~OSXMedia()
	{
	}
	void OSXMedia::CreateSound(const ValueList& args, SharedValue result)
	{
	}
	void OSXMedia::Beep(const ValueList& args, SharedValue result)
	{
		NSBeep();
	}
}
