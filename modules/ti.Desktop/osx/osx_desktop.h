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
		static void CreateShortcut(const ValueList& args, SharedValue result);
		static void OpenFiles(const ValueList& args, SharedValue result);
		static void OpenApplication(const ValueList& args, SharedValue result);
		static void OpenURL(const ValueList& args, SharedValue result);
		static void GetSystemIdleTime(const ValueList& args, SharedValue result);
	private:
		OSXDesktop();
		~OSXDesktop();
	};
}

#endif
