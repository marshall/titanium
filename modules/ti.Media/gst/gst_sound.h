/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _TI_MEDIA_GST_SOUND_H_
#define _TI_MEDIA_GST_SOUND_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <vector>

using namespace kroll;

namespace ti
{
	class GstSound : public Sound
	{
	public:
		GstSound(std::string& url);
		virtual ~GstSound();
		
		virtual void Play();
		virtual void Load();
		virtual void Pause();
		virtual void Stop();
		virtual void Reload();
		virtual void SetVolume(double volume);
		virtual double GetVolume();
		virtual void SetLooping(bool looping);
		virtual bool IsLooping();
		virtual bool IsPlaying();
		virtual bool IsPaused();
		virtual void OnComplete(SharedBoundMethod callback);

	private:
		SharedBoundMethod callback;
		GstElement *pipeline;
		bool looping;

		enum PlayState {
			PLAYING,
			STOPPED,
			PAUSED
		} state;

	};
}

#endif
