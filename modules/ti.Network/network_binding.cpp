/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "network_binding.h"
#include "tcp_socket_binding.h"
#include <kroll/kroll.h>

namespace ti
{
	NetworkBinding::NetworkBinding(SharedBoundObject global) : global(global)
	{
		// TODO: this state change needs to be implemented
		SharedValue online = Value::NewBool(true);
		this->Set("online",online);

		this->SetMethod("onConnectivityChange",&NetworkBinding::OnConnectivityChange);
		this->SetMethod("createTCPSocket",&NetworkBinding::Create);
	}
	NetworkBinding::~NetworkBinding()
	{
	}
	void NetworkBinding::Create(const ValueList& args, SharedValue result)
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
