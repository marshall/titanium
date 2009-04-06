/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <vector>
#include "process.h"
#include "pipe.h"
#include <Poco/Path.h>

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
#ifdef OS_OSX
			// this is a simple check to see if the path passed
			// in is an actual .app folder and not the full path
			// to the binary. in this case, we'll instead invoke
			// the fullpath to the binary
			size_t found = cmd.rfind(".app");
			if (found!=std::string::npos)
			{
				Poco::Path p(cmd);
				std::string fn = p.getFileName();
				found = fn.find(".app");
				fn = fn.substr(0,found);
				fn = kroll::FileUtils::Join(cmd.c_str(),"Contents","MacOS",fn.c_str(),NULL);
				if (FileUtils::IsFile(fn))
				{
					cmd = fn;
				}
			}
#endif
			this->arguments = args;
			this->command = cmd;
		}
		catch (std::exception &e)	
		{
			throw ValueException::FromString(e.what());
		}
		
		/**
		 * @tiapi(property=True,type=string,name=Process.Process.command,version=0.2) returns the command for the process
		 */
		this->Set("command",Value::NewString(cmd));
		/**
		 * @tiapi(property=True,type=integer,name=Process.Process.pid,version=0.2) returns the process id for the process
		 */
		this->Set("pid",Value::NewInt((int)process->id()));
		/**
		 * @tiapi(property=True,type=boolean,name=Process.Process.running,version=0.2) returns true if the command is running
		 */
		this->Set("running",Value::NewBool(false)); 
		 
		this->err = new Pipe(new Poco::PipeInputStream(*errp));
		SharedBoundObject errb = this->err;
		/**
		 * @tiapi(property=True,type=object,name=Process.Process.err,version=0.2) returns the error stream as a Pipe object
		 */
		this->Set("err",Value::NewObject(errb));
		
		this->out = new Pipe(new Poco::PipeInputStream(*outp));
		SharedBoundObject outb = this->out;
		/**
		 * @tiapi(property=True,type=object,name=Process.Process.out,version=0.2) returns the output stream as a Pipe object
		 */
		this->Set("out",Value::NewObject(outb));
		
		this->in = new Pipe(new Poco::PipeOutputStream(*inp));
		SharedBoundObject inb = this->in;
		/**
		 * @tiapi(property=True,type=object,name=Process.Process.in,version=0.2) returns the input stream as a Pipe object
		 */
		this->Set("in",Value::NewObject(inb));
		
		/**
		 * @tiapi(method=True,name=Process.Process.terminate,version=0.2) terminate the process
		 */
		this->SetMethod("terminate",&Process::Terminate);
		
		/**
		 * @tiapi(property=True,type=integer,name=Process.Process.exitCode,version=0.4) returns the exit code or null if not yet exited
		 */
		this->Set("exitCode",Value::Null);

		/**
		 * @tiapi(property=True,type=method,name=Process.Process.onread,since=0.4) set the function handler to call when sys out is read
		 */
		SET_NULL_PROP("onread")

		/**
		 * @tiapi(property=True,type=method,name=Process.Process.onexit,since=0.4) set the function handler to call when the process exits
		 */
		SET_NULL_PROP("onexit")

		// setup threads which can read output and also
		// monitor the exit
		this->thread1 = new Poco::Thread();
		this->thread2 = new Poco::Thread();
		this->thread3 = new Poco::Thread();
		this->thread1->start(&Process::WaitExit,(void*)this);
		this->startCondition.wait(this->startMutex);
		this->thread2->start(&Process::ReadOut,(void*)this);
		this->thread3->start(&Process::ReadErr,(void*)this);
	}
	Process::~Process()
	{
		Terminate();
		if (this->thread1->isRunning())
		{
			this->thread1->join();
		}
		if (this->thread2->isRunning())
		{
			this->thread2->join();
		}
		if (this->thread2->isRunning())
		{
			this->thread2->join();
		}
		delete this->thread1;
		this->thread1 = NULL;
		delete this->thread2;
		this->thread2 = NULL;
		delete this->thread3;
		this->thread3 = NULL;
	}
	void Process::WaitExit(void *data)
	{
		Process *process = static_cast<Process*>(data);
		process->Set("running",Value::NewBool(true));
		process->startCondition.signal();
		process->process = new Poco::ProcessHandle(Poco::Process::launch(process->command,process->arguments,process->inp,process->outp,process->errp));
		int exitCode = process->process->wait();
		process->running = false;
		process->Set("running",Value::NewBool(false));
		process->Set("exitCode",Value::NewInt(exitCode));
		SharedValue sv = process->Get("onexit");
		if (sv->IsMethod())
		{
			ValueList args;
			args.push_back(Value::NewInt(exitCode));
			SharedBoundMethod callback = sv->ToMethod();
			process->parent->GetHost()->InvokeMethodOnMainThread(callback,args,false);
		}
	}
	void Process::ReadOut(void *data)
	{
		Process *process = static_cast<Process*>(data);
		ValueList a;
		
		while (process->running)
		{
			SharedValue result;
			process->out->Read(a,result);
			SharedValue sv = process->Get("onread");
			if (sv->IsMethod())
			{
				ValueList args;
				args.push_back(result);
				args.push_back(Value::NewBool(false));
				SharedBoundMethod callback = sv->ToMethod();
				process->parent->GetHost()->InvokeMethodOnMainThread(callback,args,false);
			}
		}
	}
	void Process::ReadErr(void *data)
	{
		Process *process = static_cast<Process*>(data);
		ValueList a;
		
		while (process->running)
		{
			SharedValue result;
			process->err->Read(a,result);
			SharedValue sv = process->Get("onread");
			if (sv->IsMethod())
			{
				ValueList args;
				args.push_back(result);
				args.push_back(Value::NewBool(true));
				SharedBoundMethod callback = sv->ToMethod();
				process->parent->GetHost()->InvokeMethodOnMainThread(callback,args,false);
			}
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
			this->parent->Terminated(this);
		}
	}
}

