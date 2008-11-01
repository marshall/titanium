//
//  TIAppDelegate.h
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TIAppDelegate;
@class TIBrowserDocument;
@class TIBrowserWindowController;

TIBrowserWindowController *TIFrontController();

@interface TIAppDelegate : NSDocumentController {
	CGFloat windowWidth;
	CGFloat windowHeight;
	NSString *endpoint;
	NSString *appName;
	NSString *windowTitle;
	NSString *startPath;
}
+ (id)instance;

- (IBAction)showPreferencesWindow:(id)sender;

- (TIBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request makeKey:(BOOL)makeKey;

- (void)parseTiAppXml;

- (CGFloat)windowWidth;
- (CGFloat)windowHeight;
- (void)setWindowWidth:(CGFloat)w height:(CGFloat)h;
- (NSString *)endpoint;
- (void)setEndpoint:(NSString *)s;
- (NSString *)appName;
- (void)setAppName:(NSString *)s;
- (NSString *)windowTitle;
- (void)setWindowTitle:(NSString *)s;
- (NSString *)startPath;
- (void)setStartPath:(NSString *)s;
@end
