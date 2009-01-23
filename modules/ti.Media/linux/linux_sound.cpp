/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "linux_sound.h"

namespace ti
{
	LinuxSound::LinuxSound(std::string &url) : Sound(url), callback(0)
	{
	}
	LinuxSound::~LinuxSound()
	{
		if (callback)
		{
			delete callback;
		}
	}
	void LinuxSound::Play()
	{
	}
	void LinuxSound::Pause()
	{
	}
	void LinuxSound::Resume()
	{
	}
	void LinuxSound::Stop()
	{
	}
	void LinuxSound::Reset()
	{
	}
	void LinuxSound::SetVolume(double volume)
	{
	}
	double LinuxSound::GetVolume()
	{
		return 0.0;
	}
	void LinuxSound::SetLooping(bool loop)
	{
	}
	bool LinuxSound::IsLooping()
	{
		return false;
	}
	bool LinuxSound::IsPlaying()
	{
		return false;
	}
	bool LinuxSound::IsPaused()
	{
		return false;
	}
	void LinuxSound::OnComplete(SharedBoundMethod callback)
	{
		if (this->callback)
		{
			delete this->callback;
		}
		this->callback = new SharedBoundMethod(callback.get());
	}
}
