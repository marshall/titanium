/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "osx_process.h"
#include <Poco/Path.h>

@implementation TiOSXProcess

-(id)initWithPath:(NSString*)cmd args:(NSArray*)args host:(Host*)h bound:(KObject*)bo
{
	self = [super init];
	if (self)
	{
		host = h;
		onread = nil;
		onexit = nil;
		bound = bo;
		
		task = [[NSTask alloc] init];
		[task setLaunchPath:cmd];
		[task setArguments:args];
		[task setCurrentDirectoryPath:[[NSBundle mainBundle] resourcePath]];

		NSProcessInfo *pi = [NSProcessInfo processInfo];
		[task setEnvironment:[pi environment]];
		
		[task setStandardOutput: [NSPipe pipe]];
		[task setStandardError: [NSPipe pipe]];
		[task setStandardInput: [NSPipe pipe]];
		
		// Here we register as an observer of the NSFileHandleReadCompletionNotification, which lets
	    // us know when there is data waiting for us to grab it in the task's file handle (the pipe
	    // to which we connected stdout and stderr above). methods will be called when there
	    // is data waiting.  The reason we need to do this is because if the file handle gets
	    // filled up, the task will block waiting to send data and we'll never get anywhere.
	    // So we have to keep reading data from the file handle as we go.
		[[NSNotificationCenter defaultCenter] addObserver:self 
	        selector:@selector(getOutData:) 
	        name: NSFileHandleReadCompletionNotification 
	        object: [[task standardOutput] fileHandleForReading]];
	
		[[NSNotificationCenter defaultCenter] addObserver:self 
	        selector:@selector(getErrData:) 
	        name: NSFileHandleReadCompletionNotification 
	        object: [[task standardError] fileHandleForReading]];

		[[NSNotificationCenter defaultCenter] addObserver:self 
	        selector:@selector(terminated:) 
	        name: NSTaskDidTerminateNotification 
	        object: task];
	
		input = new ti::OSXPipe([[task standardInput] fileHandleForWriting]);
		output = new ti::OSXPipe([[task standardOutput] fileHandleForReading]);
		error = new ti::OSXPipe([[task standardError] fileHandleForReading]);
		
		shared_input = new SharedKObject(input);
		shared_output = new SharedKObject(output);
		shared_error = new SharedKObject(error);
		
		bound->Set("in", Value::NewObject(*shared_input));
		bound->Set("err", Value::NewObject(*shared_error));
		bound->Set("out", Value::NewObject(*shared_output));
	
		// schedule reads
		[[[task standardOutput] fileHandleForReading] readInBackgroundAndNotify];
		[[[task standardError] fileHandleForReading] readInBackgroundAndNotify];
		
		bound->Set("running",Value::NewBool(true));
	}
	return self;
}
-(void)dealloc
{
	if ([task isRunning])
	{
		[self stop];
	}
	delete shared_input;
	shared_input=NULL;
	delete shared_output;
	shared_output=NULL;
	delete shared_error;
	shared_error=NULL;
	if (onread)
	{
		delete onread;
		onexit = NULL;
	}
	if (onexit)
	{
		delete onexit;
		onexit = NULL;
	}
	[task release];
	[super dealloc];
}
-(NSTask*)task
{
	return task;
}
-(void)start
{
	[task launch];
}
-(void)stop
{
	if ([task isRunning])
	{
		[task terminate];
	}
}
-(void)setRead: (SharedKMethod*)method
{
	if (onread)
	{
		delete onread;
	}
	onread = method;
}
-(void)setExit: (SharedKMethod*)method
{
	if (onexit)
	{
		delete onexit;
	}
	onexit = method;
}
-(void)terminated: (NSNotification *)aNotification
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSFileHandleReadCompletionNotification object: [[task standardOutput] fileHandleForReading]];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSFileHandleReadCompletionNotification object: [[task standardError] fileHandleForReading]];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:NSTaskDidTerminateNotification object: task];

	bound->Set("running",Value::NewBool(false));
	bound->Set("exitCode",Value::NewInt([task terminationStatus]));

	if (onexit && !onexit->isNull())
	{
		ValueList args;
		args.push_back(Value::NewInt([task terminationStatus]));
		try
		{
			host->InvokeMethodOnMainThread(*onexit,args,true);
		}
		catch(...)
		{
		}
	}
	
	// close the streams after our onexit in case they want to read
	// one last time from the stream in the callback
	
	input->Close();
	output->Close();
	error->Close();
}
-(void)getOutData: (NSNotification *)aNotification
{
	NSData *data = [[aNotification userInfo] objectForKey:NSFileHandleNotificationDataItem];
    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if ([data length])
    {
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
		NSString *str = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];

		if (onread && !onread->isNull())
		{
			ValueList args;
			args.push_back(Value::NewString([str UTF8String]));
			args.push_back(Value::NewBool(false));
			host->InvokeMethodOnMainThread(*onread,args,false);
		}
		else
		{
			output->OnData(str);
		}
    } 

    // we need to schedule the file handle go read more data in the background again.
    [[aNotification object] readInBackgroundAndNotify];
}
- (void) getErrData: (NSNotification *)aNotification
{
	NSData *data = [[aNotification userInfo] objectForKey:NSFileHandleNotificationDataItem];
    // If the length of the data is zero, then the task is basically over - there is nothing
    // more to get from the handle so we may as well shut down.
    if ([data length])
    {
        // Send the data on to the controller; we can't just use +stringWithUTF8String: here
        // because -[data bytes] is not necessarily a properly terminated string.
        // -initWithData:encoding: on the other hand checks -[data length]
		NSString *str = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];

		if (onread && !onread->isNull())
		{
			ValueList args;
			args.push_back(Value::NewString([str UTF8String]));
			args.push_back(Value::NewBool(true));
			host->InvokeMethodOnMainThread(*onread,args,false);
		}
		else
		{
			error->OnData(str);
		}
    } 

    // we need to schedule the file handle go read more data in the background again.
    [[aNotification object] readInBackgroundAndNotify];
}
@end

