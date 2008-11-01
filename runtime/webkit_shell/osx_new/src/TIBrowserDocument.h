//
//  TIBrowserDocument.h
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WebView;
@class TIBrowserWindowController;

@interface TIBrowserDocument : NSDocument {
	TIBrowserWindowController *browserWindowController;
}

- (void)loadRequest:(NSURLRequest *)request;
- (TIBrowserWindowController *)browserWindowController;
- (WebView *)webView;
@end
