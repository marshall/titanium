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
#import <api/ti_host.h>
#import "../binding/kjs.h"

@class NativeTiWindow;

@interface WebViewDelegate : NSObject {
	NativeTiWindow *window;
	WebView *webView;
	TiHost *host;
	NSURL *url;
	WebInspector *inspector;
	BOOL scriptCleared;
	BOOL initialDisplay;
}
-(id)initWithWindow:(NativeTiWindow*)window host:(TiHost*)h;
-(void)setURL:(NSURL*)url;
-(void)closePrecedent;
-(NSURL*)url;
@end
