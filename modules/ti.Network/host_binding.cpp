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
		 * @tiapi(method=True,returns=string,name=Network.Host.toString) returns a string representation
		 */
		this->SetMethod("toString",&HostBinding::ToString);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.Host.isInvalid) returns true if valid
		 */
		this->SetMethod("isInvalid",&HostBinding::IsInvalid);
		/**
		 * @tiapi(method=True,returns=string,name=Network.Host.getName) returns the hostname
		 */
		this->SetMethod("getName",&HostBinding::GetName);
		/**
		 * @tiapi(method=True,returns=list,name=Network.Host.getAliases) returns a list of aliases
		 */
		this->SetMethod("getAliases",&HostBinding::GetAliases);
		/**
		 * @tiapi(method=True,returns=list,name=Network.Host.getAddresses) returns a list of addresses
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
		SharedBoundList list = new StaticBoundList();
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
		SharedBoundList list = new StaticBoundList();
		std::vector<IPAddress> addresses = this->host.addresses();
		std::vector<IPAddress>::iterator iter = addresses.begin();
		while (iter!=addresses.end())
		{
			IPAddress address = (*iter++);
			SharedBoundObject obj = new IPAddressBinding(address);
			SharedValue addr = Value::NewObject(obj);
			list->Append(addr);
		}
		result->SetList(list);
	}
}