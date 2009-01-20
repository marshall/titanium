/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "win32_media.h"

namespace ti
{
	Win32Media::Win32Media()
	{
	}
	Win32Media::~Win32Media()
	{
	}
	void Win32Media::CreateSound(const ValueList& args, SharedValue result)
	{
	}
	void Win32Media::Beep(const ValueList& args, SharedValue result)
	{
		MessageBeep(MB_OK);
	}
}
