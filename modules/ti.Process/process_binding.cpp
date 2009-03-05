/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include <Poco/Process.h>
#include "process_binding.h"
#include "process.h"

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
		std::vector<std::string> arguments;
		std::string cmd = std::string(args.at(0)->ToString());
		if (args.size()>1)
		{
			if (args.at(1)->IsString())
			{
				std::string arglist = args.at(1)->ToString();
				kroll::FileUtils::Tokenize(arglist,arguments," ");
			}
			else if (args.at(1)->IsList())
			{
				SharedBoundList list = args.at(1)->ToList();
				for (unsigned int c = 0; c < list->Size(); c++)
				{
					SharedValue value = list->At(c);
					arguments.push_back(value->ToString());
				}
			}
		}
		SharedBoundObject p = new Process(this, cmd, arguments);
		processes.push_back(p);
		result->SetObject(p);
	}
	void ProcessBinding::Terminated(Process* p)
	{
		std::vector<SharedBoundObject>::iterator i = processes.begin();
		while(i!=processes.end())
		{
			SharedBoundObject obj = (*i);
			if (obj.get()==p)
			{
				processes.erase(i);
				break;
			}
			i++;
		}
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
		std::string key(args.at(0)->ToString());
		std::string value(args.at(1)->ToString());
		Poco::Environment::set(key,value);
	}
}
