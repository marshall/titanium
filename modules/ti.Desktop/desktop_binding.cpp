/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "desktop_binding.h"
#include <kroll/kroll.h>

#ifdef OS_OSX
#include "osx/osx_desktop.h"
#elif defined(OS_WIN32)
#include "win32/win32_desktop.h"
#elif defined(OS_LINUX)
#include "linux/linux_desktop.h"
#endif

#if defined(OS_OSX)
	#define TI_DESKTOP OSXDesktop
#elif defined(OS_WIN32)
	#define TI_DESKTOP Win32Desktop
#elif defined(OS_LINUX)
	#define TI_DESKTOP LinuxDesktop
#endif

namespace ti
{
	DesktopBinding::DesktopBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("openApplication",&DesktopBinding::OpenApplication);
		this->SetMethod("openURL",&DesktopBinding::OpenURL);
		this->SetMethod("openFiles",&DesktopBinding::OpenFiles);
		this->SetMethod("getSystemIdleTime",&DesktopBinding::GetSystemIdleTime);
		this->SetMethod("createShortcut",&DesktopBinding::CreateShortcut);
	}
	DesktopBinding::~DesktopBinding()
	{
	}
	void DesktopBinding::CreateShortcut(const ValueList& args, SharedValue result)
	{
		TI_DESKTOP::CreateShortcut(args,result);
	}
	void DesktopBinding::OpenFiles(const ValueList& args, SharedValue result)
	{
		TI_DESKTOP::OpenFiles(args,result);
	}
	void DesktopBinding::OpenApplication(const ValueList& args, SharedValue result)
	{
		TI_DESKTOP::OpenApplication(args,result);
	}
	void DesktopBinding::OpenURL(const ValueList& args, SharedValue result)
	{
		TI_DESKTOP::OpenURL(args,result);
	}
	void DesktopBinding::GetSystemIdleTime(const ValueList& args, SharedValue result)
	{
		TI_DESKTOP::GetSystemIdleTime(args,result);
	}
}
