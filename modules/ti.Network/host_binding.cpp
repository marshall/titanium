/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "host_binding.h"

namespace ti
{
	HostBinding::HostBinding(IPAddress addr) : name(addr.toString())
	{
		this->Init();
		try
		{
			this->host = DNS::hostByAddress(addr);
		}
		catch (HostNotFoundException&)
		{
			this->invalid = true;
			//TODO: improve this exception so we can properly raise
		}
		catch (NoAddressFoundException&)
		{
			this->invalid = true;
			//TODO: improve this exception so we can properly raise
		}
	}
	HostBinding::HostBinding(std::string name) : name(name)
	{
		this->Init();
		try
		{
			this->host = DNS::hostByName(name.c_str());
		}
		catch (HostNotFoundException&)
		{
			this->invalid = true;
			//TODO: improve this exception so we can properly raise
		}
		catch (NoAddressFoundException&)
		{
			this->invalid = true;
			//TODO: improve this exception so we can properly raise
		}
	}
	HostBinding::~HostBinding()
	{
		KR_DUMP_LOCATION
	}
	void HostBinding::Init() 
	{
		this->invalid = false;
		/**
		 * @tiapi(method=True,name=Network.Host.toString,since=0.2) returns a string representation
		 * @tiresult(for=Network.Host.toString,type=string) string
		 */
		this->SetMethod("toString",&HostBinding::ToString);
		/**
		 * @tiapi(method=True,name=Network.Host.isInvalid,since=0.2) returns true if valid
		 * @tiresult(for=Network.Host.isInvalid,type=boolean) true if invalid
		 */
		this->SetMethod("isInvalid",&HostBinding::IsInvalid);
		/**
		 * @tiapi(method=True,name=Network.Host.getName,since=0.2) returns the hostname
		 * @tiresult(for=Network.Host.getName,type=string) hostname
		 */
		this->SetMethod("getName",&HostBinding::GetName);
		/**
		 * @tiapi(method=True,name=Network.Host.getAliases,since=0.2) returns a list of aliases
		 * @tiresult(for=Network.Host.getAliases,type=list) return a list of aliases
		 */
		this->SetMethod("getAliases",&HostBinding::GetAliases);
		/**
		 * @tiapi(method=True,name=Network.Host.getAddresses,since=0.2) returns a list of addresses
		 * @tiresult(for=Network.Host.getAddresses,type=list) return a list of ip addresses
		 */
		this->SetMethod("getAddresses",&HostBinding::GetAddresses);
	}
	void HostBinding::ToString(const ValueList& args, SharedValue result)
	{
		std::string s("[IPAddress ");
		s+=name;
		s+="]";
		result->SetString(s.c_str());
	}
	void HostBinding::IsInvalid(const ValueList& args, SharedValue result)
	{
		result->SetBool(this->invalid);
	}
	void HostBinding::GetName(const ValueList& args, SharedValue result)
	{
		result->SetString(this->host.name().c_str());
	}
	void HostBinding::GetAliases(const ValueList& args, SharedValue result)
	{
		SharedKList list = new StaticBoundList();
		std::vector<std::string> aliases = this->host.aliases();
		std::vector<std::string>::iterator iter = aliases.begin();
		while (iter!=aliases.end())
		{
			std::string alias = (*iter++);
			list->Append(Value::NewString(alias));
		}
		result->SetList(list);
	}
	void HostBinding::GetAddresses(const ValueList& args, SharedValue result)
	{
		SharedKList list = new StaticBoundList();
		std::vector<IPAddress> addresses = this->host.addresses();
		std::vector<IPAddress>::iterator iter = addresses.begin();
		while (iter!=addresses.end())
		{
			IPAddress address = (*iter++);
			SharedKObject obj = new IPAddressBinding(address);
			SharedValue addr = Value::NewObject(obj);
			list->Append(addr);
		}
		result->SetList(list);
	}
}
