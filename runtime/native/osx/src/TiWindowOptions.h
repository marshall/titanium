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
#import <Cocoa/Cocoa.h>

@interface TiWindowOptions : NSObject 
{
	NSString *windowID;
	CGFloat y;
	CGFloat x;
	CGFloat width;
	CGFloat height;
	CGFloat minWidth;
	CGFloat minHeight;
	CGFloat maxWidth;
	CGFloat maxHeight;
	NSString *title;
	NSString *url;
	BOOL chrome;
	CGFloat transparency;
	BOOL visible;
	BOOL minimizable;
	BOOL maximizable;
	BOOL resizable;
	BOOL fullscreen;
	BOOL closeable;
	BOOL scrollbars;
}

- (id) init;

- (NSString*) getID;
- (void) setID:(NSString*)i;

- (CGFloat) getX;
- (void) setX:(CGFloat)x;

- (CGFloat) getY;
- (void) setY:(CGFloat)y;

- (CGFloat) getWidth;
- (void) setWidth:(CGFloat)w;

- (CGFloat) getHeight;
- (void) setHeight:(CGFloat)h;

- (CGFloat) getMinWidth;
- (void) setMinWidth:(CGFloat)w;

- (CGFloat) getMinHeight;
- (void) setMinHeight:(CGFloat)h;

- (CGFloat) getMaxWidth;
- (void) setMaxWidth:(CGFloat)w;

- (CGFloat) getMaxHeight;
- (void) setMaxHeight:(CGFloat)h;

- (NSString*) getTitle;
- (void) setTitle:(NSString*)title;

- (BOOL) isChrome;
- (void) setChrome:(BOOL)yn;

- (CGFloat) getTransparency;
- (void) setTransparency:(CGFloat)f;

- (BOOL) isVisible;
- (void) setVisible:(BOOL)yn;

- (BOOL) isMinimizable;
- (void) setMinimizable:(BOOL)yn;

- (BOOL) isMaximizable;
- (void) setMaximizable:(BOOL)yn;

- (BOOL) isResizable;
- (void) setResizable:(BOOL)yn;

- (BOOL) isFullscreen;
- (void) setFullscreen:(BOOL)yn;

- (BOOL) isCloseable;
- (void) setCloseable:(BOOL)yn;

- (BOOL) isScrollbars;
- (void) setScrollbars:(BOOL)yn;

- (NSString*) getURL;
- (void) setURL:(NSString*)url;

- (NSUInteger) toWindowMask;
- (BOOL)urlMatches:(NSString*)url;

@end
