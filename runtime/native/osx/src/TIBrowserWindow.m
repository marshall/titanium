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


#import "TIBrowserWindow.h"
#import "TIAppDelegate.h"
#import "TIThemeFrame.h"
#import "TIBrowserWindowController.h"
#import "WebViewPrivate.h"
#import "WebDashboardRegion.h"
#import <WebKit/WebKit.h>

@interface NSWindow ()
+ (Class)frameViewClassForStyleMask:(NSUInteger)styleMask;
@end

@implementation TIBrowserWindow

// must override the ThemeFrame class to force no drawing of titlebar when window is resizable
+ (Class)frameViewClassForStyleMask:(NSUInteger)styleMask {
	if ([[TIAppDelegate instance] isFullScreen]) {
		return [TIThemeFrame class];
	} else {
		return [super frameViewClassForStyleMask:styleMask];
	}
}


// override designated initializer to cause window to be borderless.
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)mask backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag {
	if ([[TIAppDelegate instance] isFullScreen]) {
		mask = NSTitledWindowMask|NSClosableWindowMask|NSBorderlessWindowMask;
	}
	//mask = NSClosableWindowMask|NSMiniaturizableWindowMask|NSTitledWindowMask|NSResizableWindowMask; //|NSBorderlessWindowMask|NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask;
	self = [super initWithContentRect:contentRect styleMask:mask backing:bufferingType defer:flag];
	if (self != nil) {
		[self setOpaque:NO];
		
	}
	return self;
}


- (void)moveWindow:(NSEvent *)event {
	NSPoint startLocation = [event locationInWindow];
	NSPoint lastLocation = startLocation;
	BOOL mouseUpOccurred = NO;
	
	while (!mouseUpOccurred) {
		// set mouseUp flag here, but process location of event before exiting from loop, leave mouseUp in queue
		event = [self nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask) untilDate:[NSDate distantFuture] inMode:NSEventTrackingRunLoopMode dequeue:YES];
		
		if ([event type] == NSLeftMouseUp)
			mouseUpOccurred = YES;
		
		NSPoint newLocation = [event locationInWindow];
		if (NSEqualPoints(newLocation, lastLocation))
			continue;
		
		NSPoint origin = [self frame].origin;
		NSPoint newOrigin = NSMakePoint(origin.x + newLocation.x - startLocation.x, origin.y + newLocation.y - startLocation.y);
		
		[self setFrameOrigin:newOrigin];
		lastLocation = newLocation;
	}
	
	[super sendEvent:event];
}


- (void)sendEvent:(NSEvent *)evt {
//	NSRect rightResizer = NSMakeRect(self.frame.size.width-30., 0., 30., 30.);
//	if ([evt type] == NSLeftMouseDown && NSPointInRect([evt locationInWindow], rightResizer)) {
//		// no resizing from right resizer
//		// eat it
//		return;
//	}
	
	BOOL isDraggable = YES;
	
	if (isDraggable) {		

		BOOL clickedOnWebView = YES;
		if (clickedOnWebView) {
			if (mouseInRegion && [evt type] == NSLeftMouseUp)
				mouseInRegion = NO;
			
			if (([evt type] == NSLeftMouseDown || [evt type] == NSLeftMouseDragged) && !mouseInRegion) {
				TIBrowserWindowController *winController = [self windowController];
				WebView *webView = [winController webView];
				NSPoint pointInView = [[[[webView mainFrame] frameView] documentView] convertPoint:[evt locationInWindow] fromView:nil];
				NSDictionary *regions = [webView _dashboardRegions];
				
				WebDashboardRegion *region = [[regions objectForKey:@"resize"] lastObject];
//				if (region) {
//					if (NSPointInRect(pointInView, [region dashboardRegionClip])) {
//						// we are in a resize control region, resize the window now and eat the event
//						// [self resizeWindow:event];
//						//return;
//					}
//				}
				
				NSArray *controlRegions = [regions objectForKey:@"control"];
				NSEnumerator *enumerator = [controlRegions objectEnumerator];
				while ((region = [enumerator nextObject])) {
					if (NSPointInRect(pointInView, [region dashboardRegionClip])) {
						// we are in a control region, lets pass the event down
						mouseInRegion = YES;
						[super sendEvent:evt];
						return;
					}
				}
				
				// if we are dragging and the mouse isn't in a control region move the window
				if ([evt type] == NSLeftMouseDragged) {
					[self moveWindow:evt];
					return;
				}
			}
		}
	}
	
	[super sendEvent:evt];
}

@end
