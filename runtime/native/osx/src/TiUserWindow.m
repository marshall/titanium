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
#import "TiUserWindow.h"
#import "TIBrowserDocument.h"
#import "TIAppDelegate.h"


@implementation TiUserWindow

- (TiUserWindow*) init
{
	self = [super init];
	NSURL *url = [[NSURL alloc] initWithString:@"about:blank"];
	NSURLRequest *req = [[NSURLRequest alloc] initWithURL:url];
	doc = [[TIAppDelegate instance] newDocumentWithRequest:req display:NO];
	
	NSWindow *window = [[doc browserWindowController] window];
	NSRect newFrame = NSZeroRect;

	NSScreen *screen = [window screen];
	if (!screen) {
		screen = [NSScreen mainScreen];
	}
	NSRect screenFrame = [screen frame];
	bool fullscreen = NO;
	
	if (fullscreen) {
		newFrame = screenFrame;
	} else {
		NSRect winFrame = [window frame];
		CGFloat y = winFrame.origin.y;
		CGFloat w =  winFrame.size.width;
		CGFloat h =  winFrame.size.height;
		// Cocoa screen coords are from bottom left. but web coords are from top left. must convert origin.x
		CGFloat x = winFrame.origin.x;
//		if (xObj) {
//			x = [xObj floatValue];
//			x = screenFrame.size.height - x - h;
//		}
		newFrame = NSMakeRect(x, y, w, h);
	}
	[window setFrame:newFrame display:YES];
	[doc retain];
	return self;
}

- (void)dealloc {
	[doc dealloc];
	[super dealloc];
}

- (void)close {
	[doc close];
}

- (void)show {
	[doc showWindows];
}

+ (NSString *) webScriptNameForSelector:(SEL)sel{
	if (sel == @selector(show))
	{
		return @"show";
	}
	else if (sel == @selector(close))
	{
		return @"close";
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
