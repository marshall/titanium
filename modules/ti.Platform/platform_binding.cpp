/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include <Poco/UUIDGenerator.h>
#include "platform_binding.h"

#ifdef OS_OSX
#include <Foundation/Foundation.h>
#elif defined(OS_WIN32)
#include <windows.h>
#elif defined(OS_LINUX)
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <sstream>
#endif

#if defined(OS_OSX)
@interface NSProcessInfo (LegacyWarningSurpression)
- (unsigned int) processorCount;
//TODO: do the smart thing and test for it being NSUinteger
@end
#endif


namespace ti
{
	PlatformBinding::PlatformBinding(SharedBoundObject global) : global(global)
	{
		std::string os_name = Poco::Environment::osName();
		std::string os_version = Poco::Environment::osVersion();
		std::string arch = Poco::Environment::osArchitecture();
		std::string address = Poco::Environment::nodeName();


#if defined(OS_OSX)
		int num_proc = 1;
		if ([NSProcessInfo instancesRespondToSelector:@selector(processorCount)]){
			num_proc = 	[[NSProcessInfo processInfo] processorCount];
		}
#elif defined(OS_WIN32)
		SYSTEM_INFO SysInfo ;
		GetSystemInfo (&SysInfo) ;
		DWORD num_proc = SysInfo.dwNumberOfProcessors;
#elif defined(OS_LINUX)
		int num_proc = sysconf(_SC_NPROCESSORS_ONLN);
#else
		int num_proc = 1;
#endif

#if defined(OS_LINUX)
	    std::string nodeId = "";
	    struct ifreq ifr;
	    struct ifreq *IFR;
	    struct ifconf ifc;
	    char buf[1024];
	    int s, i;
	    int ok = 0;
	    s = socket(AF_INET, SOCK_DGRAM, 0);
	    if (s!=-1) 
		{
		    ifc.ifc_len = sizeof(buf);
		    ifc.ifc_buf = buf;
		    ioctl(s, SIOCGIFCONF, &ifc);

		    IFR = ifc.ifc_req;
		    for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++) {

		        strcpy(ifr.ifr_name, IFR->ifr_name);
		        if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) {
		            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {
		                if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
		                    ok = 1;
		                    break;
		                }
		            }
		        }
		    }

		    close(s);

		    if (ok) 
			{
				std::ostringstream ostr;
				for (i = 0; i < IFHWADDRLEN; i++)
				{
					char ch[10];
					sprintf(ch,"%2.2x",ifr.ifr_hwaddr.sa_data[i]);
					ostr << ch;
					if (i+1 < IFHWADDRLEN) ostr << ":";
				}
				nodeId = ostr.str();
		    }

	    }
#else
		std::string nodeId = "";
		try
		{
			nodeId = Poco::Environment::nodeId();
		}
		catch (...) { }
#endif

//NOTE: for now we determine this at compile time -- in the future
//we might want to actually programmatically determine if running on
//64-bit processor or not...
#ifdef OS_32	
		this->Set("ostype", Value::NewString("32bit"));
#else
		this->Set("ostype", Value::NewString("64bit"));
#endif
		this->Set("name", Value::NewString(os_name));
		this->Set("version", Value::NewString(os_version));
		this->Set("architecture", Value::NewString(arch));
		this->Set("address", Value::NewString(address));
		this->Set("id", Value::NewString(nodeId));
		this->Set("processorCount", Value::NewInt(num_proc));
		std::string username = kroll::FileUtils::GetUsername();
		this->Set("username", Value::NewString(username));

		// UUID create function for the platform
		this->SetMethod("createUUID",&PlatformBinding::CreateUUID);
	}

	PlatformBinding::~PlatformBinding()
	{
	}
	
	void PlatformBinding::CreateUUID(const ValueList& args, SharedValue result)
	{
		Poco::UUID uuid = Poco::UUIDGenerator::defaultGenerator().createOne();
		result->SetString(uuid.toString().c_str());
	}
}
