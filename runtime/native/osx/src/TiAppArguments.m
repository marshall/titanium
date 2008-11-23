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

#import "TiAppArguments.h"


@implementation TiAppArguments

@synthesize launchURL;
@synthesize devLaunch;
@synthesize tiAppXml;
@synthesize runtimePath;
@synthesize debug;

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
			else if ([arg isEqualToString:@"--debug"]) {
				debug = YES;
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
