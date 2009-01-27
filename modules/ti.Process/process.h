/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include "pipe.h"

namespace ti
{
	class Process : public StaticBoundObject
	{
	public:
		Process(std::string& command, std::vector<std::string>& args);
	protected:
		virtual ~Process();
	private:
		Poco::ProcessHandle *process;
		bool running;
		Pipe *in;
		Pipe *out;
		Pipe *err;
		Poco::Pipe *errp;
		Poco::Pipe *outp;
		Poco::Pipe *inp;
		
		void Terminate(const ValueList& args, SharedValue result);
		void Terminate();
	};
}

#endif
