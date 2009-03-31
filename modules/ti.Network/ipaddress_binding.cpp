/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ipaddress_binding.h"

namespace ti
{
	IPAddressBinding::IPAddressBinding(std::string ip) : invalid(false)
	{
		this->Init();
		
		try
		{
			this->address = new IPAddress(ip.c_str());
		}
		catch(InvalidAddressException &e)
		{
			this->invalid = true;
			this->address = new IPAddress("0.0.0.0");
		}
	}
	IPAddressBinding::IPAddressBinding(IPAddress ip) : invalid(false) 
	{
		IPAddressBinding(ip.toString());
	}
	IPAddressBinding::~IPAddressBinding()
	{
		KR_DUMP_LOCATION
	}
	void IPAddressBinding::Init()
	{
		/**
		 * @tiapi(method=True,returns=string,name=Network.IPAddress.toString) returns a string representation
		 */
		this->SetMethod("toString",&IPAddressBinding::ToString);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if invalid
		 */
		this->SetMethod("isInvalid",&IPAddressBinding::IsInvalid);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if an IPv4 address
		 */
		this->SetMethod("isIPV4",&IPAddressBinding::IsIPV4);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if an IPv6 address
		 */
		this->SetMethod("isIPV6",&IPAddressBinding::IsIPV6);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a wildcard address
		 */
		this->SetMethod("isWildcard",&IPAddressBinding::IsWildcard);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a broadcast address
		 */
		this->SetMethod("isBroadcast",&IPAddressBinding::IsBroadcast);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a loopback address
		 */
		this->SetMethod("isLoopback",&IPAddressBinding::IsLoopback);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a multicast address
		 */
		this->SetMethod("isMulticast",&IPAddressBinding::IsMulticast);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a unicast address
		 */
		this->SetMethod("isUnicast",&IPAddressBinding::IsUnicast);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a link local address
		 */
		this->SetMethod("isLinkLocal",&IPAddressBinding::IsLinkLocal);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a site local address
		 */
		this->SetMethod("isSiteLocal",&IPAddressBinding::IsSiteLocal);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a well-known multicast address
		 */
		this->SetMethod("isWellKnownMC",&IPAddressBinding::IsWellKnownMC);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a node-local multicast address
		 */
		this->SetMethod("isNodeLocalMC",&IPAddressBinding::IsNodeLocalMC);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a link-local multicast address
		 */
		this->SetMethod("isLinkLocalMC",&IPAddressBinding::IsLinkLocalMC);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a site local multicast address
		 */
		this->SetMethod("isSiteLocalMC",&IPAddressBinding::IsSiteLocalMC);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if an organization local multicast address
		 */
		this->SetMethod("isOrgLocalMC",&IPAddressBinding::IsOrgLocalMC);
		/**
		 * @tiapi(method=True,returns=boolean,name=Network.IPAddress.toString) returns true if a global multicast address
		 */
		this->SetMethod("isGlobalMC",&IPAddressBinding::IsGlobalMC);
	}
	void IPAddressBinding::IsInvalid(const ValueList& args, SharedValue result)
	{
		result->SetBool(this->invalid);
	}
	void IPAddressBinding::ToString(const ValueList& args, SharedValue result)
	{
		result->SetString(this->address->toString().c_str());
	}
	void IPAddressBinding::IsIPV4(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->family() == IPAddress::IPv4);
	}
	void IPAddressBinding::IsIPV6(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->family() == IPAddress::IPv6);
	}
	void IPAddressBinding::IsWildcard(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isWildcard());
	}
	void IPAddressBinding::IsBroadcast(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isBroadcast());
	}
	void IPAddressBinding::IsLoopback(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isLoopback());
	}
	void IPAddressBinding::IsMulticast(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isMulticast());
	}
	void IPAddressBinding::IsUnicast(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isUnicast());
	}
	void IPAddressBinding::IsLinkLocal(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isLinkLocal());
	}
	void IPAddressBinding::IsSiteLocal(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isSiteLocal());
	}
	void IPAddressBinding::IsWellKnownMC(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isWellKnownMC());
	}
	void IPAddressBinding::IsNodeLocalMC(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isNodeLocalMC());
	}
	void IPAddressBinding::IsLinkLocalMC(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isLinkLocalMC());
	}
	void IPAddressBinding::IsSiteLocalMC(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isSiteLocalMC());
	}
	void IPAddressBinding::IsOrgLocalMC(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isOrgLocalMC());
	}
	void IPAddressBinding::IsGlobalMC(const ValueList& args, SharedValue result)
	{
		result->SetBool(!this->invalid && this->address->isGlobalMC());
	}
}