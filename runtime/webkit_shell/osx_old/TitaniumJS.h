//
//  titanium_js.h
//  webkit_shell
//
//  Created by Marshall on 10/22/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <WebKit/WebKit.h>

@interface TitaniumJS : NSObject {
	NSString *endpoint;
	NSString *appName;
	int windowWidth, windowHeight;
	NSString *windowTitle, *startPath;
	
	WebView *webView;
}

- (void)setWebView:(WebView *)webView;
- (void)setEndpoint:(NSString*)e;
- (void)setAppName:(NSString*)a;
- (void)setWindowDimensions:(int)w height:(int)h;
- (void)setWindowTitle:(NSString *)t;
- (void)setStartPath:(NSString*)p;

@end
