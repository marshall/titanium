/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include "platform_binding.h"

#ifdef OS_OSX
#include <Foundation/Foundation.h>
#elif defined(OS_WIN32)
#include <windows.h>
#endif

namespace ti
{
	PlatformBinding::PlatformBinding(SharedBoundObject global) : global(global)
	{
		this->Set("name",Value::NewString(Poco::Environment::osName().c_str()));
		this->Set("version",Value::NewString(Poco::Environment::osVersion().c_str()));
		this->Set("architecture",Value::NewString(Poco::Environment::osArchitecture().c_str()));
		this->Set("address",Value::NewString(Poco::Environment::nodeName().c_str()));
		try
		{
			this->Set("id",Value::NewString(Poco::Environment::nodeId().c_str()));
		}
		catch(...)
		{
			this->Set("id",Value::NewString(""));
		}

#ifdef OS_OSX
		NSProcessInfo *p = [NSProcessInfo processInfo];
		this->Set("processorCount",Value::NewInt([p processorCount]));
#elif defined(OS_WIN32)
		SYSTEM_INFO SysInfo ;
		GetSystemInfo ( & SysInfo ) ;
		DWORD count = SysInfo.dwNumberOfProcessors;

		this->Set("processorCount", Value::NewInt(count));
#else
		//TODO - Linux / Win32
		this->Set("processorCount",Value::NewInt(1));
#endif
	}
	PlatformBinding::~PlatformBinding()
	{
	}
}
