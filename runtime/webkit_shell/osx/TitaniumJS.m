//
//  titanium_js.m
//  webkit_shell
//
//  Created by Marshall on 10/22/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import "TitaniumJS.h"

@implementation TitaniumJS

- (void)debug:(NSString*)s
{
	NSLog(@"%@\n", s);
}

- (void)include:(NSString*)s
{
	NSString *scriptPath = [[[[NSBundle mainBundle] resourcePath] stringByAppendingString:@"/public/"]
		 stringByAppendingString:s];
	
	NSLog(@"%@\n", scriptPath);
	
	NSString *script = [NSString stringWithContentsOfFile:scriptPath];
	NSLog(@"%@\n", script);
	
	[[webView windowScriptObject] evaluateWebScript:script];
	
	
}

- (void)setWebView:(WebView *)w
{
	webView = w;
}

- (void)setEndpoint:(NSString*)e
{
	endpoint = [[NSString alloc] initWithString:e];
}

- (void)setAppName:(NSString*)a
{
	appName = [[NSString alloc] initWithString:a];
}

- (void)setWindowDimensions:(int)w height:(int)h;
{
	windowWidth = w;
	windowHeight = h;
}

- (void)setWindowTitle:(NSString *)t
{
	windowTitle = [[NSString alloc] initWithString:t];
}

- (void)setStartPath:(NSString*)p
{
	startPath = [[NSString alloc] initWithString:p];
}

- (NSString *)getEndpoint {
	return endpoint;
}

- (NSString *)getAppName {
	return appName;
}

- (int)getWindowWidth {
	return windowWidth;
}

- (int)getWindowHeight {
	return windowHeight;
}

- (NSString*)getWindowTitle {
	return windowTitle;
}

- (NSString*)getStartPath {
	return startPath;
}


+ (BOOL)isSelectorExcludedFromWebScript:(SEL)aSelector
{
	
	return NO;
}

+ (NSString *)webScriptNameForSelector:(SEL)aSelector
{
	if (aSelector == @selector(getEndpoint)) {
		return @"getEndpoint";
	}
	
	if (aSelector == @selector(include:)) {
		NSLog(@"include selector\n");
		return @"include";
	}
	
	if (aSelector == @selector(debug:)) {
		NSLog(@"debug selector\n");
		return @"debug";
	}
	
	return nil;
}

+ (BOOL)isKeyExcludedFromWebScript:(const char*)k {
	return NO;
}

+ (NSString *)webScriptNameForKey:(const char *)name {
	return nil;
}

@end
