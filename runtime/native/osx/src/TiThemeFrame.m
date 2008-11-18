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


#import "TiThemeFrame.h"

@implementation TiThemeFrame

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
