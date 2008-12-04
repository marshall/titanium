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
#import "TiAppMenu.h"
#import "TiMenuAction.h"


@implementation TiAppMenu

-(id)initWithMenu:(NSMenu*)m
{
	self = [super init];
	if (self) 
	{
		menu = m;
		[menu retain];
	}
	return self;
}

- (void)dealloc
{
	[items removeAllObjects];
	[items release];
	items = nil;
	[menu release];
	menu = nil;
	[super dealloc];
}

- (NSString *)description {
	return @"[TiAppMenu native]";
}

- (void)setTitle:(NSString*)title
{
	[menu setTitle:title];
}

- (NSString*)getTitle
{
	return [menu title];
}

- (void)itemClicked:(id)sender
{
	[sender execute];
}

- (TiMenuAction*)addItem:(NSString*)name f:(WebScriptObject*)f
{
	WebScriptObject *fn = [f isEqual:[WebUndefined undefined]] ? nil : f;
	TiMenuAction *action = [[TiMenuAction alloc] initWithFunc:fn title:name menu:menu];
	[items addObject:action];
	[action setAction:@selector(itemClicked:)];
	[action setTitle:name];
	[action setEnabled:true];
	[action setTarget:self];
	[menu addItem:action];
	return action;
}

- (NSMenuItem*)addSeparator
{
	NSMenuItem *sep = [NSMenuItem separatorItem];
	[menu addItem:sep];
	return sep;
}

- (void)removeSeparator:(NSMenuItem*)item
{
	[menu removeItem:item];
}


#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(setTitle:)) {
		return @"setTitle";
	}
	else if (sel == @selector(getTitle)) {
		return @"getTitle";
	}
	else if (sel == @selector(addItem:f:)) {
		return @"addItem";
	}
	else if (sel == @selector(addSeparator)) {
		return @"addSeparator";
	}
	else if (sel == @selector(removeSeparator:)) {
		return @"removeSeparator";
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