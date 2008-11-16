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

#import "TIBrowserDocument.h"
#import "TIBrowserWindowController.h"
#import "TIAppDelegate.h"
#import <WebKit/WebKit.h>

@interface TIBrowserDocument (Friend)
- (void)setIsFirst:(BOOL)yn;
@end

@implementation TIBrowserDocument

- (id)init {
	self = [super init];
	if (self) {
	}
	return self;
}


- (void)dealloc {
	browserWindowController = nil;
	[super dealloc];
}


#pragma mark -
#pragma mark NSDocument

- (void)makeWindowControllers {
	browserWindowController = [[TIBrowserWindowController alloc] initWithWindowNibName:@"BrowserWindow"];
	[self addWindowController:browserWindowController];
	[browserWindowController release];
}


- (BOOL)isDocumentEdited {
	return NO;
}


- (NSString *)displayName {
	return [[TIAppDelegate instance] windowTitle];
}


- (void)close {
	if ([[TIAppDelegate instance] isFullScreen]) {
		[[TIAppDelegate instance] setIsFullScreen:NO];
	}
	[super close];
}


#pragma mark -
#pragma mark Public

- (void)loadRequest:(NSURLRequest *)request {
	[[[self webView] mainFrame] loadRequest:request];
}


- (TIBrowserWindowController *)browserWindowController {
	return browserWindowController;
}


#pragma mark -
#pragma mark Accessors

- (BOOL)isFirst {
	return isFirst;
}


- (void)setIsFirst:(BOOL)yn {
	isFirst = yn;
}


- (WebView *)webView {
	return [browserWindowController webView];
}

@end
