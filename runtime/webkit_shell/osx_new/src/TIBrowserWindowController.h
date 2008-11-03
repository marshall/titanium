//
//  TIBrowserWindowController.h
//  Titanium
//
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WebView;

@interface TIBrowserWindowController : NSWindowController {
	IBOutlet WebView *webView;
}

- (BOOL)isFirst;
- (void)loadRequest:(NSURLRequest *)request;
- (WebView *)webView;
@end