namespace ti
{
	OSXProcess::OSXProcess(ProcessBinding* parent, std::string& cmd, std::vector<std::string>& args)
	{
		NSMutableArray *arguments = [[NSMutableArray alloc] init];
		if (args.size()>0)
		{
			std::vector<std::string>::iterator i = args.begin();
			while(i!=args.end())
			{
				std::string arg = (*i++);
				[arguments addObject:[NSString stringWithCString:arg.c_str()]];
			}
		}

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
		process = [[TiOSXProcess alloc] initWithPath:[NSString stringWithCString:cmd.c_str()] args:arguments host:parent->GetHost() bound:this];
		[arguments release];
		
		// start the process
		[process start];
		
		this->Set("command",Value::NewString(cmd));
		
		SET_BOOL_PROP("running",false);
		SET_INT_PROP("pid",[[process task] processIdentifier]);
		
		SET_NULL_PROP("exitCode")
		SET_NULL_PROP("onread")
		SET_NULL_PROP("onexit")

		this->SetMethod("terminate",&OSXProcess::Terminate);
	}
	OSXProcess::~OSXProcess()
	{
		[process stop];
		[process release];
	}
	void OSXProcess::Terminate(const ValueList& args, SharedValue result)
	{
		[process stop];
		[process setRead:NULL];
		[process setExit:NULL];
	}
	void OSXProcess::Bound(const char *name, SharedValue value)
	{
		std::string fn(name);
		if (fn == "onread")
		{
			[process setRead:new SharedKMethod(value->ToMethod())];
		}
		else if (fn == "onexit")
		{
			[process setExit:new SharedKMethod(value->ToMethod())];
		}
	}
}
