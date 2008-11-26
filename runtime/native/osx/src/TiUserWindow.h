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
#import <WebKit/WebKit.h>
#import "TiWindow.h"
#import "TiBounds.h"

@interface TiUserWindow : NSObject {
	TiWindow *window;
	WebView *webView;
}

- (void)close;
- (void)show:(BOOL)animate;
- (void)hide:(BOOL)animate;

- (NSString*)getTitle;
- (void)setTitle:(NSString *)title;

- (CGFloat)getTransparency;
- (void)setTransparency:(CGFloat)alpha;

// these are immutable properties
- (BOOL)isUsingChrome;
- (BOOL)isUsingScrollbars;
- (BOOL)isFullscreen;

- (NSString*)getID;

- (CGFloat)getX;
- (void)setX:(CGFloat)x;

- (CGFloat)getY;
- (void)setY:(CGFloat)y;

- (CGFloat)getWidth;
- (void)setWidth:(CGFloat)width;

- (CGFloat)getHeight;
- (void)setHeight:(CGFloat)height;

- (TiBounds*)getBounds;
- (void)setBounds:(TiBounds*)bounds;

- (BOOL)isResizable;
- (void)setResizable:(BOOL)yn;

- (BOOL)isMaximizable;
- (void)setMaximizable:(BOOL)yn;

- (BOOL)isMinimizable;
- (void)setMinimizable:(BOOL)yn;

- (BOOL)isCloseable;
- (void)setCloseable:(BOOL)yn;

- (BOOL)isVisible;
- (void)setVisible:(BOOL)yn;


@end
