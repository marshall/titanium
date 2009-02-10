/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "osx_desktop.h"

namespace ti
{
	OSXDesktop::OSXDesktop()
	{
	}

	OSXDesktop::~OSXDesktop()
	{
	}

	bool OSXDesktop::OpenApplication(std::string& app)
	{
		NSWorkspace* ws = [NSWorkspace sharedWorkspace];
		NSString *name = [NSString stringWithCString:app.c_str()];
		return [ws launchApplication:name];
	}

	bool OSXDesktop::OpenURL(std::string& url)
	{
		NSWorkspace * ws = [NSWorkspace sharedWorkspace];
		BOOL wasOpened = [ws openURL:[NSURL URLWithString:[NSString stringWithCString:url.c_str()]]];
		return wasOpened;
	}
}
