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

#import "TiWindowFactory.h"
#import "TiController.h"

@implementation TiWindowFactory

@synthesize mainWindow;
@synthesize currentWindow;

- (id)initWithWindow:(TiWindow *)win 
{
	self = [super init];
	TRACE(@"TiWindowFactory::initWithWindow: %x",self);
	if (self != nil)  
	{
		// don't retain any of these
		window = win;
		webView = [TiController getWebView:window];
		mainWindow = [[TiController getDocument:win] userWindow];
		currentWindow = [[TiController getDocument:win] userWindow];
	}
	return self;
}

- (void)dealloc
{
	TRACE(@"TiWindowFactory::dealloc: %x",self);
	webView = nil;
	currentWindow = nil;
	mainWindow = nil;
	window = nil;
	[super dealloc];
}

- (TiUserWindow *)getWindow
{
	//TODO: search for windows
	return nil;
}


- (TiUserWindow *)createWindow
{
	TiDocument *doc = [TiController getDocument:window];
	TiUserWindow *win = [[TiUserWindow alloc] initWithWebview:webView];
	[win setParent:doc];
	[doc addChildWindow:win];
	TRACE(@"TiWindowFactory::createWindow - created: %x, parent: %x",win,doc);
	return win;
}

#pragma mark -
#pragma mark WebScripting

+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(createWindow)) {
		return @"createWindow";
	}
	return nil;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (NSString *)webScriptNameForKey:(const char *)name {
	if (strcmp(name, "mainWindow") == 0) {
		return @"mainWindow";
	}
	else if (strcmp(name, "currentWindow") == 0) {
		return @"currentWindow";
	}
	return nil;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return (nil == [self webScriptNameForKey:key]);
}

@end
