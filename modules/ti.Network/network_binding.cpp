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
#include "http/http_client_binding.h"

namespace ti
{
	std::vector<SharedBoundObject> NetworkBinding::bindings;
	NetworkBinding::NetworkBinding(Host* host) :
		 host(host),
		 global(host->GetGlobalObject()),
		 next_listener_id(0)
	{
   		SharedValue online = Value::NewBool(true);
		/**
		 * @tiapi(property=True,returns=boolean,name=Network.online) returns true if the machine is connected to the Internet
		 */
		this->Set("online", online);

		// methods that are available on Titanium.Network
		/**
		 * @tiapi(method=True,returns=object,name=Network.createTCPSocket) creates a TCP client socket
		 */
		this->SetMethod("createTCPSocket",&NetworkBinding::CreateTCPSocket);
		/**
		 * @tiapi(method=True,returns=object,name=Network.createIRCClient) creates an IRC client socket
		 */
		this->SetMethod("createIRCClient",&NetworkBinding::CreateIRCClient);
		/**
		 * @tiapi(method=True,returns=object,name=Network.createIPAddress) creates IP Address object
		 */
		this->SetMethod("createIPAddress",&NetworkBinding::CreateIPAddress);
		/**
		 * @tiapi(method=True,returns=object,name=Network.createHTTPClient) creates an HTTP client
		 */
		this->SetMethod("createHTTPClient",&NetworkBinding::CreateHTTPClient);
		/**
		 * @tiapi(method=True,returns=object,name=Network.getHostByName) convert a host by name into a Host object
		 */
		this->SetMethod("getHostByName",&NetworkBinding::GetHostByName);
		/**
		 * @tiapi(method=True,returns=object,name=Network.getHostByAddress) convert a host by ip into a Host object
		 */
		this->SetMethod("getHostByAddress",&NetworkBinding::GetHostByAddress);
		/**
		 * @tiapi(method=True,returns=object,name=Network.encodeURIComponent) encode a URI component
		 */
		this->SetMethod("encodeURIComponent",&NetworkBinding::EncodeURIComponent);
		/**
		 * @tiapi(method=True,returns=object,name=Network.decodeURIComponent) decode a URI component
		 */
		this->SetMethod("decodeURIComponent",&NetworkBinding::DecodeURIComponent);

		/**
		 * @tiapi(method=True,returns=int,name=Network.addConnectivityListener) add a connectivity change listener. returns an id to be used when removing.
		 */
		this->SetMethod("addConnectivityListener",&NetworkBinding::AddConnectivityListener);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.removeConnectivityListener) remove a connectivity change listener
		 */
		this->SetMethod("removeConnectivityListener",&NetworkBinding::RemoveConnectivityListener);


		// NOTE: this is only used internally and shouldn't be published
		this->SetMethod("FireOnlineStatusChange",&NetworkBinding::FireOnlineStatusChange);

#if defined(OS_LINUX)
		this->net_status = new DBusNetworkStatus(this);
		this->net_status->Start();
#elif defined(OS_OSX)
		SharedBoundMethod delegate = this->Get("FireOnlineStatusChange")->ToMethod();
		networkDelegate = [[NetworkReachability alloc] initWithDelegate:delegate];
#endif
	}

	NetworkBinding::~NetworkBinding()
	{
#if defined(OS_OSX)
		[networkDelegate release];
		networkDelegate=nil;
#else
		delete this->net_status;
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
	void NetworkBinding::RemoveBinding(void* binding)
	{
		std::vector<SharedBoundObject>::iterator i = bindings.begin();
		while(i!=bindings.end())
		{
			SharedBoundObject b = (*i);
			if (binding == b.get())
			{
				bindings.erase(i);
				break;
			}
			i++;
		}
	}
	void NetworkBinding::CreateHTTPClient(const ValueList& args, SharedValue result)
	{
		HTTPClientBinding* http = new HTTPClientBinding(host);
		SharedBoundObject obj = http->GetSelf()->ToObject();
		// we hold the reference to this until we're done with it
		// which happense when the binding impl calls remove
		this->bindings.push_back(obj);
		result->SetObject(obj);
	}
	void NetworkBinding::AddConnectivityListener(const ValueList& args, SharedValue result)
	{
		ArgUtils::VerifyArgsException("addConnectivityListener", args, "m");
		SharedBoundMethod target = args.at(0)->ToMethod();

		Listener listener = Listener();
		listener.id = this->next_listener_id++;
		listener.callback = target;
		this->listeners.push_back(listener);
		result->SetInt(listener.id);
	}

	void NetworkBinding::RemoveConnectivityListener(
		const ValueList& args,
		SharedValue result)
	{
		ArgUtils::VerifyArgsException("removeConnectivityListener", args, "n");
		int id = args.at(0)->ToInt();

		std::vector<Listener>::iterator it = this->listeners.begin();
		while (it != this->listeners.end())
		{
			if ((*it).id == id)
			{
				this->listeners.erase(it);
				result->SetBool(true);
				return;
			}
			it++;
		}
		result->SetBool(false);
	}

	bool NetworkBinding::HasNetworkStatusListeners()
	{
		return this->listeners.size() > 0;
	}

	void NetworkBinding::NetworkStatusChange(bool online)
	{
		PRINTD("ti.Network: Online status changed ==> " << online);
		this->Set("online", Value::NewBool(online));

		ValueList args = ValueList();
		args.push_back(Value::NewBool(online));
		std::vector<Listener>::iterator it = this->listeners.begin();
		while (it != this->listeners.end())
		{
			SharedBoundMethod callback = (*it++).callback;
			try
			{
				host->InvokeMethodOnMainThread(callback, args, false);
			}
			catch(ValueException& e)
			{
				SharedString ss = e.GetValue()->DisplayString();
				std::cerr << "ti.Network.NetworkStatusChange callback failed: "
				          << *ss << std::endl;
			}
		}
	}

	void NetworkBinding::FireOnlineStatusChange(const ValueList& args, SharedValue result)
	{
		if (args.at(0)->IsBool())
		{
			this->NetworkStatusChange(args.at(0)->ToBool());
		}
	}

	void NetworkBinding::EncodeURIComponent(const ValueList &args, SharedValue result)
	{
		std::string src = args.at(0)->ToString();
	   	std::string sResult = kroll::FileUtils::EncodeURIComponent(src);
		result->SetString(sResult);
	}

	void NetworkBinding::DecodeURIComponent(const ValueList &args, SharedValue result)
	{
		std::string src = args.at(0)->ToString();
		std::string sResult = kroll::FileUtils::DecodeURIComponent(src);
		result->SetString(sResult);
	}

	Host* NetworkBinding::GetHost()
	{
		return this->host;
	}
}
