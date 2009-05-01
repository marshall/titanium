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
#include "proxy/proxy.h"

using kroll::DataUtils;

namespace ti
{
	std::vector<SharedKObject> NetworkBinding::bindings;
	NetworkBinding::NetworkBinding(Host* host) :
		 host(host),
		 global(host->GetGlobalObject()),
		 proxy(NULL),
		 next_listener_id(0)
	{
		SharedValue online = Value::NewBool(true);
		/**
		 * @tiapi(property=True,name=Network.online,since=0.2) returns true if the machine is connected to the Internet
		 * @tiresult(for=Network.online,type=boolean) returns true if online
		 */
		this->Set("online", online);

		// methods that are available on Titanium.Network
		/**
		 * @tiapi(method=True,name=Network.createTCPSocket,since=0.2) creates a TCP client socket
		 * @tiarg(for=Network.createTCPSocket,name=host,type=string) hostname
		 * @tiarg(for=Network.createTCPSocket,name=port,type=integer) port
		 * @tiresult(for=Network.createTCPSocket,type=object) return a Network.TCPSocket object
		 */
		this->SetMethod("createTCPSocket",&NetworkBinding::CreateTCPSocket);
		/**
		 * @tiapi(method=True,name=Network.createIRCClient,since=0.2) creates an IRC client socket
		 * @tiresult(for=Network.createIRCClient,type=object) return a Network.IRCClient object
		 */
		this->SetMethod("createIRCClient",&NetworkBinding::CreateIRCClient);
		/**
		 * @tiapi(method=True,name=Network.createIPAddress,since=0.2) creates IP Address object
		 * @tiarg(for=Network.createIPAddress,name=address,type=string) address
		 * @tiresult(for=Network.createIPAddress,type=object) return a Network.IPAddress object
		 */
		this->SetMethod("createIPAddress",&NetworkBinding::CreateIPAddress);
		/**
		 * @tiapi(method=True,name=Network.createHTTPClient,since=0.3) creates an HTTP client
		 * @tiresult(for=Network.createHTTPClient,type=object) return a Network.HTTPClient object
		 */
		this->SetMethod("createHTTPClient",&NetworkBinding::CreateHTTPClient);
		/**
		 * @tiapi(method=True,name=Network.getHostByName,since=0.2) convert a host by name into a Host object
		 * @tiarg(for=Network.getHostByName,name=name,type=string) hostname
		 * @tiresult(for=Network.getHostByName,type=object) return a Network.Host object
		 */
		this->SetMethod("getHostByName",&NetworkBinding::GetHostByName);
		/**
		 * @tiapi(method=True,name=Network.getHostByAddress,since=0.2) convert a host by ip into a Host object
		 * @tiarg(for=Network.getHostByAddress,name=address,type=string) address
		 * @tiresult(for=Network.getHostByAddress,type=object) return a Network.Host object
		 */
		this->SetMethod("getHostByAddress",&NetworkBinding::GetHostByAddress);
		/**
		 * @tiapi(method=True,name=Network.encodeURIComponent,since=0.3) encode a URI component
		 * @tiarg(for=Network.encodeURIComponent,name=value,type=string) value to encode
		 * @tiresult(for=Network.encodeURIComponent,type=string) encoded value
		 */
		this->SetMethod("encodeURIComponent",&NetworkBinding::EncodeURIComponent);
		/**
		 * @tiapi(method=True,name=Network.decodeURIComponent,since=0.3) decode a URI component
		 * @tiarg(for=Network.decodeURIComponent,name=value,type=string) value to decode
		 * @tiresult(for=Network.decodeURIComponent,type=string) decoded value
		 */
		this->SetMethod("decodeURIComponent",&NetworkBinding::DecodeURIComponent);

		/**
		 * @tiapi(method=True,name=Network.addConnectivityListener,since=0.2) add a connectivity change listener. returns an id to be used when removing.
		 * @tiarg(for=Network.addConnectivityListener,type=method,name=listener) callback method
		 * @tiresult(for=Network.addConnectivityListener,type=integer) returns an registration id to use when removing
		 */
		this->SetMethod("addConnectivityListener",&NetworkBinding::AddConnectivityListener);
		/**
		 * @tiapi(method=True,name=Network.removeConnectivityListener,since=0.2) remove a connectivity change listener
		 * @tiarg(for=Network.removeConnectivityListener,type=integer,name=id) registration id
		 */
		this->SetMethod("removeConnectivityListener",&NetworkBinding::RemoveConnectivityListener);

		/**
		 * @tiapi(method=True,name=Network.SetProxy,since=0.2) sets the proxy parameters to the system.
		 * @tiarg(for=Network.SetProxy,type=integer,name=id) bool indicating success/failute
		 */
		this->SetMethod("setProxy",&NetworkBinding::SetProxy);

		/**
		 * @tiapi(method=True,name=Network.GetProxy,since=0.2)
		 * @tiarg(for=Network.GetProxy,type=integer,name=id) returns the Proxy object
		 */
		this->SetMethod("getProxy",&NetworkBinding::GetProxy);

		// NOTE: this is only used internally and shouldn't be published
		this->SetMethod("FireOnlineStatusChange",&NetworkBinding::FireOnlineStatusChange);

#if defined(OS_LINUX)
		this->net_status = new DBusNetworkStatus(this);
		this->net_status->Start();
#elif defined(OS_OSX)
		SharedKMethod delegate = this->Get("FireOnlineStatusChange")->ToMethod();
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
			SharedKObject obj = args.at(0)->ToObject();
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
					SharedKMethod m = bo->ToMethod();
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
		std::vector<SharedKObject>::iterator i = bindings.begin();
		while(i!=bindings.end())
		{
			SharedKObject b = (*i);
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
		SharedKObject obj = http->GetSelf()->ToObject();
		// we hold the reference to this until we're done with it
		// which happense when the binding impl calls remove
		this->bindings.push_back(obj);
		result->SetObject(obj);
	}
	void NetworkBinding::AddConnectivityListener(const ValueList& args, SharedValue result)
	{
		args.VerifyException("addConnectivityListener", "m");
		SharedKMethod target = args.at(0)->ToMethod();

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
		args.VerifyException("removeConnectivityListener", "n");
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
			SharedKMethod callback = (*it++).callback;
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
	   	std::string sResult = DataUtils::EncodeURIComponent(src);
		result->SetString(sResult);
	}

	void NetworkBinding::DecodeURIComponent(const ValueList &args, SharedValue result)
	{
		std::string src = args.at(0)->ToString();
		std::string sResult = DataUtils::DecodeURIComponent(src);
		result->SetString(sResult);
	}
	
	void NetworkBinding::SetProxy(const ValueList& args, SharedValue result)
	{
		std::string hostname = args.at(0)->ToString();
		std::string port = args.at(1)->ToString();
		std::string username = args.at(2)->ToString();
		std::string password = args.at(3)->ToString();
		if(proxy)
		{
			delete proxy;
			proxy = NULL;
		}

		proxy = new ti::Proxy(hostname, port, username,password);
		result->SetBool(true);
	}

	void NetworkBinding::GetProxy(const ValueList& args, SharedValue result)
	{
		if(proxy)
		{
			result->SetObject(this->proxy);
		}
	}


	Host* NetworkBinding::GetHost()
	{
		return this->host;
	}
}
