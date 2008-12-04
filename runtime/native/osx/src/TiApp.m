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

#import "TiApp.h"
#import "TiController.h"

@implementation TiApp

- (id)initWithWindow:(TiWindow*)win
{
	self = [super init];
	if (self != nil) 
	{
		// don't retain
		window = win;
		webView = [TiController getWebView:window];
	}
	return self;
}

- (void)dealloc 
{
	webView = nil;
	window = nil;
	[super dealloc];
}


- (NSString *)description 
{
	return @"[TiApp native]";
}

- (void)include:(NSString *)s 
{
	[self includeFromObject:s webScriptObject:[webView windowScriptObject]];
}

- (void)includeFromObject:(NSString *)s webScriptObject:(WebScriptObject*)object
{
	TRACE(@"attempting to include %@\n", s);
	
	NSString *script = nil;
	if ([s rangeOfString:@"://"].location != NSNotFound) {
		NSURL *url = [[NSURL alloc] initWithString:s];
		@try {
			script = [NSString stringWithContentsOfURL:url];
		} @catch (id exception) {
			script = nil;
		}
		[url release];
	}
	else {
		NSString *resPath = [[NSBundle mainBundle] resourcePath];
		NSString *scriptPath = [[resPath stringByAppendingPathComponent:@"public"] stringByAppendingPathComponent:s];
		script = [NSString stringWithContentsOfFile:scriptPath];
		[resPath release];
		[scriptPath release];
	}
	
	if (script != nil) {
		[object evaluateWebScript:script];
	}
}


- (void)debug:(NSString *)s 
{
	NSLog(@"%@\n", s);
}


- (void)quit 
{
	[NSApp terminate:self];
}


- (void)activate 
{
	[NSApp activateIgnoringOtherApps:YES];
}


- (void)show 
{
	[[webView window] makeKeyAndOrderFront:nil];
}

- (void)hide 
{
	[[webView window] orderOut:nil];
}


- (void)minimize 
{
	[[webView window] miniaturize:self];
}


- (void)setSize:(int)width height:(int)height animate:(int)animate
{
	NSWindow *win = [webView window];
	NSRect r = [win frame];
	bool a = animate == 1 ? YES : NO;
	CGFloat w = width < 0 ? r.size.width : width;
	CGFloat h = height < 0 ? r.size.height : height;
	[win setFrame:NSMakeRect(r.origin.x,r.origin.y,w,h) display:YES animate:a];
	[win center]; //FIXME: this is temporary
}

/*
 NSColor: Instantiate from Web-like Hex RRGGBB string
 Original Source: <http://cocoa.karelia.com/Foundation_Categories/NSColor__Instantiat.m>
 (See copyright notice at <http://cocoa.karelia.com>)
 */

+ (NSColor *) colorFromHexRGB:(NSString *) inColorString
{
	if ([inColorString compare:@"transparent"]==0)
	{
		return [NSColor clearColor];
	}
		
	NSColor *result = nil;
	unsigned int colorCode = 0;
	unsigned char redByte, greenByte, blueByte;
	
	if (nil != inColorString)
	{
		NSScanner *scanner = [NSScanner scannerWithString:inColorString];
		(void) [scanner scanHexInt:&colorCode];	// ignore error
	}
	redByte		= (unsigned char) (colorCode >> 16);
	greenByte	= (unsigned char) (colorCode >> 8);
	blueByte	= (unsigned char) (colorCode);	// masks off high bits
	result = [NSColor
			  colorWithCalibratedRed:		(float)redByte	/ 0xff
			  green:	(float)greenByte/ 0xff
			  blue:	(float)blueByte	/ 0xff
			  alpha:1.0];
	return result;
}

- (void)setBackgroundColor:(NSString*)color
{
	NSWindow *win = [webView window];
	[win setBackgroundColor:[TiApp colorFromHexRGB:color]];
}

- (NSString*)resourcePath
{
	return [[NSBundle mainBundle] resourcePath];
}



#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}


+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(include:)) {
		return @"include";
	} else if (sel == @selector(debug:)) {
		return @"debug";
	} else if (sel == @selector(quit)) {
		return @"quit";
	} else if (sel == @selector(activate)) {
		return @"activate";
	} else if (sel == @selector(hide)) {
		return @"hide";
	} else if (sel == @selector(show)) {
		return @"show";
	} else if (sel == @selector(minimize)) {
		return @"minimize";
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
	} else if (sel == @selector(setSize:height:animate:)) {
		return @"setSize";
	} else if (sel == @selector(setBackgroundColor:)) {
		return @"setBackgroundColor";
	} else if (sel == @selector(getResourcePath)) {
		return @"getResourcePath";
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


@end

