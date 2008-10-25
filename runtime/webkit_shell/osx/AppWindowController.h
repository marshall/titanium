//
//  AppWindowController.h
//  webkit_shell
//
//  Created by Marshall on 9/30/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "TitaniumJS.h"

@interface AppWindowController : NSObject {
	IBOutlet WebView* webView;
	TitaniumJS *titanium_js;
}

- (IBAction)loadApp:(id)sender;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;

@end
