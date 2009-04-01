/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "sound.h"

namespace ti
{
	Sound::Sound (std::string &s) : url(s)
	{
		/**
		 * @tiapi(method=True,name=Media.Sound.play,since=0.2) starting playing the sound
		 */
		this->SetMethod("play",&Sound::Play);
		/**
		 * @tiapi(method=True,name=Media.Sound.pause,since=0.2) pause a playing sound
		 */
		this->SetMethod("pause",&Sound::Pause);
		/**
		 * @tiapi(method=True,name=Media.Sound.stop,since=0.2) stop a playing sound
		 */
		this->SetMethod("stop",&Sound::Stop);
		/**
		 * @tiapi(method=True,name=Media.Sound.reload,since=0.2) reload a playing sound
		 */
		this->SetMethod("reload",&Sound::Reload);
		/**
		 * @tiapi(method=True,name=Media.Sound.setVolume,since=0.2) set the volume from 0.0-1.0
		 * @tiarg(for=Media.Sound.setVolume,type=double,name=volume) volume from 0.0-1.0
		 */
		this->SetMethod("setVolume",&Sound::SetVolume);
		/**
		 * @tiapi(method=True,name=Media.Sound.getVolume,since=0.2) get the volume from 0.0-1.0
		 * @tiresult(for=Media.Sound.getVolume,type=double) return the volume as 0.0-1.0
		 */
		this->SetMethod("getVolume",&Sound::GetVolume);
		/**
		 * @tiapi(method=True,name=Media.Sound.setLooping,since=0.2) sets the looping of the sound
		 * @tiarg(for=Media.Sound.setLooping,name=loop,type=boolean) true to set looping
		 */
		this->SetMethod("setLooping",&Sound::SetLooping);
		/**
		 * @tiapi(method=True,name=Media.Sound.isLooping,since=0.2) returns true if the sound is looping
		 * @tiresult(for=Media.Sound.isLooping,type=boolean) returns true if looping
		 */
		this->SetMethod("isLooping",&Sound::IsLooping);
		/**
		 * @tiapi(method=True,name=Media.Sound.isPlaying,since=0.2) returns true if the sound is playing
		 * @tiresult(for=Media.Sound.isPlaying,type=boolean) returns true if playing
		 */
		this->SetMethod("isPlaying",&Sound::IsPlaying);
		/**
		 * @tiapi(method=True,name=Media.Sound.isPaused,since=0.2) returns true if the sound is paused
		 * @tiresult(for=Media.Sound.isPaused,type=boolean) returns true if paused
		 */
		this->SetMethod("isPaused",&Sound::IsPaused);
		/**
		 * @tiapi(method=True,name=Media.Sound.onComplete,since=0.2) set the oncomplete function callback
		 * @tiarg(for=Media.Sound.onComplete,type=method,name=callback) callback method
		 */
		this->SetMethod("onComplete",&Sound::OnComplete);
	}
	Sound::~Sound()
	{
	}
	void Sound::Play(const ValueList& args, SharedValue result)
	{
		this->Play();
	}
	void Sound::Pause(const ValueList& args, SharedValue result)
	{
		if (!this->IsPlaying())
		{
			throw ValueException::FromString("Sounds is not currently playing");
		}
		this->Pause();
	}
	void Sound::Stop(const ValueList& args, SharedValue result)
	{
		if (!this->IsPlaying() && !this->IsPaused())
		{
			return;
		}
		this->Stop();
	}
	void Sound::Reload(const ValueList& args, SharedValue result)
	{
		this->Reload();
	}
	void Sound::SetVolume(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1)
		{
			throw ValueException::FromString("setVolume takes 1 parameter");
		}
		this->SetVolume(args.at(0)->ToDouble());
	}
	void Sound::GetVolume(const ValueList& args, SharedValue result)
	{
		result->SetDouble(this->GetVolume());
	}
	void Sound::SetLooping(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1)
		{
			throw ValueException::FromString("setLooping takes 1 parameter");
		}
		this->SetLooping(args.at(0)->ToBool());
	}
	void Sound::IsLooping(const ValueList& args, SharedValue result)
	{
		result->SetBool(this->IsLooping());
	}
	void Sound::IsPlaying(const ValueList& args, SharedValue result)
	{
		result->SetBool(this->IsPlaying());
	}
	void Sound::IsPaused(const ValueList& args, SharedValue result)
	{
		result->SetBool(this->IsPaused());
	}
	void Sound::OnComplete(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1)
		{
			throw ValueException::FromString("onComplete takes 1 parameter");
		}
		if (!args.at(0)->IsMethod())
		{
			throw ValueException::FromString("onComplete takes a function parameter");
		}
		SharedBoundMethod method = args.at(0)->ToMethod();
		this->OnComplete(method);
	}
}
