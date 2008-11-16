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

#import "TIMenuAction.h"

@implementation TIMenuAction

- (void)dealloc {
	[target dealloc];
	[title dealloc];
	[super dealloc];
}


- (TIMenuAction*)initWithFunc:(WebScriptObject*)f title:(NSString*)t
{
	self = [super init];
	target = f;
	title = t;
	[target retain];
	[title retain];
	return self;
}

- (void)execute
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	[result addObject:target]; // scope
	[result addObject:title]; // argument
	[target callWebScriptMethod:@"call" withArguments:result];
}

@end

