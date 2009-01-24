/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>

#include "network_binding.h"
#include "tcp_socket_binding.h"
#include "ipaddress_binding.h"

namespace ti
{
	NetworkBinding::NetworkBinding(SharedBoundObject global) : global(global)
	{
		// TODO: this state change needs to be implemented
		SharedValue online = Value::NewBool(true);
		this->Set("online",online);

		this->SetMethod("onConnectivityChange",&NetworkBinding::OnConnectivityChange);
		this->SetMethod("createTCPSocket",&NetworkBinding::CreateTCPSocket);
		this->SetMethod("createIPAddress",&NetworkBinding::CreateIPAddress);
	}
	NetworkBinding::~NetworkBinding()
	{
	}
	void NetworkBinding::CreateIPAddress(const ValueList& args, SharedValue result)
	{
		SharedBoundObject ip = new IPAddressBinding(args.at(0)->ToString());
		result->SetObject(ip);
	}
	void NetworkBinding::CreateTCPSocket(const ValueList& args, SharedValue result)
	{
		//TODO: check for args
		SharedBoundObject tcp = new TCPSocketBinding(args.at(0)->ToString(), args.at(1)->ToInt());
		result->SetObject(tcp);
	}
	void NetworkBinding::OnConnectivityChange(const ValueList& args, SharedValue result)
	{
		//TODO: implement
	}
}
