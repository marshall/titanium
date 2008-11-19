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
#import "TiWindowOptions.h"
#import <WebKit/WebKit.h>

@implementation TiWindowOptions

- (id) init
{
	self = [super init];

	// setup defaults
	x=0;
	y=0;
	width=100;
	height=100;
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

	return self;
}

- (void)dealloc
{
	title = nil;
	url = nil;
	[super dealloc];
}

- (NSString *)description 
{
	return @"[TiWindowOptions native]";
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

- (int) getWidth
{
	return width;
}

- (void) setWidth:(int)w
{
	width = w;
}

- (int) getHeight
{
	return height;
}

- (void) setHeight:(int)h
{
	height = h;
}

- (bool) isChrome
{
	return chrome;
}

- (void) setChrome:(bool)yn
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

- (bool) isVisible
{
	return visible;
}

- (void) setVisible:(bool)yn
{
	visible = yn;
}

- (bool) isMinimizable
{
	return minimizable;
}

- (void) setMinimizable:(bool)yn
{
	minimizable = yn;
}

- (bool) isMaximizable
{
	return maximizable;
}

- (void) setMaximizable:(bool)yn
{
	maximizable = yn;
}

- (bool) isResizable
{
	return resizable;
}

- (void) setResizable:(bool)yn
{
	resizable = yn;
}

- (bool) isFullscreen
{
	return fullscreen;
}

- (void) setFullscreen:(bool)yn
{
	fullscreen = yn;
}

- (bool) isCloseable
{
	return closeable;
}

- (void) setCloseable:(bool)yn
{
	closeable = yn;
}

- (bool) isScrollbars
{
	return scrollbars;
}

- (void) setScrollbars:(bool)yn
{
	scrollbars = yn;
}


- (NSString*) getTitle
{
	return [[title copy] autorelease];
}

- (void) setTitle:(NSString*)t
{
	[title autorelease];
	title = [t copy];
}

- (NSString*) getURL
{
	return [[url copy] autorelease];
}

- (void) setURL:(NSString*)u
{
	[url autorelease];
	url = [u copy];
}

- (NSUInteger) toWindowMask
{
	NSUInteger mask = 0;
	if ([self isResizable])
	{
		mask |= NSResizableWindowMask;
	}
	if ([self isCloseable])
	{
		mask |= NSClosableWindowMask;
	}
	if ([self isMinimizable])
	{
		mask |= NSMiniaturizableWindowMask;
	}
	if ([self isMaximizable])
	{
		//???
	}
	if (![self isChrome])
	{
		mask |= NSBorderlessWindowMask;
	}
	else
	{
		mask |= NSTitledWindowMask;
	}		
	return mask;
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