/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _NETWORK_BINDING_H_
#define _NETWORK_BINDING_H_

#include <kroll/kroll.h>
#if defined(OS_OSX)
#include "osx/network_status.h"
#elif defined(OS_WIN32)
#include "win32/win32_wmi_network_status.h"
#endif

namespace ti
{
	class NetworkBinding : public StaticBoundObject
	{
	public:
		NetworkBinding(Host*);
		virtual ~NetworkBinding();

	private:
		Host* host;
		SharedBoundObject global;
		std::vector<SharedBoundMethod> listeners;

#if defined(OS_OSX)
		NetworkReachability *networkDelegate;
#elif defined(OS_WIN32)
		Win32WMINetworkStatus *networkStatus;
#endif

		void CreateIPAddress(const ValueList& args, SharedValue result);
		void CreateTCPSocket(const ValueList& args, SharedValue result);
		void CreateIRCClient(const ValueList& args, SharedValue result);

		void _GetByHost(std::string host, SharedValue result);
		void GetHostByName(const ValueList& args, SharedValue result);
		void GetHostByAddress(const ValueList& args, SharedValue result);
		void AddConnectivityListener(const ValueList& args, SharedValue result);
		void RemoveConnectivityListener(const ValueList& args, SharedValue result);
		void FireOnlineStatusChange(const ValueList& args, SharedValue result);
	};
}

#endif
