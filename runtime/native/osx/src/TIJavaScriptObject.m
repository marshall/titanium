//
//  TIJavaScriptObject.m
//  Titanium
//
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
	return @"[TiNative object]";
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

- (NSString *)resourcePath {
	return [[NSBundle mainBundle] resourcePath];
}

@end
