/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <vector>
#include "process.h"
#include "pipe.h"

namespace ti
{
	Process::Process(ProcessBinding* parent, std::string& cmd, std::vector<std::string>& args) : process(0), running(true)
	{
		this->parent = parent;
		this->errp = new Poco::Pipe();
		this->outp = new Poco::Pipe();
		this->inp = new Poco::Pipe();

#ifdef DEBUG
		std::cout << "Running process: " << cmd << std::endl;
#endif

		try
		{
			this->process = new Poco::ProcessHandle(Poco::Process::launch(cmd,args,inp,outp,errp));
		}
		catch (std::exception &e)	
		{
			throw ValueException::FromString(e.what());
		}
		
		this->Set("command",Value::NewString(cmd));
		this->Set("pid",Value::NewInt((int)process->id()));
		this->Set("running",Value::NewBool(true)); 
		 
		this->err = new Pipe(new Poco::PipeInputStream(*errp));
		SharedBoundObject errb = this->err;
		this->Set("err",Value::NewObject(errb));
		
		this->out = new Pipe(new Poco::PipeInputStream(*outp));
		SharedBoundObject outb = this->out;
		this->Set("out",Value::NewObject(outb));
		
		this->in = new Pipe(new Poco::PipeOutputStream(*inp));
		SharedBoundObject inb = this->in;
		this->Set("in",Value::NewObject(inb));
		
		this->SetMethod("terminate",&Process::Terminate);
	}
	Process::~Process()
	{
		Terminate();
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
			this->parent->Terminated(this);
		}
	}
}
