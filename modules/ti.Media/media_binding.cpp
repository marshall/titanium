/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "media_binding.h"
#include <kroll/kroll.h>

#ifdef OS_OSX
#include "osx/osx_media.h"
#include "osx/osx_sound.h"
#elif defined(OS_WIN32)
#include "win32/win32_media.h"
#include "win32/win32_sound.h"
#elif defined(OS_LINUX)
#include "linux/linux_media.h"
#include "linux/linux_sound.h"
#endif

#if defined(OS_OSX)
	#define TI_MEDIA OSXMedia
	#define TI_SOUND OSXSound
#elif defined(OS_WIN32)
	#define TI_MEDIA Win32Media
	#define TI_SOUND Win32Sound
#elif defined(OS_LINUX)
	#define TI_MEDIA LinuxMedia
	#define TI_SOUND LinuxSound
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
		if (args.size()!=1)
		{
			throw "invalid parameters passed. createSound takes 1 parameter";
		}
		std::string path(args.at(0)->ToString());
		SharedBoundObject sound = new TI_SOUND(path);
		result->SetObject(sound);
	}
	void MediaBinding::Beep(const ValueList& args, SharedValue result)
	{
		TI_MEDIA::Beep();
	}
}
