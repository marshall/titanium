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
#import "TiSplashScreen.h"

@implementation TiSplashScreen 
	
- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)style backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
	// Determine the center of the main screen and set the origin of the content rect
	// so that the window will be centered in the main screen.
	NSScreen* mainScreen = [NSScreen mainScreen];
	NSRect screen = [mainScreen frame];
	NSPoint center = NSMakePoint(screen.size.width / 2, screen.size.height / 2);
	contentRect.origin.x = center.x - (contentRect.size.width / 2);
	contentRect.origin.y = center.y - (contentRect.size.height / 2);
	
	// Call the inherited init function, but pass NSBorderlessWindowMask instead of
	// the normal style. This will give us a window with no title bar or edge.
	id window = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:bufferingType defer:NO];
	
	if (window)
	{
		// Set the opacity of the window so that we can see through it
		[window setOpaque:NO];
		
		// Set the window's background color to a clear color (without this step, the
		// normal OS X background with the alternating gray bars will still draw).
		[window setBackgroundColor:[NSColor blackColor]];
		[window setAlphaValue:0.85];
		
		// We don't want the window's regular (rectanglar) shadow to draw since our
		// splash image already has the shadow in it
		[window setHasShadow:YES];
		
		// Make the window sit on top of all other windows. Note that this particular
		// level will make the splash screen float above ALL windows, even of other
		// applications. As long as its displayed for less then 2 seconds most users
		// won't complain about this, but you may want to consider changing the level
		// to the highest application level window instead so that the user can
		// continue to use their computer while your program is loading. See the
		// documentation on NSWindow for descriptions of the various levels of windows.
		[window setLevel:NSFloatingWindowLevel];
	}
	
	return window;
}

@end
