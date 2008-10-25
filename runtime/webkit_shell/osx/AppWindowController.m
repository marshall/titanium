//
//  AppWindowController.m
//  webkit_shell
//
//  Created by Marshall on 9/30/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import "AppWindowController.h"

extern int argCount;
extern char** args;

@implementation AppWindowController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	
	printf("app finished loadingL %s\n", args[1]);
	
	NSString *urlString = [[NSString alloc] initWithUTF8String:args[1]];
	NSURL *url = [NSURL URLWithString:urlString];
	NSURLRequest *request = [NSURLRequest requestWithURL:url];
	
	[[webView mainFrame] loadRequest:request];

}

- (IBAction)loadApp:(id)sender {
	}

@end
