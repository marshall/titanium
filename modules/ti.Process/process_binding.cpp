/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include <Poco/Process.h>
#include "process_binding.h"

#ifdef OS_OSX
	#include <Foundation/Foundation.h>
#endif

namespace ti
{
	ProcessBinding::ProcessBinding(SharedBoundObject global) : global(global)
	{
#ifdef OS_OSX
		NSProcessInfo *p = [NSProcessInfo processInfo];
		this->Set("pid",Value::NewInt([p processIdentifier]));
#else
		this->Set("pid",Value::NewInt((int)Poco::Process::id()));
#endif
		//TODO: support times api
		//static void times(long& userTime, long& kernelTime);

		this->SetMethod("getEnv",&ProcessBinding::GetEnv);
		this->SetMethod("setEnv",&ProcessBinding::SetEnv);
		this->SetMethod("hasEnv",&ProcessBinding::HasEnv);
		this->SetMethod("launch",&ProcessBinding::Launch);
	}
	ProcessBinding::~ProcessBinding()
	{
	}
	void ProcessBinding::Launch(const ValueList& args, SharedValue result)
	{
		//TODO: use Poco::Process
	}
	void ProcessBinding::GetEnv(const ValueList& args, SharedValue result)
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
	void ProcessBinding::HasEnv(const ValueList& args, SharedValue result)
	{
		std::string key(args.at(0)->ToString());
		result->SetBool(Poco::Environment::has(key));
	}
	void ProcessBinding::SetEnv(const ValueList& args, SharedValue result)
	{
		std::string key(args.at(1)->ToString());
		std::string value(args.at(2)->ToString());
		Poco::Environment::set(key,value);
	}
}
