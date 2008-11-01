//
//  TIBrowserDocument.m
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//

#import "TIBrowserDocument.h"
#import "TIBrowserWindowController.h"
#import <WebKit/WebKit.h>

@implementation TIBrowserDocument

- (id)init {
    self = [super init];
    if (self) {
    }
    return self;
}


- (void)dealloc {
	[super dealloc];
}


- (void)makeWindowControllers {
	TIBrowserWindowController *winController = [[TIBrowserWindowController alloc] initWithWindowNibName:@"BrowserWindow"];
	[self addWindowController:winController];
	[winController release];
}


- (BOOL)isDocumentEdited {
	return NO;
}


- (WebView *)webView {
	if ([[self windowControllers] count]) {
		TIBrowserWindowController *winController = [[self windowControllers] objectAtIndex:0];
		return [winController webView];
	} else {
		return nil;
	}
}

@end
