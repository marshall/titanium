/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "linux_desktop.h"

namespace ti
{
	LinuxDesktop::LinuxDesktop()
	{
	}
	LinuxDesktop::~LinuxDesktop()
	{
	}
	bool LinuxDesktop::CreateShortcut(std::string &from, std::string &to)
	{
		return false;
	}
	bool LinuxDesktop::OpenFiles(SharedBoundObject properties)
	{
		return false;
	}
	bool LinuxDesktop::OpenApplication(std::string &name)
	{
		return false;
	}
	bool LinuxDesktop::OpenURL(std::string &url)
	{
		return false;
	}
	int LinuxDesktop::GetSystemIdleTime()
	{
		return -1;
	}
}
