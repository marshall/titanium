/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
//	__argc = argc;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    int result=NSApplicationMain(argc, (const char **) argv);
	[pool release];
	return result;
}
