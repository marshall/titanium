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
	bool chrome;
	CGFloat transparency;
	bool visible;
	bool minimizable;
	bool maximizable;
	bool resizable;
	bool fullscreen;
	bool closeable;
	bool scrollbars;
}

- (id) init;

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

- (bool) isChrome;
- (void) setChrome:(bool)yn;

- (CGFloat) getTransparency;
- (void) setTransparency:(CGFloat)f;

- (bool) isVisible;
- (void) setVisible:(bool)yn;

- (bool) isMinimizable;
- (void) setMinimizable:(bool)yn;

- (bool) isMaximizable;
- (void) setMaximizable:(bool)yn;

- (bool) isResizable;
- (void) setResizable:(bool)yn;

- (bool) isFullscreen;
- (void) setFullscreen:(bool)yn;

- (bool) isCloseable;
- (void) setCloseable:(bool)yn;

- (bool) isScrollbars;
- (void) setScrollbars:(bool)yn;

- (NSString*) getURL;
- (void) setURL:(NSString*)url;

- (NSUInteger) toWindowMask;

@end
