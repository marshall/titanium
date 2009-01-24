/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>

#include "network_binding.h"
#include "tcp_socket_binding.h"
#include "ipaddress_binding.h"
#include "host_binding.h"

namespace ti
{
	NetworkBinding::NetworkBinding(SharedBoundObject global) : global(global)
	{
		// TODO: this state change needs to be implemented
		SharedValue online = Value::NewBool(true);
		this->Set("online",online);

		// methods that are available on Titanium.Network
		this->SetMethod("onConnectivityChange",&NetworkBinding::OnConnectivityChange);
		this->SetMethod("createTCPSocket",&NetworkBinding::CreateTCPSocket);
		this->SetMethod("createIPAddress",&NetworkBinding::CreateIPAddress);
		this->SetMethod("getHostByName",&NetworkBinding::GetHostByName);
		this->SetMethod("getHostByAddress",&NetworkBinding::GetHostByAddress);
	}
	NetworkBinding::~NetworkBinding()
	{
	}
	void NetworkBinding::_GetByHost(std::string hostname, SharedValue result)
	{
		HostBinding *binding = new HostBinding(hostname);
		if (binding->IsInvalid())
		{
			throw Value::NewString("could not resolve address");
		}
		SharedBoundObject host = binding;
		result->SetObject(host);
	}
	void NetworkBinding::GetHostByAddress(const ValueList& args, SharedValue result)
	{
		if (args.at(0)->IsObject())
		{
			SharedBoundObject obj = args.at(0)->ToObject();
			SharedPtr<IPAddressBinding> b = obj.cast<IPAddressBinding>();
			if (!b.isNull())
			{
				// in this case, they've passed us an IPAddressBinding
				// object, which we can just retrieve the ipaddress
				// instance and resolving using it
				IPAddress addr(b->GetAddress()->toString());
				HostBinding *binding = new HostBinding(addr);
				if (binding->IsInvalid())
				{
					throw Value::NewString("could not resolve address");
				}
				SharedBoundObject host = binding;
				result->SetObject(host);
				return;
			}
			else
			{
				SharedValue bo = obj->Get("toString");
				if (bo->IsMethod())
				{
					SharedBoundMethod m = bo->ToMethod();
					ValueList args;
					SharedValue tostr = m->Call(args);
					this->_GetByHost(tostr->ToString(),result);
					return;
				}
				throw Value::NewString("unknown object passed");
			}
		}
		else if (args.at(0)->IsString())
		{
			// in this case, they just passed in a string so resolve as 
			// normal
			this->_GetByHost(args.at(0)->ToString(),result);
		}
	}
	void NetworkBinding::GetHostByName(const ValueList& args, SharedValue result)
	{
		this->_GetByHost(args.at(0)->ToString(),result);
	}
	void NetworkBinding::CreateIPAddress(const ValueList& args, SharedValue result)
	{
		IPAddressBinding *binding = new IPAddressBinding(args.at(0)->ToString());
		if (binding->IsInvalid())
		{
			throw Value::NewString("invalid address");
		}
		SharedBoundObject ip = binding;
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
