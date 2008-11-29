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

#import "TiWindow.h"
#import "TiController.h"

@implementation TiWindow

-(void)dealloc
{
	TRACE(@"TiWindow::dealloc=%x",self);
	[config release];
	config=nil;
	[super dealloc];
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)inmask backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag 
{
	TRACE(@"TiWindow::initWithContentRect=%x",self);

	config = [[TiController instance] pendingConfig]; 
	[config retain];
	[[TiController instance] resetPendingConfig];

	NSUInteger mask = [config toWindowMask];

	// our tiapp.xml decides the initial size of the window not the nib
	NSRect r = NSMakeRect(contentRect.origin.x, contentRect.origin.y, [config getWidth], [config getHeight]);
	
	self = [super initWithContentRect:r styleMask:mask backing:bufferingType defer:NO];
	if (self != nil) 
	{
		[config assign:self];
		[self setOpaque:NO];
		[self setHasShadow:YES];
		[self setBackgroundColor:[NSColor clearColor]];
		[self setAlphaValue:[config getTransparency]];
		
		// turn on/off zoom button to control app maximize behavior
		[[self standardWindowButton:NSWindowZoomButton] setHidden:![config isMaximizable]];
		
		// this is suppose to enable move by grabbing a window background area
		[self setMovableByWindowBackground:YES];
		
		[self center];
	}
	return self;
}

- (TiWindowConfig*) config
{
	return config;
}


- (NSSize)windowWillResize:(NSWindow *) window toSize:(NSSize)newSize
{
	if ([config isResizable])
	{
		// if we're resizable, we need to resize within the constraints of the 
		// windows min/max width/height
		
		CGFloat minWidth = [config getMinWidth];
		CGFloat maxWidth = [config getMaxWidth];
		CGFloat minHeight = [config getMinHeight];
		CGFloat maxHeight = [config getMaxHeight];
		
		if (newSize.width >= minWidth && newSize.width <= maxWidth && 
			newSize.height >= minHeight && newSize.height <= maxHeight)
		{
			return newSize;
		}
	}
	return [window frame].size;
}

@end
