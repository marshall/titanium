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

#import "TiSystemMenu.h"

@implementation TiSystemMenu

- (void)dealloc {
	[[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
	[statusItem dealloc];
	if (menu)
	{
		[menu dealloc];
	}
	[itemImage dealloc];
	[super dealloc];
}

- (TiSystemMenu*)initWithURL:(NSString*)u f:(WebScriptObject*)f
{
	self = [super init];
	
	target = f;
	[target retain];

	NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
	statusItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];

	NSURL *url = [NSURL URLWithString:u];
	itemImage = [[NSImage alloc]initWithContentsOfURL:url];
	[itemImage retain];

	[statusItem setImage: itemImage];
	[statusItem setHighlightMode:YES];
	[statusItem setEnabled:true];
	[statusItem setAction:@selector(menuClicked:)];
	[statusItem setTarget:self];
	[statusItem retain];

	showing = YES;
	return self;
}


- (void)menuClicked:(id)sender{
	if (target)
	{
		[self execute];
	}
}

- (NSString *)description {
	return @"[TiSystemMenu native]";
}

- (void)itemClicked:(id)sender{
	[sender execute];
}

- (void)addItem:(NSString*)name f:(WebScriptObject*)f{
	
	if (!menu)
	{
		menu = [NSMenu new];
		[statusItem setMenu:menu];
		[menu release];
	}
	
	TiMenuAction *action = [[TiMenuAction alloc] initWithFunc:f title:name];
	[items addObject:action];
	
	[action setAction:@selector(itemClicked:)];
	[action setTitle:name];
	[action setEnabled:true];
	[action setTarget:self];
	[menu addItem:action];
}

- (void)addSeparator
{
	if (menu)
	{
		NSMenuItem *sep = [NSMenuItem separatorItem];
		[menu addItem:sep];
	}
}

- (void)execute
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	[result addObject:target]; // scope
	[result addObject:self]; // argument
	[target callWebScriptMethod:@"call" withArguments:result];
}

- (void)hide
{
	if (showing)
	{
		[[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
		[statusItem dealloc];
//		[menu dealloc];
		showing = NO;
	}
}

- (void)show
{
	if (!showing)
	{
		NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
		statusItem = [statusBar statusItemWithLength:NSVariableStatusItemLength];
		
		[statusItem setImage: itemImage];
		[statusItem setHighlightMode:YES];
		[statusItem setEnabled:true];
		[statusItem setAction:@selector(menuClicked:)];
		[statusItem setTarget:self];
		[statusItem retain];
		
//
//FIXME: need to re-work this
//
//		menu = [NSMenu alloc];
//		[statusItem setMenu:menu];
//		
//		//TODO: you can't just re-attach the statusItem to the menu, doesn't work
//		NSEnumerator *enumerator = [items objectEnumerator];
//		TiMenuAction *action;
//		while ((action = [enumerator nextObject]) != nil)
//		{
//			[menu addItem:action];
//		}
//		[menu release];
		
		showing = YES;
	}
}

+ (NSString *) webScriptNameForSelector:(SEL)sel{
	if (sel == @selector(addItem:f:))
	{
		return @"addItem";
	}
	else if (sel == @selector(addSeparator))
	{
		return @"addSeparator";
	}
	else if (sel == @selector(hide))
	{
		return @"hide";
	}
	else if (sel == @selector(show))
	{
		return @"show";
	}
	return nil;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel{
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name{
	return YES;
}


@end

