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
#import <WebKit/WebKit.h>
#import "TiMenuAction.h"
#import "TiController.h"

@implementation TiMenuAction

- (void)dealloc 
{
	[target release];
	target=nil;
	[title release];
	title=nil;
	[menu release];
	menu=nil;
	[super dealloc];
}


- (TiMenuAction*)initWithFunc:(WebScriptObject*)f title:(NSString*)t menu:(NSMenu*)m
{
	self = [super init];
	if (self != nil)
	{
		target = f;
		title = t;
		menu = m;
		if (target) 
		{
			[target retain];
		}
		[title retain];
		[menu retain];
	}
	return self;
}

- (void)execute
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	if (target)
	{
		[result addObject:target]; // scope
		[result addObject:title]; // argument
		[target callWebScriptMethod:@"call" withArguments:result];
	}
	[result release];
}

- (void)setTitle:(NSString*)t
{
	[title release];
	title = t;
	[title retain];
	[super setTitle:t];
}

-(NSString*)getTitle
{
	return title;
}

-(void)setEnabled:(BOOL)yn
{
	[super setEnabled:yn];
}

-(BOOL)isEnabled
{
	return [super isEnabled];
}

-(void)setVisible:(BOOL)yn
{
	[super setHidden:!yn];
}

-(BOOL)isVisible
{
	return ![super isHidden];
}

-(void)setIcon:(NSString*)u 
{
	NSURL *url = [TiController formatURL:u];
	NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
	[super setImage:image];
}

-(void)setCallback:(WebScriptObject*)f
{
	[target release];
	target = f;
	[target retain];
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(setEnabled:)) {
		return @"setEnabled";
	}
	else if (sel == @selector(isEnabled)) {
		return @"isEnabled";
	}
	else if (sel == @selector(isVisible)) {
		return @"isVisible";
	}
	else if (sel == @selector(setVisible:)) {
		return @"setVisible";
	}
	else if (sel == @selector(setIcon:)) {
		return @"setIcon";
	}
	else if (sel == @selector(setCallback:)) {
		return @"setAction";
	}
	else if (sel == @selector(setTitle:)) {
		return @"setTitle";
	}
	else if (sel == @selector(title)) {
		return @"getTitle";
	}
	return nil;
}


+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return (nil == [self webScriptNameForKey:key]);
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	return nil;
}

@end

