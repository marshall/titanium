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
	NetworkBinding::NetworkBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
		
		// TODO: this state change needs to be implemented
		Value *online = new Value(true);
		this->Set("online",online);
		KR_DECREF(online);
		
		this->SetMethod("onConnectivityChange",&NetworkBinding::OnConnectivityChange);
		this->SetMethod("createTCPSocket",&NetworkBinding::Create);
	}
	NetworkBinding::~NetworkBinding()
	{
		KR_DECREF(global);
	}
	void NetworkBinding::Create(const ValueList& args, Value *result)
	{
		BoundObject *tcp = new TCPSocketBinding(args.at(0)->ToString(), args.at(1)->ToInt());
		result->Set(tcp);
		KR_DECREF(tcp);
	}
	void NetworkBinding::OnConnectivityChange(const ValueList& args, Value *result)
	{
		//TODO: implement
	}
}
