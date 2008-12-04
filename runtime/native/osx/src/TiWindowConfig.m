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
#import "TiWindowConfig.h"
#import "AppProtocol.h"

@implementation TiWindowConfig


- (id) init
{
	self = [super init];

	// setup defaults
	x=-1; // indicate not set
	y=-1; // indicate not set
	width=100;
	height=100;
	maxHeight=9000;
	maxWidth=9000;
	minHeight=0;
	minWidth=0;
	title=@"Titanium Application";
	url=@"index.html";
	chrome=NO;
	transparency=1.0;
	visible=YES;
	minimizable=YES;
	maximizable=YES;
	resizable=YES;
	fullscreen=NO;
	closeable=YES;
	scrollbars=YES;
	
	TRACE(@"TiWindowConfig: alloc\n");

	return self;
}

- (void)dealloc
{
	TRACE(@"TiWindowConfig: dealloc\n");
	window = nil;
	[title release];
	title = nil;
	[url release];
	url = nil;
	[super dealloc];
}

- (NSString *)description 
{
	return @"[TiWindowConfig native]";
}


- (NSString*) getID
{
	return windowID;
}

- (void) setID:(NSString*)i
{
	[windowID release];
	windowID = [i copy];
}


- (CGFloat) getX
{
	return x;
}

- (void) setX:(CGFloat)newx
{
	x = newx;
}

- (CGFloat) getY
{
	return y;
}

- (void) setY:(CGFloat)newy
{
	y = newy;
}

- (CGFloat) getWidth
{
	return width;
}

- (void) setWidth:(CGFloat)w
{
	width = w;
}

- (CGFloat) getHeight
{
	return height;
}

- (void) setHeight:(CGFloat)h
{
	height = h;
}

- (CGFloat) getMinWidth
{
	return minWidth;
}

- (void) setMinWidth:(CGFloat)w
{
	minWidth = w;
}

- (CGFloat) getMinHeight
{
	return minHeight;
}

- (void) setMinHeight:(CGFloat)h
{
	minHeight = h;
}

- (CGFloat) getMaxWidth
{
	return maxWidth;
}

- (void) setMaxWidth:(CGFloat)w
{
	maxWidth = w;
}

- (CGFloat) getMaxHeight
{
	return maxHeight;
}

- (void) setMaxHeight:(CGFloat)h
{
	maxHeight = h;
}

- (BOOL) isChrome
{
	return chrome;
}

- (void) setChrome:(BOOL)yn
{
	chrome = yn;
}

- (CGFloat) getTransparency
{
	return transparency;
}

- (void) setTransparency:(CGFloat)f;
{
	transparency = f;
}

- (BOOL) isVisible
{
	return visible;
}

- (void) setVisible:(BOOL)yn
{
	visible = yn;
}

- (BOOL) isMinimizable
{
	return minimizable;
}

- (void) setMinimizable:(BOOL)yn
{
	minimizable = yn;
}

- (BOOL) isMaximizable
{
	return maximizable;
}

- (void) setMaximizable:(BOOL)yn
{
	maximizable = yn;
}

- (BOOL) isResizable
{
	return resizable;
}

- (void) setResizable:(BOOL)yn
{
	resizable = yn;
}

- (BOOL) isFullscreen
{
	return fullscreen;
}

- (void) setFullscreen:(BOOL)yn
{
	fullscreen = yn;
}

- (BOOL) isCloseable
{
	return closeable;
}

- (void) setCloseable:(BOOL)yn
{
	closeable = yn;
}

- (BOOL) isScrollbars
{
	return scrollbars;
}

- (void) setScrollbars:(BOOL)yn
{
	scrollbars = yn;
}


- (NSString*) getTitle
{
	return title;
}

- (void) setTitle:(NSString*)t
{
	[title release];
	title = [t copy];
	
	if (window!=nil)
	{
		[window setTitle:title];
	}
}

- (NSString*) getURL
{
	return url;
}

- (void) setURL:(NSString*)u
{
	[url release];
	url = [u copy];
}

- (NSUInteger) toWindowMask
{
	NSUInteger mask = 0;
	if (chrome)
	{
		mask = NSBorderlessWindowMask;
	}
	else
	{
		mask |= NSTitledWindowMask;
		if (resizable)
		{
			mask |= NSResizableWindowMask;
		}
		if (closeable)
		{
			mask |= NSClosableWindowMask;
		}
		if (maximizable)
		{
			mask |= NSMiniaturizableWindowMask;
		}
		if (maximizable)
		{
			// handled in the window code
		}
	}
	return mask;
}

- (BOOL)urlMatches:(NSString*)testURL
{
	// first see if we match the full URL
	if ([url isEqualToString:testURL])
	{
		return YES;
	}
	NSString *a = [AppProtocol getPath:[NSURL URLWithString:url]];
	NSString *b = [AppProtocol getPath:[NSURL URLWithString:testURL]];
	TRACE(@"a=%@, b=%@, matches=%d",a,b,[a isEqualToString:b]);
	return [a isEqualToString:b];
}

- (void)assign:(TiWindow*)w
{
	window = w;
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(getX)) {
		return @"getX";
	} else if (sel == @selector(setX:)) {
		return @"setX";
	} else if (sel == @selector(getY)) {
		return @"getY";
	} else if (sel == @selector(setY:)) {
		return @"setY";
	} else if (sel == @selector(getID)) {
		return @"getID";
	} else if (sel == @selector(getWidth)) {
		return @"getWidth";
	} else if (sel == @selector(setWidth:)) {
		return @"setWidth";
	} else if (sel == @selector(getHeight)) {
		return @"getHeight";
	} else if (sel == @selector(setHeight:)) {
		return @"setHeight";
	} else if (sel == @selector(setFullscreen:)) {
		return @"setFullscreen";
	} else if (sel == @selector(isFullscreen)) {
		return @"isFullscreen";
	} else if (sel == @selector(isResizable)) {
		return @"isResizable";
	} else if (sel == @selector(setResizable:)) {
		return @"setResizable";
	} else if (sel == @selector(isMaximizable)) {
		return @"isMaximizable";
	} else if (sel == @selector(setMaximizable:)) {
		return @"setMaximizable";
	} else if (sel == @selector(isMinimizable)) {
		return @"isMinimizable";
	} else if (sel == @selector(setMinimizable:)) {
		return @"setMinimizable";
	} else if (sel == @selector(isChrome)) {
		return @"isChrome";
	} else if (sel == @selector(setChrome:)) {
		return @"setChrome";
	} else if (sel == @selector(isScrollbars)) {
		return @"isScrollbars";
	} else if (sel == @selector(setScrollbars:)) {
		return @"setScrollbars";
	} else if (sel == @selector(setTransparency:)) {
		return @"setTransparency";
	} else if (sel == @selector(getTransparency)) {
		return @"getTransparency";
	} else if (sel == @selector(isCloseable)) {
		return @"isCloseable";
	} else if (sel == @selector(setCloseable:)) {
		return @"setCloseable";
	} else if (sel == @selector(isVisible)) {
		return @"isVisible";
	} else if (sel == @selector(setVisible:)) {
		return @"setVisible";
	} else if (sel == @selector(setTitle:)) {
		return @"setTitle";
	} else if (sel == @selector(getTitle)) {
		return @"getTitle";
	} else if (sel == @selector(setURL:)) {
		return @"setURL";
	} else if (sel == @selector(getURL)) {
		return @"getURL";
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