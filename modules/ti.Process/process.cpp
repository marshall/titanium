/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */	
#include <vector>
#include "process.h"
#include "pipe.h"

namespace ti
{
	Process::Process(std::string& cmd, std::vector<std::string>& args) : process(0), running(true)
	{
		try
		{
			this->process = new Poco::ProcessHandle(Poco::Process::launch(cmd,args,&outp,&inp,&errp));
		}
		catch (std::exception &e)
		{
			throw Value::NewString(e.what());
		}
		
		this->Set("command",Value::NewString(cmd));
		this->Set("pid",Value::NewInt((int)process->id()));
		this->Set("running",Value::NewBool(true)); 
 
		this->err = new Pipe(errp);
		this->Set("err",Value::NewObject(SharedBoundObject(err)));
		
		this->out = new Pipe(outp);
		this->Set("out",Value::NewObject(SharedBoundObject(out)));
		
		this->in = new Pipe(inp);
		this->Set("in",Value::NewObject(SharedBoundObject(in)));
		
		this->SetMethod("terminate",&Process::Terminate);
	}
	Process::~Process()
	{
		Terminate();
		
		if (this->err)
		{
			delete this->err;
			this->err = NULL;
		}
		if (this->out)
		{
			delete this->out;
			this->out = NULL;
		}
		if (this->in)
		{
			delete this->in;
			this->in = NULL;
		}
		if (this->process)
		{
			delete this->process;
			this->process = NULL;
		}
	}
	void Process::Terminate(const ValueList& args, SharedValue result)
	{
		Terminate();
	}
	void Process::Terminate()
	{
		if (running)
		{
			Poco::Process::kill(this->process->id());
			running = false;
			this->Set("running",Value::NewBool(false));
		}
	}
}