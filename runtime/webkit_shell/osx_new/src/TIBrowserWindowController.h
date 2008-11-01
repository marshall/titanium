//
//  TIBrowserWindowController.h
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
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
