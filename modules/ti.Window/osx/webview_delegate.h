/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "preinclude.h"
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "WebViewPrivate.h"
#import "WebInspector.h"
#import "WebScriptDebugDelegate.h"
#import "WebScriptObject.h"
#import "window_module.h"

@class NativeWindow;

@interface WebViewDelegate : NSObject {
	NativeWindow *window;
	WebView *webView;
	Host *host;
	NSURL *url;
	WebInspector *inspector;
	BOOL scriptCleared;
	BOOL initialDisplay;
}
-(id)initWithWindow:(NativeWindow*)window host:(Host*)h;
-(void)setURL:(NSURL*)url;
-(void)closePrecedent;
-(NSURL*)url;
@end
