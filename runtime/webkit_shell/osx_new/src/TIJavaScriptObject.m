//
//  TIJavaScriptObject.m
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "TIJavaScriptObject.h"
#import "TIAppDelegate.h"
#import <WebKit/WebKit.h>

@implementation TIJavaScriptObject

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
	return [NSString stringWithFormat:@"<TIJavaScriptObject appName='%@', endpoint='%@', windowTitle='%@', startPath='%@'>",
			[self appName], [self endpoint], [self windowTitle], [self startPath]];
}


- (NSString *)toString {
	return @"[TiNative object]";
}


- (void)debug:(NSString *)s {
	NSLog(@"%@\n", s);
}


- (void)include:(NSString *)s {
	NSString *resPath = [[NSBundle mainBundle] resourcePath];
	NSString *scriptPath = [[resPath stringByAppendingPathComponent:@"public"] stringByAppendingPathComponent:s];
	NSString *script = [NSString stringWithContentsOfFile:scriptPath];
	[[webView windowScriptObject] evaluateWebScript:script];
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
	} else if (sel == @selector(debug:)) {
		return @"debug";
	} else if (sel == @selector(toString)) {
		return @"toString";
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
	return [[TIAppDelegate instance] windowWidth];
}


- (CGFloat)windowHeight {
	return [[TIAppDelegate instance] windowHeight];
}


- (NSString *)endpoint {
	return [[TIAppDelegate instance] endpoint];
}


- (NSString *)appName {
	return [[TIAppDelegate instance] appName];
}


- (NSString *)windowTitle {
	return [[TIAppDelegate instance] windowTitle];
}


- (NSString *)startPath {
	return [[TIAppDelegate instance] startPath];
}

@end
