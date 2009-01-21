/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#import "osx_sound.h"
#import "sound_delegate.h"

namespace ti
{
	OSXSound::OSXSound(std::string &url) : 
		Sound(url), callback(0), sound(0), playing(false), paused(false)
	{
		theurl = [NSURL URLWithString:[NSString stringWithCString:url.c_str()]];
		[theurl retain];
		delegate = [[SoundDelegate alloc] initWithSound:this];
		this->Load();
	}
	OSXSound::~OSXSound()
	{
		this->Unload();
		[delegate release];
		[theurl release];
		
		if (callback)
		{
			delete callback;
			callback = NULL;
		}
	}
	void OSXSound::Unload()
	{
		if (sound)
		{
			if (playing)
			{
				[sound stop];	
			}
			[sound setDelegate:nil];
			[sound release];
			sound = NULL;
			playing = false;
			paused = false;
		}
	}
	void OSXSound::Load()
	{
		this->Unload();
		@try
		{
			sound = [[NSSound alloc] initWithContentsOfURL:theurl byReference:NO];
			[sound setDelegate:delegate];
		}
		@catch(NSException *ex)
		{
			throw [[ex reason] UTF8String];
		}
		@catch(...)
		{
			throw "error loading media";
		}
	}
	void OSXSound::Play()
	{
		[sound play];
		playing = true;
		paused = false;
	}
	void OSXSound::Pause()
	{
		[sound pause];
		paused = true;
		playing = false;
	}
	void OSXSound::Resume()
	{
		[sound resume];
		paused = false;
		playing = true;
	}
	void OSXSound::Stop()
	{
		if (sound && playing)
		{
			[sound stop];
		}
		paused = false;
		playing = false;
	}
	void OSXSound::Reset()
	{
		this->Load();
	}
	void OSXSound::SetVolume(double volume)
	{
		[sound setVolume:volume];
	}
	double OSXSound::GetVolume()
	{
		return [sound volume];
	}
	void OSXSound::SetLooping(bool loop)
	{
		[sound setLoops:loop];
	}
	bool OSXSound::IsLooping()
	{
		return [sound loops];
	}
	bool OSXSound::IsPlaying()
	{
		return playing;
	}
	bool OSXSound::IsPaused()
	{
		return paused;
	}
	void OSXSound::OnComplete(bool finished)
	{
		// this is called from SoundDelegate and it will be 
		// on the main thread already
		this->playing = false;
		this->paused = false;
		if (this->callback)
		{
			ValueList args;
			SharedValue arg = Value::NewBool(finished);
			args.push_back(arg);
			(*this->callback)->Call(args);
		}
	}
	void OSXSound::OnComplete(SharedBoundMethod callback)
	{
		if (this->callback)
		{
			delete this->callback;
		}
		this->callback = new SharedBoundMethod(callback);
	}
}