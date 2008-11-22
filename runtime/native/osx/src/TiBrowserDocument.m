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

#import "TiBrowserDocument.h"
#import "TiBrowserWindowController.h"
#import "TiAppDelegate.h"
#import "TiWindowOptions.h"

#import <WebKit/WebKit.h>

@interface TiBrowserDocument (Friend)
- (void)setIsFirst:(BOOL)yn;
@end

@implementation TiBrowserDocument

- (id)init 
{
	self = [super init];
	if (self) {
	}
	return self;
}

- (void)dealloc 
{
	browserWindowController = nil;
	//TODO: need to release options?
	windowOptions = nil;
	[super dealloc];
}

- (TiWindowOptions*)getOptions
{
	return windowOptions;
}

- (void)setOptions:(TiWindowOptions*)opts
{
	windowOptions = opts;
	NSUInteger mask = 0;
	if ([windowOptions isResizable])
	{
		mask |= NSResizableWindowMask;
	}
	if ([windowOptions isCloseable])
	{
		mask |= NSClosableWindowMask;
	}
	if ([windowOptions isMinimizable])
	{
		mask |= NSMiniaturizableWindowMask;
	}
	if ([windowOptions isMaximizable])
	{
		// this is handled special in the TiBrowserWindow by
		// hiding the actual control on the title bar
	}
	if (![windowOptions isChrome])
	{
		mask |= NSBorderlessWindowMask;
	}
	else
	{
		mask |= NSTitledWindowMask;
	}		
	
	//we set it on the delegate since he's a singleton and these options are "sticky"
	[[TiAppDelegate instance] setActiveWindowOption:opts];
}


#pragma mark -
#pragma mark NSDocument

- (void)makeWindowControllers 
{
	browserWindowController = [[TiBrowserWindowController alloc] initWithWindowNibName:@"BrowserWindow"];
	[self addWindowController:browserWindowController];
	[browserWindowController release];
}


- (BOOL)isDocumentEdited 
{
	return NO;
}


- (NSString *)displayName 
{
	TiWindowOptions *opts = [[TiAppDelegate instance] getActiveWindowOption];
	NSString *ns = [opts getTitle];
	[opts release];
	return [ns autorelease];
}


- (void)close 
{
	if ([[self getOptions] isFullscreen])
	{
		//FIXME: fullscreen
		//[[TiAppDelegate instance] setIsFullScreen:NO];
	}
	[super close];
}


#pragma mark -
#pragma mark Public

- (void)loadRequest:(NSURLRequest *)request 
{
	[[[self webView] mainFrame] loadRequest:request];
}


- (TiBrowserWindowController *)browserWindowController 
{
	return browserWindowController;
}


#pragma mark -
#pragma mark Accessors

- (BOOL)isFirst 
{
	return isFirst;
}


- (void)setIsFirst:(BOOL)yn 
{
	isFirst = yn;
}


- (WebView *)webView 
{
	return [browserWindowController webView];
}

@end
