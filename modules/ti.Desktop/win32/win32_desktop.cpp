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
	void Win32Desktop::CreateShortcut(const ValueList& args, SharedValue result)
	{
	}
	void Win32Desktop::OpenFiles(const ValueList& args, SharedValue result)
	{
	}
	void Win32Desktop::OpenApplication(const ValueList& args, SharedValue result)
	{
	}
	void Win32Desktop::OpenURL(const ValueList& args, SharedValue result)
	{
		long response = (long)ShellExecuteA(NULL, "open", args.at(0)->ToString(), NULL, NULL, SW_SHOWNORMAL);
		result->SetBool(response > 0);
	}
	void Win32Desktop::GetSystemIdleTime(const ValueList& args, SharedValue result)
	{
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		result->SetInt((int)idleTicks);
	}
}
