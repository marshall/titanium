/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include "platform_binding.h"

#ifdef OS_OSX
#include <Foundation/Foundation.h>
#endif

namespace ti
{
	PlatformBinding::PlatformBinding(SharedBoundObject global) : global(global)
	{
		this->Set("name",Value::NewString(Poco::Environment::osName().c_str()));
		this->Set("version",Value::NewString(Poco::Environment::osVersion().c_str()));
		this->Set("architecture",Value::NewString(Poco::Environment::osArchitecture().c_str()));
		this->Set("address",Value::NewString(Poco::Environment::nodeName().c_str()));
		try
		{
			this->Set("id",Value::NewString(Poco::Environment::nodeId().c_str()));
		}
		catch(...)
		{
			this->Set("id",Value::NewString(""));
		}
		this->SetMethod("getEnv",&PlatformBinding::GetEnv);
		this->SetMethod("setEnv",&PlatformBinding::SetEnv);
		this->SetMethod("hasEnv",&PlatformBinding::HasEnv);

#ifdef OS_OSX
		NSProcessInfo *p = [NSProcessInfo processInfo];
		this->Set("processorCount",Value::NewInt([p processorCount]));
#else
		//TODO - Linux / Win32
		this->Set("processorCount",Value::NewInt(1));
#endif
	}
	PlatformBinding::~PlatformBinding()
	{
	}
	void PlatformBinding::GetEnv(const ValueList& args, SharedValue result)
	{
		std::string key(args.at(0)->ToString());
		try
		{
			std::string value = Poco::Environment::get(key);
			result->SetString(value.c_str());
		}
		catch(...)
		{
			// if they specified a default as 2nd parameter, return it
			// otherwise, return null
			if (args.size()==2)
			{
				result->SetString(args.at(1)->ToString());
			}
			else
			{
				result->SetNull();
			}
		}
	}
	void PlatformBinding::HasEnv(const ValueList& args, SharedValue result)
	{
		std::string key(args.at(0)->ToString());
		result->SetBool(Poco::Environment::has(key));
	}
	void PlatformBinding::SetEnv(const ValueList& args, SharedValue result)
	{
		std::string key(args.at(1)->ToString());
		std::string value(args.at(2)->ToString());
		Poco::Environment::set(key,value);
	}
}
