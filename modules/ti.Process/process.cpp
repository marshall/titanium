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
	Process::Process(ProcessBinding* parent, std::string& cmd, std::vector<std::string>& args) : 
		thread1(0),thread2(0),thread3(0),thread1Running(false),
		thread2Running(false),thread3Running(false),running(false),
		pid(-1),errp(0),outp(0),inp(0)
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
		this->Set("pid",Value::NewNull());
		/**
		 * @tiapi(property=True,type=boolean,name=Process.Process.running,version=0.2) returns true if the command is running
		 */
		this->Set("running",Value::NewBool(false)); 
		 
		this->err = new Pipe(new Poco::PipeInputStream(*errp));
		this->shared_error = new SharedKObject(this->err);
		
		/**
		 * @tiapi(property=True,type=object,name=Process.Process.err,version=0.2) returns the error stream as a Pipe object
		 */
		this->Set("err",Value::NewObject(*shared_error));
		
		this->out = new Pipe(new Poco::PipeInputStream(*outp));
		this->shared_output = new SharedKObject(this->out);
		/**
		 * @tiapi(property=True,type=object,name=Process.Process.out,version=0.2) returns the output stream as a Pipe object
		 */
		this->Set("out",Value::NewObject(*shared_output));
		
		this->in = new Pipe(new Poco::PipeOutputStream(*inp));
		this->shared_input = new SharedKObject(this->in);
		/**
		 * @tiapi(property=True,type=object,name=Process.Process.in,version=0.2) returns the input stream as a Pipe object
		 */
		this->Set("in",Value::NewObject(*shared_input));
		
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
		this->thread1->start(&Process::WaitExit,(void*)this);
	}
	Process::~Process()
	{
		Terminate();
		if (this->thread1)
		{
			if (this->thread1Running)
			{
				try
				{
					this->thread1->join();
				}
				catch(...)
				{
				}
			}
			delete this->thread1;
			this->thread1 = NULL;
		}
		if (this->thread2)
		{
			if (this->thread2Running)
			{
				try
				{
					this->thread2->join();
				}
				catch (...)
				{
				}
			}
			delete this->thread2;
			this->thread2 = NULL;
		}
		if (this->thread3)
		{
			if (this->thread3Running)
			{
				try
				{
					this->thread3->join();
				}
				catch(...)
				{
				}
			}
			delete this->thread3;
			this->thread3 = NULL;
		}
		delete shared_output;
		delete shared_input;
		delete shared_error;
		SET_NULL_PROP("onexit")
		SET_NULL_PROP("onread")
	}
	void Process::WaitExit(void *data)
	{
		Process *process = static_cast<Process*>(data);
		process->Set("running",Value::NewBool(true));
		int exitCode = -1;
		try
		{
			Poco::ProcessHandle ph = Poco::Process::launch(process->command,process->arguments,process->inp,process->outp,process->errp);
			process->pid = (int)ph.id();
			process->Set("pid",Value::NewInt(process->pid));
			exitCode = ph.wait();
			process->Set("exitCode",Value::NewInt(exitCode));
		}
		catch (Poco::SystemException &se)
		{
			std::cerr << "System Exception starting: " << process->command << ",message: " << se.what() << std::endl;
		}
		process->running = false;
		process->pid = -1;
		process->Set("running",Value::NewBool(false));
		SharedValue sv = process->Get("onexit");
		if (sv->IsMethod())
		{
			ValueList args;
			args.push_back(Value::NewInt(exitCode));
			SharedKMethod callback = sv->ToMethod();
			process->parent->GetHost()->InvokeMethodOnMainThread(callback,args,false);
		}
		process->parent->Terminated(process);
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
				SharedKMethod callback = sv->ToMethod();
				process->parent->GetHost()->InvokeMethodOnMainThread(callback,args,false);
			}
		}
		process->thread2Running = false;
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
				SharedKMethod callback = sv->ToMethod();
				process->parent->GetHost()->InvokeMethodOnMainThread(callback,args,false);
			}
		}
		process->thread3Running = false;
	}
	void Process::Terminate(const ValueList& args, SharedValue result)
	{
		Terminate();
	}
	void Process::Terminate()
	{
		if (running && pid!=-1)
		{
			running = false;
#ifdef OS_WIN32			
			// win32 needs a kill to terminate process
			Poco::Process::kill(this->pid);
#else
			// this sends a more graceful SIGINT instead of SIGKILL
			// which is important for programs that manage child processes
			// and handle their own signals
			Poco::Process::requestTermination(this->pid);
#endif			
			this->Set("running",Value::NewBool(false));
			this->parent->Terminated(this);
		}
	}
	void Process::Bound(const char *name, SharedValue value)
	{
		if (std::string(name) == "onread")
		{
			if (this->thread2 == NULL)
			{
				this->thread2 = new Poco::Thread();
				this->thread2->start(&Process::ReadOut,(void*)this);
			}
			if (this->thread3 == NULL)
			{
				this->thread3 = new Poco::Thread();
				this->thread3->start(&Process::ReadErr,(void*)this);
			}
		}
	}
}

