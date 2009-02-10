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
	Process::Process(SharedPtr<ProcessBinding> parent, std::string& cmd, std::vector<std::string>& args) : process(0), running(true)
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
		if (this->errp)
		{
			delete this->errp;
			this->errp = NULL;
		}
		if (this->outp)
		{
			delete this->outp;
			this->outp = NULL;
		}
		if (this->inp)
		{
			delete this->inp;
			this->inp = NULL;
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
			SharedBoundObject p = this;
			this->parent->Terminated(p);
		}
	}
}
