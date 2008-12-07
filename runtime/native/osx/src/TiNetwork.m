/**
 * This file is part of Appcelerator's Titanium project.
 *
 * Copyright 2008 Appcelerator, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */
#import "TiNetwork.h"
#import <WebKit/WebKit.h>
#import <Foundation/Foundation.h>
#import <SystemConfiguration/SCNetworkReachability.h>


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
    assert(target != NULL);
    assert(info   != NULL);
	TiNetwork *network = (TiNetwork*)info;
	BOOL online = (flags & kSCNetworkFlagsReachable) && !(flags & kSCNetworkFlagsTransientConnection);
	[network triggerChange:online];
}

@implementation TiNetwork

@synthesize online;

- (id)initWithWebView:(WebView*)wv
{
	if (self!=nil)
	{
		webView = wv;
		callbacks = [[NSMutableArray alloc] init];

		
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
			SCNetworkReachabilityScheduleWithRunLoop(thisTarget, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
			SCNetworkConnectionFlags flags;
			if ( SCNetworkReachabilityGetFlags(thisTarget, &flags) ) 
			{
				BOOL yn = (flags & kSCNetworkFlagsReachable) && !(flags & kSCNetworkFlagsTransientConnection);
				[self triggerChange:yn];
			}
		}
	}
	return self;
}

- (void)dealloc
{
	[timer invalidate];
	[timer release];
	timer = nil;
	[callbacks removeAllObjects];
	[callbacks release];
	callbacks = nil;
	webView = nil;
	[super dealloc];
}

- (NSString*)description
{
	return @"[TiNetwork native]";
}

- (void)triggerChange:(BOOL)yn
{
	online = yn;
	for (int c=0;c<[callbacks count];c++)
	{
		WebScriptObject* callback = [callbacks objectAtIndex:c];
		NSMutableArray *args = [[NSMutableArray alloc] init];
		[args addObject:[webView windowScriptObject]];
		[args addObject:[NSNumber numberWithBool:online]];
		[callback callWebScriptMethod:@"call" withArguments:args];
		[args release];
	}
}

- (void)onConnectivityChange:(WebScriptObject*)callback
{
	[callbacks addObject:callback];
}


#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(onConnectivityChange:)) {
		return @"onConnectivityChange";
	}
	return nil;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return (nil == [self webScriptNameForKey:key]);
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	if (strcmp(name, "online") == 0) {
		return @"online";
	}
	return nil;
}


@end