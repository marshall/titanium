/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/IPAddress.h>
#include "platform_binding.h"

#ifdef OS_OSX
#include <Foundation/Foundation.h>
#elif defined(OS_WIN32)
#include <windows.h>
#include <Iptypes.h>
#include <Iphlpapi.h>
#elif defined(OS_LINUX)
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
		std::string address = "127.0.0.1";

		std::vector<Poco::Net::NetworkInterface> list = Poco::Net::NetworkInterface::list();
		std::vector<Poco::Net::NetworkInterface>::iterator i = list.begin();
		SharedBoundList interfaces = new StaticBoundList();
		int c = 0;
		while (i!=list.end())
		{
			Poco::Net::NetworkInterface nitf = (*i++);
			if (nitf.supportsIPv4())
			{
				const Poco::Net::IPAddress ip = nitf.address();
				if (ip.isLoopback() || !ip.isIPv4Compatible())
				{
					continue;
				}
				c++;
				// just get the first one and bail
				if (c==1) address = ip.toString();
				// add each interface
				SharedBoundObject obj = new StaticBoundObject();
				std::string ip_addr = ip.toString();
				std::string display_name = nitf.displayName();
				std::string name = nitf.name();
				obj->Set("address",Value::NewString(ip_addr));
				obj->Set("name",Value::NewString(name));
				obj->Set("displayName",Value::NewString(display_name));
				interfaces->Append(Value::NewObject(obj));
			}
		}
		this->Set("interfaces", Value::NewList(interfaces));


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

		std::string machineid = FileUtils::GetMachineId();
#ifdef OS_OSX
		// for OSX this returns the mac address, can we just use this
		// for the other OS too??? -JGH
		std::string macid = Poco::Environment::nodeId();
#elif defined(OS_WIN32)
		//IP_ADAPTER_INFO adapter;
		PIP_ADAPTER_ADDRESSES addresses = NULL;
		//DWORD dwBufLen = sizeof(adapter);
		ULONG family = AF_UNSPEC;
		ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
		ULONG bufferLength = sizeof (IP_ADAPTER_ADDRESSES);
		addresses = (IP_ADAPTER_ADDRESSES *) malloc(bufferLength);
		// first call to check if we've allocated enough buffer space
		if (GetAdaptersAddresses(family, flags, NULL, addresses, &bufferLength) == ERROR_BUFFER_OVERFLOW) {
			free(addresses);
			addresses = (IP_ADAPTER_ADDRESSES *) malloc(bufferLength);
		}

		// "actual" call
		DWORD dwStatus = GetAdaptersAddresses(family, flags, NULL, addresses, &bufferLength);
		if (dwStatus != NO_ERROR) return;
		BYTE *MACData = addresses->PhysicalAddress;
		char buf[MAX_PATH];
		sprintf_s(buf,MAX_PATH,"%02X:%02X:%02X:%02X:%02X:%02X", MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
		std::string macid = std::string(buf);
#elif defined(OS_LINUX)
		//Based on code from:
		//http://adywicaksono.wordpress.com/2007/11/08/detecting-mac-address-using-c-application/
		std::string macid = "00:00:00:00:00:00";
		struct ifreq ifr;
		struct ifconf ifc;
		char buf[1024];
		u_char addr[6] = {'\0','\0','\0','\0','\0','\0'};
		int s,a;

		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s != -1)
		{
			ifc.ifc_len = sizeof(buf);
			ifc.ifc_buf = buf;
			ioctl(s, SIOCGIFCONF, &ifc);
			struct ifreq* IFR = ifc.ifc_req;
			bool success = false;
			for (a = ifc.ifc_len / sizeof(struct ifreq); --a >= 0; IFR++) {
				strcpy(ifr.ifr_name, IFR->ifr_name);
				if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0
					 && (!(ifr.ifr_flags & IFF_LOOPBACK))
					 && (ioctl(s, SIOCGIFHWADDR, &ifr) == 0))
				{
					success = true;
					bcopy(ifr.ifr_hwaddr.sa_data, addr, 6);
					break;
				}
			}
			close(s);

			if (success)
			{
				char mac_buf[36];
				std::ostringstream mac;
				snprintf(mac_buf, 36,
					"%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
					addr[0], addr[1],
					addr[2], addr[3],
					addr[4], addr[5]);
				macid = std::string(mac_buf);
			}
		}
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
		this->Set("id", Value::NewString(machineid));
		this->Set("macaddress", Value::NewString(macid));
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
