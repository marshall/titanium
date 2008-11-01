//
//  TIBrowserDocument.m
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//

#import "TIBrowserDocument.h"
#import "TIBrowserWindowController.h"
#import "TIAppDelegate.h"
#import <WebKit/WebKit.h>

@interface TIBrowserDocument (Friend)
- (void)setIsFirst:(BOOL)yn;
@end

@implementation TIBrowserDocument

- (id)init {
    self = [super init];
    if (self) {
    }
    return self;
}


- (void)dealloc {
	browserWindowController = nil;
	[super dealloc];
}


#pragma mark -
#pragma mark NSDocument

- (void)makeWindowControllers {
	browserWindowController = [[TIBrowserWindowController alloc] initWithWindowNibName:@"BrowserWindow"];
	[self addWindowController:browserWindowController];
	[browserWindowController release];
}


- (BOOL)isDocumentEdited {
	return NO;
}


- (NSString *)displayName {
	return [[TIAppDelegate instance] windowTitle];
}


#pragma mark -
#pragma mark Public

- (void)loadRequest:(NSURLRequest *)request {
	[[[self webView] mainFrame] loadRequest:request];
}


- (TIBrowserWindowController *)browserWindowController {
	return browserWindowController;
}


#pragma mark -
#pragma mark Accessors

- (BOOL)isFirst {
	return isFirst;
}


- (void)setIsFirst:(BOOL)yn {
	isFirst = yn;
}


- (WebView *)webView {
	return [browserWindowController webView];
}

@end
