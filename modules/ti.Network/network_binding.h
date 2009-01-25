/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _NETWORK_BINDING_H_
#define _NETWORK_BINDING_H_

#include <kroll/kroll.h>

namespace ti
{
	class NetworkBinding : public StaticBoundObject
	{
	public:
		NetworkBinding(SharedBoundObject);
	protected:
		virtual ~NetworkBinding();
	private:
		SharedBoundObject global;
		
		void CreateIPAddress(const ValueList& args, SharedValue result);
		void CreateTCPSocket(const ValueList& args, SharedValue result);
		void _GetByHost(std::string host, SharedValue result);
		void GetHostByName(const ValueList& args, SharedValue result);
		void GetHostByAddress(const ValueList& args, SharedValue result);
		void OnConnectivityChange(const ValueList& args, SharedValue result);
	};
}

#endif
