/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "preinclude.h"
#import <titanium/titanium.h>
#import <WebKit/WebKit.h>
#import <Cocoa/Cocoa.h>
#import <api/config/ti_window_config.h>
#import <api/ti_host.h>
#import "webview_delegate.h"

@class WebViewDelegate;
class TiOSXUserWindow;

@interface NativeTiWindow : NSWindow {
	BOOL canReceiveFocus;
	TiWindowConfig* config;
	WebView* webView;
	WebViewDelegate* delegate;
}
- (void)setupDecorations:(TiWindowConfig*)config host:(TiHost*)h;
- (void)setTransparency:(double)transparency;
- (WebView*)webView;
- (TiWindowConfig*)config;
@end
