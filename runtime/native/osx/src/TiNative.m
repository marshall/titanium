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

#import "TiNative.h"
#import "TiAppDelegate.h"
#import <WebKit/WebKit.h>

@implementation TiNative

- (id)initWithWebView:(WebView *)wv {
	self = [super init];
	if (self != nil) {
		webView = wv; // assign only. don't retain. prevents retain loop memory leak
		app = [[TiApp alloc] initWithWebView:webView];
		dock = [[TiSystemDock alloc] initWithWebView:webView];
	}
	return self;
}


- (void)dealloc {
	webView = nil;
	[app dealloc];
	[dock dealloc];
	[super dealloc];
}


- (NSString *)description {
	return @"[TiNative native]";
}

- (TiSystemMenu *)createSystemMenu:(NSString*)url f:(WebScriptObject*)f
{
	TiSystemMenu *menu = [TiSystemMenu alloc];
	[menu initWithURL:url f:f];
	return menu;
}

- (TiUserWindow *)createWindow
{
	TiUserWindow *win = [TiUserWindow new];
	return win;
}

- (TiWindowOptions*)createWindowOptions
{
	TiWindowOptions *opts = [TiWindowOptions new];
	return opts;
}

- (TiApp*) getApp
{
	return app;
}

- (TiSystemDock*) getDock
{
	return dock;
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(createSystemMenu:f:)) {
		return @"createSystemMenu";
	} else if (sel == @selector(createWindow)) {
		return @"createWindow";
	} else if (sel == @selector(createWindowOptions)) {
		return @"createWindowOptions";
	} else if (sel == @selector(getApp)) {
		return @"getApp";
	} else if (sel == @selector(getDock)) {
		return @"getDock";
	}
	return nil;
}


+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return YES;
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	return nil;
}


@end
