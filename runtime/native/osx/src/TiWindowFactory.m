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

@implementation TiWindowFactory

- (id)initWithWebView:(WebView *)wv 
{
	self = [super init];
	if (self != nil) {
		webView = wv; // assign only. don't retain. prevents retain loop memory leak
	}
	return self;
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

#pragma mark -
#pragma mark WebScripting

+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(createWindow)) {
		return @"createWindow";
	}
	else if (sel == @selector(createWindowOptions)) {
		return @"createWindowOptions";
	}
	return nil;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (NSString *)webScriptNameForKey:(const char *)name {
	return nil;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return (nil == [self webScriptNameForKey:key]);
}

@end
