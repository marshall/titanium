//
//  NSDocumentController+TIAdditions.m
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "NSDocumentController+TIAdditions.h"
#import "TIBrowserDocument.h"
#import <WebKit/WebKit.h>

@implementation NSDocumentController (TIAdditions)

- (TIBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request makeKey:(BOOL)makeKey {
	TIBrowserDocument *oldDoc = [self currentDocument];
	TIBrowserDocument *newDoc = [self openUntitledDocumentAndDisplay:makeKey error:nil];
	
	if (!makeKey) {
		[newDoc makeWindowControllers];
	}
	
	WebView *webView = [newDoc webView];
	[[webView mainFrame] loadRequest:request];
	
	if (!makeKey) {
		NSWindow *oldWindow = [[[oldDoc windowControllers] objectAtIndex:0] window];
		NSWindow *newWindow = [[[newDoc windowControllers] objectAtIndex:0] window];;
		[newWindow orderWindow:NSWindowBelow relativeTo:oldWindow.windowNumber];
	}
	
	return newDoc;
}


@end
