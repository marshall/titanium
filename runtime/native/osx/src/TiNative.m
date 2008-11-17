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

#import "TiNative.h"
#import "TiAppDelegate.h"
#import <WebKit/WebKit.h>

@implementation TiNative

- (id)initWithWebView:(WebView *)wv {
	self = [super init];
	if (self != nil) {
		webView = wv; // assign only. don't retain. prevents retain loop memory leak
	}
	return self;
}


- (void)dealloc {
	webView = nil;
	[super dealloc];
}


- (NSString *)description {
	return @"[TiNative object]";
}

- (TiSystemMenu *)createSystemMenu:(NSString*)url f:(WebScriptObject*)f
{
	TiSystemMenu *menu = [TiSystemMenu alloc];
	[menu initWithURL:url f:f];
	return menu;
}

- (TiUserWindow *)createWindow
{
	TiUserWindow *win = [[TiUserWindow alloc] init];
	return win;
}

- (void)include:(NSString *)s {
	NSString *resPath = [[NSBundle mainBundle] resourcePath];
	NSString *scriptPath = [[resPath stringByAppendingPathComponent:@"public"] stringByAppendingPathComponent:s];
	NSString *script = [NSString stringWithContentsOfFile:scriptPath];
	[[webView windowScriptObject] evaluateWebScript:script];
}


- (void)debug:(NSString *)s {
	NSLog(@"%@\n", s);
}


- (void)terminate {
	[NSApp terminate:self];
}


- (void)activate {
	[NSApp activateIgnoringOtherApps:YES];
}


- (void)hide {
	[NSApp hide:self];
}


- (void)minimize {
	[[webView window] miniaturize:self];
}


- (void)beep {
	NSBeep();
}

- (void)playSoundNamed:(NSString *)s {
	[[NSSound soundNamed:s] play];
}


#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel {
	
	// ?? do we want to expose all of these?
	
	if (sel == @selector(include:)) {
		return @"include";
	} else if (sel == @selector(createSystemMenu:f:)) {
		return @"createSystemMenu";
	} else if (sel == @selector(createWindow)) {
		return @"createWindow";
	} else if (sel == @selector(debug:)) {
		return @"debug";
	} else if (sel == @selector(terminate)) {
		return @"terminate";
	} else if (sel == @selector(activate)) {
		return @"activate";
	} else if (sel == @selector(hide)) {
		return @"hide";
	} else if (sel == @selector(minimize)) {
		return @"minimize";
	} else if (sel == @selector(beep)) {
		return @"beep";
	} else if (sel == @selector(playSoundNamed:)) {
		return @"playSound";
	} else if (sel == @selector(windowWidth)) {
		return @"getWindowWidth";
	} else if (sel == @selector(windowHeight)) {
		return @"getWindowHeight";
	} else if (sel == @selector(endpoint)) {
		return @"getEndpoint";
	} else if (sel == @selector(appName)) {
		return @"getAppName";
	} else if (sel == @selector(windowTitle)) {
		return @"getWindowTitle";
	} else if (sel == @selector(startPath)) {
		return @"getStartPath";
	} else if (sel == @selector(resourcePath)) {
		return @"getResourcePath";
	} else if (sel == @selector(loadPage)) {
		return @"loadPage";
	} else {
		return nil;
	}
}


+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return YES;
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	return nil;
}


#pragma mark -
#pragma mark Accessors

- (CGFloat)windowWidth {
	return [[TiAppDelegate instance] windowWidth];
}


- (CGFloat)windowHeight {
	return [[TiAppDelegate instance] windowHeight];
}


- (NSString *)endpoint {
	return [[TiAppDelegate instance] endpoint];
}


- (NSString *)appName {
	return [[TiAppDelegate instance] appName];
}


- (NSString *)windowTitle {
	return [[TiAppDelegate instance] windowTitle];
}


- (NSString *)startPath {
	return [[TiAppDelegate instance] startPath];
}

- (NSString *)resourcePath {
	return [[NSBundle mainBundle] resourcePath];
}

@end
