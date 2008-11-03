//
//  TIJavaScriptObject.h
//  Titanium
//
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WebView;

@interface TIJavaScriptObject : NSObject {
	WebView *webView;
}
- (id)initWithWebView:(WebView *)wv;

- (void)include:(NSString *)s;
- (void)debug:(NSString *)s;
- (void)terminate;
- (void)activate;
- (void)hide;
- (void)minimize;
- (void)beep;
- (void)playSoundNamed:(NSString *)s;

- (CGFloat)windowWidth;
- (CGFloat)windowHeight;
- (NSString *)endpoint;
- (NSString *)appName;
- (NSString *)windowTitle;
- (NSString *)startPath;
@end
