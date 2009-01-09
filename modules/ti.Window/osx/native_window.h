/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "preinclude.h"
#import <WebKit/WebKit.h>
#import <Cocoa/Cocoa.h>
#import "webview_delegate.h"
#import "../user_window.h"
#import "../../ti.App/window_config.h"

@class WebViewDelegate;
class OSXUserWindow;

using namespace ti;

@interface NativeWindow : NSWindow {
	BOOL canReceiveFocus;
	WindowConfig* config;
	WebView* webView;
	WebViewDelegate* delegate;
}
- (void)setupDecorations:(WindowConfig*)config host:(Host*)h;
- (void)setTransparency:(double)transparency;
- (WebView*)webView;
- (WindowConfig*)config;
@end
