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
#import "Ti.h"
#import "TiController.h"
#import "TiObject.h"

@implementation TiObject

@synthesize App;
@synthesize Dock;
@synthesize Menu;
@synthesize Window;
@synthesize Media;
@synthesize Network;

- (id)initWithWindow:(TiWindow*)win 
{
	self = [super init];
	if (self != nil) 
	{
		window = win;
		webView = [TiController getWebView:window];
		
		[window retain];
		[webView retain];

		App = [[TiApp alloc] initWithWindow:window];
		Dock = [[TiSystemDock alloc] initWithWebView:webView];
		Menu = [[TiMenuFactory alloc] initWithWebView:webView];
		Window = [[TiWindowFactory alloc] initWithWindow:window];
		Media = [[TiMedia alloc] initWithWebView:webView];
		Network = [[TiNetwork alloc] initWithWebView:webView];
	}
	return self;
}


- (void)dealloc 
{
	[window release];
	[webView release];
	
	window = nil;
	webView = nil;
	[App release];
	[Dock release];
	[Menu release];
	[Window release];
	[Media release];
	[Network release];
	[super dealloc];
}


- (NSString *)description 
{
	return @"[TiObject native]";
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	return nil;
}


+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return (nil == [self webScriptNameForKey:key]);
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	if (strcmp(name, "App") == 0) {
		return @"App";
	}
	else if (strcmp(name, "Dock") == 0) {
		return @"Dock";
	}
	else if (strcmp(name, "Menu") == 0) {
		return @"Menu";
	}
	else if (strcmp(name, "Window") == 0) {
		return @"Window";
	}
	else if (strcmp(name, "Media") == 0) {
		return @"Media";
	}
	else if (strcmp(name, "Network") == 0) {
		return @"Network";
	}
	return nil;
}


@end
