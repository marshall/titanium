/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "win32_desktop.h"

namespace ti
{
	Win32Desktop::Win32Desktop()
	{
	}
	Win32Desktop::~Win32Desktop()
	{
	}
	bool Win32Desktop::CreateShortcut(std::string &from, std::string &to)
	{
		return false;
	}
	SharedBoundList Win32Desktop::OpenFiles(SharedBoundObject properties)
	{
		return NULL;
	}
	bool Win32Desktop::OpenApplication(std::string &name)
	{
		return false;
	}
	bool Win32Desktop::OpenURL(std::string &url)
	{
		long response = (long)ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return (response > 0);
	}
	int Win32Desktop::GetSystemIdleTime()
	{
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		return (int)idleTicks;
	}
}
