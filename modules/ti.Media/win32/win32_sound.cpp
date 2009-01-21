/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "win32_sound.h"

namespace ti
{
	Win32Sound::Win32Sound(std::string &url) : Sound(url), callback(0)
	{
	}
	Win32Sound::~Win32Sound()
	{
		if (callback)
		{
			delete callback;
		}
	}
	void Win32Sound::Play()
	{
	}
	void Win32Sound::Pause()
	{
	}
	void Win32Sound::Resume()
	{
	}
	void Win32Sound::Stop()
	{
	}
	void Win32Sound::Reset()
	{
	}
	void Win32Sound::SetVolume(double volume)
	{
	}
	double Win32Sound::GetVolume()
	{
		return 0.0;
	}
	void Win32Sound::SetLooping(bool loop)
	{
	}
	bool Win32Sound::IsLooping()
	{
		return false;
	}
	bool Win32Sound::IsPlaying()
	{
		return false;
	}
	bool Win32Sound::IsPaused()
	{
		return false;
	}
	void Win32Sound::OnComplete(SharedBoundMethod callback)
	{
		if (this->callback)
		{
			delete this->callback;
		}
		this->callback = new SharedBoundMethod(callback.get());
	}
}