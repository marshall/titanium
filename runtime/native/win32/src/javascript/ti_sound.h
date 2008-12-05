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
#ifndef TI_SOUND_H_
#define TI_SOUND_H_

#include "js_class.h"
#include "wavelib.h"

class TiSound : public JsClass
{
private:
	std::wstring path;
	NPObject *callback;
	HWAVELIB hWaveLib;

	bool loop;
	float volume;

	void initWaveLib();
	DWORD getWaveOutVolume();

public:
	TiSound(std::string url);

	void donePlaying();
	void play(const CppArgumentList &args, CppVariant *result);
	void pause(const CppArgumentList &args, CppVariant *result);
	void resume(const CppArgumentList &args, CppVariant *result);
	void stop(const CppArgumentList &args, CppVariant *result);
	void getVolume(const CppArgumentList &args, CppVariant *result);
	void setVolume(const CppArgumentList &args, CppVariant *result);
	void getLooping(const CppArgumentList &args, CppVariant *result);
	void setLooping(const CppArgumentList &args, CppVariant *result);
	void onComplete(const CppArgumentList &args, CppVariant *result);
};

#endif