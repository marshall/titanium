//
//  AppWindowController.m
//  webkit_shell
//
//  Created by Marshall on 9/30/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import "AppWindowController.h"


@implementation AppWindowController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	
	printf("app finished loading\n");
	
	[[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:[NSURL fileURLWithPath:@"/Users/marshall/Code/appcelerator/appcelerator_titanium/trunk/app/titanium.html"]]];

}

- (IBAction)loadApp:(id)sender {
	}

@end
