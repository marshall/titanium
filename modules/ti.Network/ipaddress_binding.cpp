/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ipaddress_binding.h"

namespace ti
{
	IPAddressBinding::IPAddressBinding(std::string ip) : invalid(false)
	{
		this->SetMethod("toString",&IPAddressBinding::ToString);
		this->SetMethod("isInvalid",&IPAddressBinding::IsInvalid);
		this->SetMethod("isIPV4",&IPAddressBinding::IsIPV4);
		this->SetMethod("isIPV6",&IPAddressBinding::IsIPV6);
		this->SetMethod("isWildcard",&IPAddressBinding::IsWildcard);
		this->SetMethod("isBroadcast",&IPAddressBinding::IsBroadcast);
		this->SetMethod("isLoopback",&IPAddressBinding::IsLoopback);
		this->SetMethod("isMulticast",&IPAddressBinding::IsMulticast);
		this->SetMethod("isUnicast",&IPAddressBinding::IsUnicast);
		this->SetMethod("isLinkLocal",&IPAddressBinding::IsLinkLocal);
		this->SetMethod("isSiteLocal",&IPAddressBinding::IsSiteLocal);
		this->SetMethod("isWellKnownMC",&IPAddressBinding::IsWellKnownMC);
		this->SetMethod("isNodeLocalMC",&IPAddressBinding::IsNodeLocalMC);
		this->SetMethod("isLinkLocalMC",&IPAddressBinding::IsLinkLocalMC);
		this->SetMethod("isSiteLocalMC",&IPAddressBinding::IsSiteLocalMC);
		this->SetMethod("isOrgLocalMC",&IPAddressBinding::IsOrgLocalMC);
		this->SetMethod("isGlobalMC",&IPAddressBinding::IsGlobalMC);
		
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
	IPAddressBinding::~IPAddressBinding()
	{
		if (this->address)
		{
			delete this->address;
		}
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