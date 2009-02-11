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
		const char *os_name = Poco::Environment::osName().c_str();
		const char *os_version = Poco::Environment::osVersion().c_str();
		const char *arch = Poco::Environment::osArchitecture().c_str();
		const char *address = Poco::Environment::nodeName().c_str();

		
#if defined(OS_OSX)
		int num_proc = [[NSProcessInfo processInfo] processorCount];
#elif defined(OS_WIN32)
		SYSTEM_INFO SysInfo ;
		GetSystemInfo (&SysInfo) ;
		DWORD num_proc = SysInfo.dwNumberOfProcessors;
#else
		// Is there an easy way to do this in Linux without external
		// libraries or running "cat /proc/cpuinfo" ?
		int num_proc = 1;
#endif

		char *nodeId = "";
		try
		{
			nodeId = (char*)Poco::Environment::nodeId().c_str();
		}
		catch (...) { }

		this->Set("name", Value::NewString(os_name));
		this->Set("version", Value::NewString(os_version));
		this->Set("architecture", Value::NewString(arch));
		this->Set("address", Value::NewString(address));
		this->Set("id", Value::NewString(nodeId));
		this->Set("processorCount", Value::NewInt(num_proc));
		this->Set("username", Value::NewString(kroll::FileUtils::GetUsername()));

		}

	PlatformBinding::~PlatformBinding()
	{
	}
}
