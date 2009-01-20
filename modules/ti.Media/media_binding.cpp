/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "media_binding.h"
#include <kroll/kroll.h>

#ifdef OS_OSX
#include "osx/osx_media.h"
#elif defined(OS_WIN32)
#include "win32/win32_media.h"
#elif defined(OS_LINUX)
#include "linux/linux_media.h"
#endif

#if defined(OS_OSX)
	#define TI_MEDIA OSXMedia
#elif defined(OS_WIN32)
	#define TI_MEDIA Win32Media
#elif defined(OS_LINUX)
	#define TI_MEDIA LinuxMedia
#endif


namespace ti
{
	MediaBinding::MediaBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("createSound",&MediaBinding::CreateSound);
		this->SetMethod("beep",&MediaBinding::Beep);
	}
	MediaBinding::~MediaBinding()
	{
	}
	void MediaBinding::CreateSound(const ValueList& args, SharedValue result)
	{
		TI_MEDIA::CreateSound(args,result);
	}
	void MediaBinding::Beep(const ValueList& args, SharedValue result)
	{
		TI_MEDIA::Beep(args,result);
	}
}
