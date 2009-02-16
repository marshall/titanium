/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>

#include "network_binding.h"
#include "tcp_socket_binding.h"
#include "ipaddress_binding.h"
#include "host_binding.h"
#include "irc/irc_client_binding.h"

namespace ti
{
	NetworkBinding::NetworkBinding(Host* host) : host(host), global(host->GetGlobalObject())
	{
		SharedValue online = Value::NewBool(true);
		this->Set("online",online);

		// methods that are available on Titanium.Network
		this->SetMethod("createTCPSocket",&NetworkBinding::CreateTCPSocket);
		this->SetMethod("createIRCClient",&NetworkBinding::CreateIRCClient);
		this->SetMethod("createIPAddress",&NetworkBinding::CreateIPAddress);
		this->SetMethod("getHostByName",&NetworkBinding::GetHostByName);
		this->SetMethod("getHostByAddress",&NetworkBinding::GetHostByAddress);

		this->SetMethod("addConnectivityListener",&NetworkBinding::AddConnectivityListener);
		this->SetMethod("removeConnectivityListener",&NetworkBinding::RemoveConnectivityListener);


		// NOTE: this is only used internally and shouldn't be published
		this->SetMethod("FireOnlineStatusChange",&NetworkBinding::FireOnlineStatusChange);
	}
	NetworkBinding::~NetworkBinding()
	{
#ifdef OS_OSX
		[networkDelegate release];
		networkDelegate=nil;
#elif defined(OS_WIN32)
		delete networkStatus;
#endif
	}
	void NetworkBinding::_GetByHost(std::string hostname, SharedValue result)
	{
		SharedPtr<HostBinding> binding = new HostBinding(hostname);
		if (binding->IsInvalid())
		{
			throw ValueException::FromString("Could not resolve address");
		}
		result->SetObject(binding);
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
				SharedPtr<HostBinding> binding = new HostBinding(addr);
				if (binding->IsInvalid())
				{
					throw ValueException::FromString("Could not resolve address");
				}
				result->SetObject(binding);
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
				throw ValueException::FromString("Unknown object passed");
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
		SharedPtr<IPAddressBinding> binding = new IPAddressBinding(args.at(0)->ToString());
		if (binding->IsInvalid())
		{
			throw ValueException::FromString("Invalid address");
		}
		result->SetObject(binding);
	}
	void NetworkBinding::CreateTCPSocket(const ValueList& args, SharedValue result)
	{
		//TODO: check for args
		SharedPtr<TCPSocketBinding> tcp = new TCPSocketBinding(host, args.at(0)->ToString(), args.at(1)->ToInt());
		result->SetObject(tcp);
	}
	void NetworkBinding::CreateIRCClient(const ValueList& args, SharedValue result)
	{
		SharedPtr<IRCClientBinding> irc = new IRCClientBinding(host);
		result->SetObject(irc);
	}
	void NetworkBinding::AddConnectivityListener(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1 || !args.at(0)->IsMethod())
		{
			throw ValueException::FromString("invalid argument");
		}
		SharedBoundMethod target = args.at(0)->ToMethod();
		this->listeners.push_back(target);

		// lazy add the network connectivity listener
		if (this->listeners.size()==1)
		{
			SharedBoundMethod delegate = this->Get("FireOnlineStatusChange")->ToMethod();
#ifdef OS_OSX
			networkDelegate = [[NetworkReachability alloc] initWithDelegate:delegate];
#elif defined(OS_WIN32)
			networkStatus = new Win32WMINetworkStatus(delegate);
#endif
		}
	}
	void NetworkBinding::RemoveConnectivityListener(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1 || !args.at(0)->IsMethod())
		{
			throw ValueException::FromString("invalid argument");
		}
		SharedBoundMethod target = args.at(0)->ToMethod();
		std::vector<SharedBoundMethod>::iterator it = this->listeners.begin();
		while(it!=this->listeners.end())
		{
			SharedBoundMethod m = (*it);
			if (m == target)
			{
				this->listeners.erase(it);
				result->SetBool(true);

				// once there are no more listeners, shut it down
				if (this->listeners.size()==0)
				{
		#ifdef OS_OSX
					[networkDelegate release];
					networkDelegate=nil;
		#elif defined(OS_WIN32)
					delete networkStatus;
		#endif
				}
				return;
			}
			it++;
		}
		result->SetBool(false);
	}
	void NetworkBinding::FireOnlineStatusChange(const ValueList& args, SharedValue result)
	{
		SharedValue o = args.at(0);
		this->Set("online",o);
#ifdef DEBUG
		std::cout << "ONLINE STATUS CHANGED: " << o->ToBool() << std::endl;
#endif
		// optimize by returning if no listeners
		if (this->listeners.size()==0) return;
		std::vector<SharedBoundMethod>::iterator it = this->listeners.begin();
		while(it!=this->listeners.end())
		{
			SharedBoundMethod callback = (*it++);
			try
			{
				host->InvokeMethodOnMainThread(callback,args);
			}
			catch(std::exception &e)
			{
				std::cerr << "Caught exception dispatching network event callback for online status, Error: " << e.what() << std::endl;
			}
		}
	}
}
