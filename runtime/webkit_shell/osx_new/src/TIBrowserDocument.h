//
//  TIBrowserDocument.h
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WebView;

@interface TIBrowserDocument : NSDocument {

}
- (WebView *)webView;
@end
