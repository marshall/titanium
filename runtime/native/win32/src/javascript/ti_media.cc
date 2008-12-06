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

#include "ti_media.h"
#include "ti_sound.h"

#include <windows.h>

TiMedia::TiMedia()
{
	BindMethod("createSound", &TiMedia::createSound);
	BindMethod("beep", &TiMedia::beep);
}

void TiMedia::createSound(const CppArgumentList& args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isString())
	{
		TiSound *sound = new TiSound(args[0].ToString());
		createdSounds.push_back(sound);

		result->Set(sound->ToNPObject());
	}
}

void TiMedia::beep(const CppArgumentList& args, CppVariant *result)
{
	MessageBeep(MB_OK);
}

TiMedia::~TiMedia ()
{
	for (int i = 0; i < createdSounds.size(); i++) {
		delete createdSounds[i];
	}
	createdSounds.clear();
}