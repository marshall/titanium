/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _DESKTOP_WIN32_H_
#define _DESKTOP_WIN32_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <map>
#include <vector>
#include <string>
#include <windows.h>
#include <shellapi.h>

using namespace kroll;

namespace ti
{
	class Win32Desktop
	{
	public:
		static void CreateShortcut(const ValueList& args, SharedValue result);
		static void OpenFiles(const ValueList& args, SharedValue result);
		static void OpenApplication(const ValueList& args, SharedValue result);
		static void OpenURL(const ValueList& args, SharedValue result);
		static void GetSystemIdleTime(const ValueList& args, SharedValue result);
	private:
		Win32Desktop();
		~Win32Desktop();
	};
}

#endif
