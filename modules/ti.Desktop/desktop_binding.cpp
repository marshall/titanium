/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
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
	}

	DesktopBinding::~DesktopBinding()
	{
	}

	void DesktopBinding::OpenApplication(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1)
		{
			throw ValueException::FromString("openApplication takes 1 parameter");
		}
		std::string app = args.at(0)->ToString();
		result->SetBool(TI_DESKTOP::OpenApplication(app));
	}

	void DesktopBinding::OpenURL(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1)
		{
			throw ValueException::FromString("openURL takes 1 parameter");
		}
		std::string url = args.at(0)->ToString();
		result->SetBool(TI_DESKTOP::OpenURL(url));
	}
}
