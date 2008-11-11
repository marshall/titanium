//
//  TIAppDelegate.h
//  Titanium
//
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TIAppDelegate;
@class TIBrowserDocument;
@class TIBrowserWindowController;
@class WebFrame;

TIBrowserWindowController *TIFirstController();
TIBrowserWindowController *TIFrontController();

@interface TIAppDelegate : NSDocumentController {
	BOOL isFullScreen;

	CGFloat windowWidth;
	CGFloat windowHeight;
	NSString *endpoint;
	NSString *appName;
	NSString *windowTitle;
	NSString *startPath;
}
+ (id)instance;

- (IBAction)showPreferencesWindow:(id)sender;
- (IBAction)showAboutWindow:(id)sender;
- (IBAction)showWebInspector:(id)sender;
- (IBAction)showErrorConsole:(id)sender;
- (IBAction)showNetworkTimeline:(id)sender;
- (IBAction)toggleFullScreen:(id)sender;

- (TIBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request display:(BOOL)display;
- (WebFrame *)findFrameNamed:(NSString *)frameName;

- (void)parseTiAppXML;

- (BOOL)isFullScreen;
- (void)setIsFullScreen:(BOOL)yn;

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
