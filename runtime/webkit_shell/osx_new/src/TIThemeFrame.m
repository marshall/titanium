//
//  TIThemeFrame.m
//  Titanium
//
//  Created by Todd Ditchendorf on 11/2/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "TIThemeFrame.h"

@implementation TIThemeFrame

// supress titleBar
+ (CGFloat)_titlebarHeight:(unsigned int)fp8 {
	return 0.0;
}


//- (void)_drawResizeIndicators:(NSRect)rect {
//	NSImage *resizeIndicator = [NSImage imageNamed:@"resizeIndicator"];
////	NSPoint indicatorPoint = NSMakePoint(NSMaxX([self frame]) - resizeIndicator.size.width - 2.0, 2.0);
//	NSRect indicatorRect = (NSRect){NSZeroPoint, resizeIndicator.size};
//	[resizeIndicator drawAtPoint:rect.origin fromRect:indicatorRect operation:NSCompositeSourceAtop fraction:1];
//}

//- (void)drawRect:(NSRect)rect {
//	//[super drawRect:rect];
//	
//	[self lockFocus];
//
//	// clear
//	[[NSColor clearColor] set];
//	NSRectFill(rect);
//	
//	NSRect imgRect = NSMakeRect(5., 5., 11., 11.);
//	NSImage *img = [NSImage imageNamed:@"resizeIndicator"];
//	[img drawInRect:imgRect
//		   fromRect:NSZeroRect
//		  operation:NSCompositeSourceOver
//		   fraction:1.];
//
//	[self unlockFocus];
//}

@end
