/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _MEDIA_WIN32_H_
#define _MEDIA_WIN32_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <windows.h>

using namespace kroll;

namespace ti
{
	class Win32Media
	{
	public:
		static void CreateSound(const ValueList& args, SharedValue result);
		static void Beep(const ValueList& args, SharedValue result);
	private:
		Win32Media();
		~Win32Media();
	};
}

#endif
