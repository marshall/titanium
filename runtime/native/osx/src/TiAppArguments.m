//
//  TiAppArguments.m
//  Titanium
//
//  Created by Marshall on 11/18/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import "TiAppArguments.h"


@implementation TiAppArguments

@synthesize launchURL;
@synthesize devLaunch;
@synthesize tiAppXml;
@synthesize runtimePath;

-(id) init {
	self = [super init];
	if (self != nil) {
		arguments = [[NSProcessInfo processInfo] arguments];
		pluginPaths = [[NSMutableDictionary alloc] init];
		devLaunch = NO;
		
		for (int i = 1; i < [arguments count]; i++) {
			NSString *arg = [arguments objectAtIndex:i];
			if ([arg isEqualToString:@"--devlaunch"]) {
				devLaunch = YES;
			}
			else if ([arg isEqualToString:@"--xml"] && [arguments count] > i+1) {
				tiAppXml = [NSString stringWithString:[arguments objectAtIndex:i+1]];
				i++;
			}
			else if ([arg isEqualToString:@"--runtime"] && [arguments count] > i+1) {
				runtimePath = [NSString stringWithString:[arguments objectAtIndex:i+1]];
				i++;
			}
			else if ([arg rangeOfString:@"--plugin-"].location != NSNotFound && [arguments count] > i+1) {
				NSString *pluginName = [arg substringFromIndex:[arg rangeOfString:@"--plugin-"].length];
				NSString *pluginPath = [arguments objectAtIndex:i+1];
				
				[pluginPaths setObject:pluginPath forKey:pluginName];
				i++;
			}
			else {
				if (launchURL == nil) {
					launchURL = [NSString stringWithString:arg];
				}
			}
		}
	}
	return self;
}

-(NSEnumerator*) plugins {
	return [pluginPaths keyEnumerator];
}

-(NSString*) pluginPath:(NSString*)pluginName {
	return [pluginPaths objectForKey:pluginName];
}

@end
