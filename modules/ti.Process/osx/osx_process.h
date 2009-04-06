/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _OSX_PROCESS_H_
#define _OSX_PROCESS_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <Cocoa/Cocoa.h>
#include "../process_binding.h"
#include "osx_pipe.h"

@interface TiOSXProcess : NSObject
{
	NSTask *task;
	Host *host;
	SharedBoundMethod *onread;
	SharedBoundMethod *onexit;
	ti::OSXPipe *input;
	ti::OSXPipe *output;
	ti::OSXPipe *error;
	SharedBoundObject *shared_input;
	SharedBoundObject *shared_output;
	SharedBoundObject *shared_error;
	BoundObject *bound;
}
-(id)initWithPath:(NSString*)cmd args:(NSArray*)args host:(Host*)host bound:(BoundObject*)bo;
-(NSTask*)task;
-(void)start;
-(void)stop;
-(void)setRead: (SharedBoundMethod*)method;
-(void)setExit: (SharedBoundMethod*)method;
-(void)getOutData: (NSNotification *)aNotification;
-(void)getErrData: (NSNotification *)aNotification;
-(void)terminated: (NSNotification *)aNotification;
@end

namespace ti
{
	class OSXProcess : public StaticBoundObject
	{
	public:
		OSXProcess(ProcessBinding* parent, std::string& cmd, std::vector<std::string>& args);
		virtual ~OSXProcess();
	private:
		TiOSXProcess *process;
	public:

	protected:
		void Bound(const char *name, SharedValue value);
		void Terminate(const ValueList& args, SharedValue result);
	};
}

#endif
