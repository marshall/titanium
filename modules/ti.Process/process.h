/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/Thread.h>
#include <Poco/Mutex.h>
#include <Poco/Condition.h>
#include "pipe.h"
#include "process_binding.h"

namespace ti
{
	class Process : public StaticBoundObject
	{
	public:
		Process(ProcessBinding* parent, std::string& command, std::vector<std::string>& args);
	protected:
		virtual ~Process();
	private:
		ProcessBinding *parent;
		Poco::ProcessHandle *process;
		Poco::Mutex startMutex;
		Poco::Condition startCondition;
		Poco::Thread *thread1;
		Poco::Thread *thread2;
		Poco::Thread *thread3;
		bool running;
		std::vector<std::string> arguments;
		std::string command;
		Pipe *in;
		Pipe *out;
		Pipe *err;
		Poco::Pipe *errp;
		Poco::Pipe *outp;
		Poco::Pipe *inp;
		
		void Terminate(const ValueList& args, SharedValue result);
		void Terminate();
		static void WaitExit(void*);
		static void ReadOut(void*);
		static void ReadErr(void*);
	};
}

#endif
