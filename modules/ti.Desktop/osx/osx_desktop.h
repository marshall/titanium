/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _DESKTOP_OSX_H_
#define _DESKTOP_OSX_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <map>
#include <vector>
#include <string>
#import <Cocoa/Cocoa.h>

using namespace kroll;

namespace ti
{
	class OSXDesktop
	{
	public:
		static bool CreateShortcut(std::string &from, std::string &to);
		static SharedBoundList OpenFiles(SharedBoundObject properties);
		static bool OpenApplication(std::string &name);
		static bool OpenURL(std::string &url);
		static int GetSystemIdleTime();
	private:
		OSXDesktop();
		~OSXDesktop();
	};
}

#endif
