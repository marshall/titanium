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

#import "TiSystemDock.h"

@implementation TiSystemDock

- (id)initWithWebView:(WebView *)wv 
{
	self = [super init];
	if (self != nil) {
		webView = wv; // assign only. don't retain. prevents retain loop memory leak
		[self setupDock];
	}
	return self;
}

- (void)dealloc 
{
	webView = nil;
	[super dealloc];
}

- (NSString *)description {
	return @"[TiSystemDock native]";
}

- (void)setupDock
{
	//more details: http://th30z.netsons.org/2008/10/cocoa-notification-badge/
}

- (void)setBadge:(NSString*)s
{
	NSDockTile *dockicon = [NSApp dockTile];
	if (nil != s)
	{
		[dockicon setShowsApplicationBadge:YES];
		[dockicon setBadgeLabel:s];
	}
	else
	{
		[dockicon setShowsApplicationBadge:NO];
		[dockicon setBadgeLabel:@""];
	}
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(setBadge:)) {
		return @"setBadge";
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
