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
	#include "osx/osx_process.h"
#endif


#ifdef OS_WIN32
#include <windows.h>
#endif


namespace ti
{
	ProcessBinding::ProcessBinding(Host *h, SharedKObject global) : host(h),global(global)
	{
		/**
		 * @tiapi(property=True,type=integer,name=Process.pid,since=0.3) returns the process id from the application
		 */
#ifdef OS_OSX
		NSProcessInfo *p = [NSProcessInfo processInfo];
		this->Set("pid",Value::NewInt([p processIdentifier]));
#else
		this->Set("pid",Value::NewInt((int)Poco::Process::id()));
#endif
		//TODO: support times api
		//static void times(long& userTime, long& kernelTime);

		/**
		 * @tiapi(method=True,returns=list,name=Process.getEnv,since=0.2) returns a list of environment variables
		 * @tiarg(for=Process.getEnv,name=name,type=string) name of the environment property
		 * @tiresult(for=Process.getEnv,type=string) value for name
		 */
		this->SetMethod("getEnv",&ProcessBinding::GetEnv);
		/**
		 * @tiapi(method=True,returns=void,name=Process.setEnv,since=0.2) sets an environment variable
		 * @tiarg(for=Process.setEnv,name=name,type=string) name of the environment property
		 * @tiarg(for=Process.setEnv,name=value,type=string) value of the environment property
		 */
		this->SetMethod("setEnv",&ProcessBinding::SetEnv);
		/**
		 * @tiapi(method=True,name=Process.hasEnv,since=0.2) returns true if a value exists in the environment
		 * @tiarg(for=Process.hasEnv,name=name,type=string) name of the environment property
		 * @tiresult(for=Process.hasEnv,type=boolean) returns true if found
		 */
		this->SetMethod("hasEnv",&ProcessBinding::HasEnv);
		/**
		 * @tiapi(method=True,name=Process.launch,since=0.2) launch an external application and returns a Process object
		 * @tiarg(for=Process.launch,name=command,type=string) command line
		 * @tiresult(for=Process.launch,type=object) returns Process object
		 */
		this->SetMethod("launch",&ProcessBinding::Launch);
		/**
		 * @tiapi(method=True,returns=void,name=Process.restart,since=0.3) restart the application
		 */
		this->SetMethod("restart",&ProcessBinding::Restart);
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
				SharedKList list = args.at(1)->ToList();
				for (unsigned int c = 0; c < list->Size(); c++)
				{
					SharedValue value = list->At(c);
					arguments.push_back(value->ToString());
				}
			}
		}
#ifdef OS_OSX
		SharedKObject p = new OSXProcess(this, cmd, arguments);
#else
		SharedKObject p = new Process(this, cmd, arguments);
#endif
		processes.push_back(p);
		result->SetObject(p);
	}
	void ProcessBinding::Terminated(Process* p)
	{
		std::vector<SharedKObject>::iterator i = processes.begin();
		while(i!=processes.end())
		{
			SharedKObject obj = (*i);
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
	void ProcessBinding::Restart(const ValueList& args, SharedValue result)
	{
#ifdef OS_OSX
		NSProcessInfo *p = [NSProcessInfo processInfo];
		NSString *path = [[NSBundle mainBundle] bundlePath];
		NSString *killArg1AndOpenArg2Script = [NSString stringWithFormat:@"kill -9 %d\n open \"%@\"",[p processIdentifier],path];
		NSArray *shArgs = [NSArray arrayWithObjects:@"-c", // -c tells sh to execute the next argument, passing it the remaining arguments.
			killArg1AndOpenArg2Script,nil];
		NSTask *restartTask = [NSTask launchedTaskWithLaunchPath:@"/bin/sh" arguments:shArgs];
		[restartTask waitUntilExit]; //wait for killArg1AndOpenArg2Script to finish
#elif OS_WIN32
		std::string cmdline = host->GetCommandLineArg(0);
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		CreateProcessA(NULL,
			(LPSTR)cmdline.c_str(),
			NULL, /*lpProcessAttributes*/
			NULL, /*lpThreadAttributes*/
			FALSE, /*bInheritHandles*/
			NORMAL_PRIORITY_CLASS,
			NULL,
			NULL,
			&si,
			&pi);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
#elif OS_LINUX
		std::string cmdline = host->GetCommandLineArg(0);
		size_t idx;
		while ((idx = cmdline.find_first_of('\"')) != std::string::npos)
		{
			cmdline.replace(idx, 1, "\\\"");
		}
		std::string script = "\"" + cmdline + "\" &";
		system(script.c_str());
#endif
		host->Exit(999);
	}
}
