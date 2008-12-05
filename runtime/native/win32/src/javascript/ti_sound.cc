/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ti_sound.h"
#include "ti_url.h"

#include <math.h>
#include <windows.h>

TiSound::TiSound(std::string url)
	: loop(false), hWaveLib(NULL), callback(NULL), volume(1.0)
{
	path = TiURL::getPathForURL(GURL(url));

	BindMethod("play", &TiSound::play);
	BindMethod("pause", &TiSound::pause);
	BindMethod("resume", &TiSound::resume);
	BindMethod("stop", &TiSound::stop);
	BindMethod("getVolume", &TiSound::getVolume);
	BindMethod("setVolume", &TiSound::setVolume);
	BindMethod("getLooping", &TiSound::getLooping);
	BindMethod("setLooping", &TiSound::setLooping);
	BindMethod("onComplete", &TiSound::onComplete);
}

extern "C" {
void __cdecl DonePlaying(void *data)
{
	TiSound *sound = (TiSound *)data;
	sound->donePlaying();
}
}

void TiSound::donePlaying()
{
	WaveLib_UnInit(hWaveLib);
	hWaveLib = NULL;

	if (callback != NULL) {
		NPVariant result;
		NPN_InvokeDefault(0, callback, NULL, 0, &result);
	}
}

DWORD TiSound::getWaveOutVolume()
{
	return (DWORD)(floor(this->volume * 0xFFFF));
}

void TiSound::initWaveLib()
{
	hWaveLib = WaveLib_Init(path.c_str(), (BOOL)loop, FALSE, &DonePlaying, (void*)this);
	if (hWaveLib != NULL) {
		HWAVEOUT hWaveOut = WaveLib_GetWaveOut(hWaveLib);
		waveOutSetVolume(hWaveOut, getWaveOutVolume());
	}
}

void TiSound::play(const CppArgumentList &args, CppVariant *result)
{
	if (hWaveLib == NULL) {
		initWaveLib();
	} else {
		hWaveLib = NULL;
		initWaveLib();
	}
}

void TiSound::pause(const CppArgumentList &args, CppVariant *result)
{
	if (hWaveLib != NULL)
		WaveLib_Pause(hWaveLib, TRUE);
}

void TiSound::resume(const CppArgumentList &args, CppVariant *result)
{
	if (hWaveLib != NULL)
		WaveLib_Pause(hWaveLib, FALSE);
}

void TiSound::stop(const CppArgumentList &args, CppVariant *result)
{
	if (hWaveLib != NULL) {
		WaveLib_UnInit(hWaveLib);
		hWaveLib = NULL;
	}
}

void TiSound::getVolume(const CppArgumentList &args, CppVariant *result)
{
	result->Set(this->volume);
}

void TiSound::setVolume(const CppArgumentList &args, CppVariant *result)
{	
	if (args.size() > 0 && args[0].isNumber() && args[0].ToDouble() <= 1) {
		this->volume = args[0].ToDouble();

		if (hWaveLib != NULL) {
			HWAVEOUT hWaveOut = WaveLib_GetWaveOut(hWaveLib);
			waveOutSetVolume(hWaveOut, getWaveOutVolume());
		}
	}
}

void TiSound::getLooping(const CppArgumentList &args, CppVariant *result)
{
	result->Set(loop);
}

void TiSound::setLooping(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool()) {
		loop = args[0].ToBoolean();
		if (hWaveLib != NULL)
			WaveLib_Loop(hWaveLib, loop);
	}
}

void TiSound::onComplete(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isObject()) {
		NPVariant variant;
		args[0].CopyToNPVariant(&variant);

		callback = NPVARIANT_TO_OBJECT(variant);
	}
}