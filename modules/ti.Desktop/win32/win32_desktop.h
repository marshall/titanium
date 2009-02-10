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
		static bool OpenApplication(std::string &name);
		static bool OpenURL(std::string &url);
	private:
		Win32Desktop();
		~Win32Desktop();

	};
}

#endif
