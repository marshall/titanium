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

#import "TiMenuFactory.h"

@implementation TiMenuFactory

- (id)initWithWebView:(WebView *)wv 
{
	self = [super init];
	if (self != nil) 
	{
		webView = wv; // assign only. don't retain. prevents retain loop memory leak
	}
	return self;
}

- (void)dealloc
{
	webView = nil;
	[super dealloc];
}

- (TiUserMenu *)createUserMenu:(NSString*)label
{
	//TODO: review with marshall - probably need a getUserMenu
	NSMenu *appMenu = [[[NSApp mainMenu] itemWithTitle:label] submenu];
	if (appMenu == nil)
	{
		//FIXME
	}
	else
	{
		return [[TiUserMenu alloc]initWithMenu:appMenu];
	}
	return nil;
}


- (TiSystemMenu *)createSystemMenu:(NSString*)iconURL caption:(NSString*)caption callback:(WebScriptObject*)callback;
{
	return [[TiSystemMenu alloc] initWithURL:iconURL caption:caption callback:callback];
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(createSystemMenu:caption:callback:)) {
		return @"createSystemMenu";
	}
	else if (sel == @selector(createUserMenu:)) {
		return @"createUserMenu";
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
