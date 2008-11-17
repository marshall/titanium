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

@implementation TiWindowOptions

- (id) init
{
	self = [super init];
	return self;
}

- (void)dealloc
{
	title = nil;
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
	return transparent;
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
	
	//mask = NSClosableWindowMask|NSMiniaturizableWindowMask|NSTitledWindowMask|NSResizableWindowMask; //|NSBorderlessWindowMask|NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask;
	return mask;
}

@end