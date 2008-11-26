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
#import "TiDocument.h"
#import "TiController.h"
 
@implementation TiUserWindow

- (id) initWithWindow:(TiWindow *)win
{
	self = [super init];
	if (self != nil)
	{
		window = win;
		[window retain];
	}
	return self;
}

- (void)dealloc 
{
	[window release];
	window = nil;
	[super dealloc];
}

- (NSString *)description 
{
	return @"[TiUserWindow native]";
}

- (NSString*)getTitle
{
	return [window title];
}

- (void)setTitle:(NSString*)title
{
	[window setTitle:title];
}

- (CGFloat)getTransparency
{
	return [window alphaValue];
}

- (void)setTransparency:(CGFloat)alphaValue
{
	[window setAlphaValue:alphaValue];
}

- (void)close 
{
	[[TiController getDocument:window] close];
}

- (void)hide 
{
	[window orderOut:nil]; // to hide it
}

- (void)show 
{
	[window makeKeyAndOrderFront:nil]; // to show it
}

// these are immutable properties
-(BOOL)isUsingChrome
{
	return [[window config] isChrome];
}

- (BOOL)isUsingScrollbars
{
	return [[window config] isScrollbars];
}

- (BOOL)isFullscreen
{
	return [[window config] isFullscreen];
}

- (NSString*)getID
{
	return [[window config] getID];
}

- (TiBounds*)getBounds
{
	TiBounds* bounds = [[TiBounds alloc] init];
	bounds.x = [self getX];
	bounds.y = [self getY];
	bounds.width = [self getWidth];
	bounds.height = [self getHeight];
	return [bounds autorelease];
}

- (void)setBounds:(TiBounds*)bounds
{
	[self setX:[bounds x]];
	[self setY:[bounds y]];
	[self setWidth:[bounds width]];
	[self setHeight:[bounds height]];
	//TODO: do i need to release?
	NSLog(@"bounds ref count= %d",[bounds retainCount]);
}


- (CGFloat)getX
{
	//FIXME: translate these since Cocoa is left-bottom origin
	NSRect o = [window frame];
	return o.origin.x;
}

- (void)setX:(CGFloat)newx
{
	//FIXME: translate these since Cocoa is left-bottom origin
	NSRect frame = [window frame];
	frame.origin.x = newx;
	[window setFrame:frame display:[self isVisible] animate:YES];
}

- (CGFloat)getY
{
	//FIXME: translate these since Cocoa is left-bottom origin
	NSRect o = [window frame];
	return o.origin.y;
}

- (void)setY:(CGFloat)newy
{
	//FIXME: translate these
	NSPoint point = [window frame].origin;
	point.y = newy;
	[window setFrameTopLeftPoint:point];
}

- (CGFloat)getWidth
{
	return [window frame].size.width;
}

- (void)setWidth:(CGFloat)width
{
	NSRect r = [window frame];
	r.size.width = width;
}

- (CGFloat)getHeight
{
	return [window frame].size.height;
}

- (void)setHeight:(CGFloat)height
{
	NSRect r = [window frame];
	r.size.height = height;
}

- (BOOL)isResizable
{
	return [[window config] isResizable];
}

- (void)setResizable:(BOOL)yn
{
	//FIXME: do we need to do anything else??
	[[window config] setResizable:yn];
}

- (BOOL)isMaximizable
{
	return [[window config] isMaximizable];
}

- (void)setMaximizable:(BOOL)yn
{
	[[window config] setMaximizable:yn];
	[[window standardWindowButton:NSWindowZoomButton] setHidden:yn==NO];
}

- (BOOL)isMinimizable
{
	return [[window config] isMinimizable];
}

- (void)setMinimizable:(BOOL)yn
{
	[[window config] setMinimizable:yn];
	[[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:yn==NO];
}

- (BOOL)isCloseable
{
	return [[window config] isCloseable];
}

- (void)setCloseable:(BOOL)yn
{
	[[window config] setCloseable:yn];
	[[window standardWindowButton:NSWindowCloseButton] setHidden:yn==NO];
}

- (BOOL)isVisible
{
	return [[window config] isVisible];
}

- (void)setVisible:(BOOL)yn
{
	[[window config] setVisible:yn];
	if (yn)
	{
		[self show];
	}
	else
	{
		[self hide];
	}
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
	else if (sel == @selector(hide))
	{
		return @"hide";
	}
	else if (sel == @selector(getTitle))
	{
		return @"getTitle";
	}
	else if (sel == @selector(setTitle:))
	{
		return @"setTitle";
	}
	else if (sel == @selector(setTransparency:))
	{
		return @"setTransparency";
	}
	else if (sel == @selector(getTransparency))
	{
		return @"getTransparency";
	}
	else if (sel == @selector(isUsingChrome))
	{
		return @"isUsingChrome";
	}
	else if (sel == @selector(getID))
	{
		return @"getID";
	}
	else if (sel == @selector(getX))
	{
		return @"getX";
	}
	else if (sel == @selector(setX:))
	{
		return @"setX";
	}
	else if (sel == @selector(getY))
	{
		return @"getY";
	}
	else if (sel == @selector(setY:))
	{
		return @"setY";
	}
	else if (sel == @selector(getWidth))
	{
		return @"getWidth";
	}
	else if (sel == @selector(setWidth:))
	{
		return @"setWidth";
	}
	else if (sel == @selector(getHeight))
	{
		return @"getHeight";
	}
	else if (sel == @selector(setHeight:))
	{
		return @"setHeight";
	}
	else if (sel == @selector(getBounds))
	{
		return @"getBounds";
	}
	else if (sel == @selector(setBounds:))
	{
		return @"setBounds";
	}
	else if (sel == @selector(getURL))
	{
		return @"getURL";
	}
	else if (sel == @selector(setURL:))
	{
		return @"setURL";
	}
	else if (sel == @selector(isResizable))
	{
		return @"isResizable";
	}
	else if (sel == @selector(setResizable:))
	{
		return @"setResizable";
	}
	else if (sel == @selector(isMaximizable))
	{
		return @"isMaximizable";
	}
	else if (sel == @selector(setMaximizable:))
	{
		return @"setMaximizable";
	}
	else if (sel == @selector(isMinimizable))
	{
		return @"isMinimizable";
	}
	else if (sel == @selector(setMinimizable:))
	{
		return @"setMinimizable";
	}
	else if (sel == @selector(isCloseable))
	{
		return @"isCloseable";
	}
	else if (sel == @selector(setCloseable:))
	{
		return @"setCloseable";
	}
	else if (sel == @selector(isFullscreen))
	{
		return @"isFullscreen";
	}
	else if (sel == @selector(isVisible))
	{
		return @"isVisible";
	}
	else if (sel == @selector(setVisible:))
	{
		return @"setVisible";
	}
	else if (sel == @selector(isUsingScrollbars))
	{
		return @"isUsingScrollbars";
	}
	else if (sel == @selector(getBounds))
	{
		return @"getBounds";
	}
	else if (sel == @selector(setBounds:))
	{
		return @"setBounds";
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
