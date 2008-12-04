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
#import "TiMedia.h"
#import "TiController.h"

@implementation TiMedia

- (id)initWithWebView:(WebView *)wv
{
	self = [super init];
	if (self!=nil)
	{
		webView = wv;
	}
	return self;
}

- (void)dealloc
{
	webView = nil;
	[super dealloc];
}

- (void)beep
{
	NSBeep();
}

- (TiSound*)createSound:(NSString*)url
{
	NSURL *theurl = [TiController formatURL:url];
	return [[TiSound alloc] initWithScope:[webView windowScriptObject] url:theurl];
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(createSound:)) {
		return @"createSound";
	}
	else if (sel == @selector(beep)) {
		return @"beep";
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

