//
//  TIJavaScriptObject.h
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WebView;

@interface TIJavaScriptObject : NSObject {
	WebView *webView;
}
- (id)initWithWebView:(WebView *)wv;

- (NSString *)toString;

- (void)include:(NSString *)s;
- (void)debug:(NSString *)s;

- (CGFloat)windowWidth;
- (CGFloat)windowHeight;
- (NSString *)endpoint;
- (NSString *)appName;
- (NSString *)windowTitle;
- (NSString *)startPath;
@end
