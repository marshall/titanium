/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#import "network_status.h"
 

static void TiReachabilityCallback(SCNetworkReachabilityRef  target,
                   SCNetworkConnectionFlags  flags,
                   void *info
)
// This routine is a System Configuration framework callback that 
// indicates that the reachability of a given host has changed.  
// It's call from the runloop.  target is the host whose reachability 
// has changed, the flags indicate the new reachability status, and 
// info is the context parameter that we passed in when we registered 
// the callback.  In this case, info is a pointer to the host name.
// 
// Our response to this notification is simply to print a line 
// recording the transition.
{
  BOOL online = (flags & kSCNetworkFlagsReachable) && !(flags & kSCNetworkFlagsTransientConnection);
  NetworkReachability *network = (NetworkReachability*)info;
  [network triggerChange:online];
}

@implementation NetworkReachability

-(id)initWithDelegate:(SharedBoundMethod)m
{
	self = [super init];
	if (self!=nil)
	{	
		delegate = new SharedBoundMethod(m);
		online = YES;
		[NSThread detachNewThreadSelector:@selector(start) toTarget:self withObject:nil];
	}
	return self;
}
- (void)dealloc
{
	KR_DUMP_LOCATION
	delete delegate;
	[super dealloc];
}
- (void)start
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool init] alloc];
	SCNetworkReachabilityRef thisTarget;
	SCNetworkReachabilityContext thisContext;
    thisContext.version = 0;
    thisContext.info = (void *)self;
    thisContext.retain = NULL;
    thisContext.release = NULL;
    thisContext.copyDescription = NULL;

    // Create the target with the most reachable internet site in the world
    thisTarget = SCNetworkReachabilityCreateWithName(NULL, "google.com");

    // Set our callback and install on the runloop.
    if (thisTarget) 
    {
    	SCNetworkReachabilitySetCallback(thisTarget, TiReachabilityCallback, &thisContext);
    	SCNetworkReachabilityScheduleWithRunLoop(thisTarget, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
      	SCNetworkConnectionFlags flags;
      	if ( SCNetworkReachabilityGetFlags(thisTarget, &flags) ) 
      	{
        	BOOL yn = (flags & kSCNetworkFlagsReachable) && !(flags & kSCNetworkFlagsTransientConnection);
        	[self triggerChange:yn];
      	}
    }
	[pool release];
}
- (void)triggerChange:(BOOL)yn
{
  if (yn!=online)
  {
    online = yn;
	ValueList args;
	args.push_back(Value::NewBool(online));
	(*delegate)->Call(args);
  }
}
@end
