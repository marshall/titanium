//
//  TIBrowserWindowController.m
//  Titanium
//
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "TIBrowserWindowController.h"
#import "TIAppDelegate.h"
#import "TIBrowserDocument.h"
#import "TIJavaScriptObject.h"
#import "TIJavaScriptPromptWindowController.h"
#import "WebViewPrivate.h"
#import <WebKit/WebKit.h>

@interface NSApplication (DeclarationStolenFromAppKit)
- (void)_cycleWindowsReversed:(BOOL)reversed;
@end

typedef enum {
	WebNavigationTypePlugInRequest = WebNavigationTypeOther + 1
} WebExtraNavigationType;

@interface TIBrowserWindowController (Private)
- (WebView *)webView:(WebView *)wv createWebViewWithRequest:(NSURLRequest *)request windowFeatures:(NSDictionary *)features;
- (void)openPanelDidEnd:(NSSavePanel *)openPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo;
- (void)updateWindowFrameIfFirst;
- (void)customizeUserAgent;
- (void)showWindowIfFirst;
@end

@implementation TIBrowserWindowController

- (id)initWithWindowNibName:(NSString *)nibName {
	self = [super initWithWindowNibName:nibName];
	if (self != nil) {
		
	}
	return self;
}


- (void)dealloc {	
	[super dealloc];
}


- (void)awakeFromNib {
	[self updateWindowFrameIfFirst];
	[self customizeUserAgent];

	// customize WebView/Window
//	[[[webView mainFrame] frameView] setAllowsScrolling:NO];
//	[webView setBackgroundColor:[NSColor clearColor]];
//	[[self window] setShowsResizeIndicator:NO];
	
//	[webView setBackgroundColor:[NSColor colorWithDeviceRed:0. green:0. blue:1. alpha:.2]];
}


- (void)loadRequest:(NSURLRequest *)request {
	[[webView mainFrame] loadRequest:request];
}


- (void)handleLoadError:(NSError *)err {
	// TODO
	NSLog(@"Load Error: %@", err);
}


- (void)includeScript:(NSString*)path {
	DOMDocument *doc = [webView mainFrameDocument];
	if (!doc) return;

	DOMNodeList *headEls = [doc getElementsByTagName:@"head"];
	if (![headEls length]) return;
	
	DOMElement *headEl = (DOMElement *)[headEls item:0];
	if (!headEl) return;
	
	DOMElement *scriptEl = [doc createElement:@"script"];
	[scriptEl setAttribute:@"src" value:path];
	[scriptEl setAttribute:@"type" value:@"text/javascript"];
	//DOMText *text = [doc createTextNode:@""];
	//[script appendChild:text];

	[headEl appendChild:scriptEl];
}


#pragma mark -
#pragma mark Private

- (void)updateWindowFrameIfFirst {
	if ([self isFirst]) {
		CGFloat w = [[TIAppDelegate instance] windowWidth];
		CGFloat h = [[TIAppDelegate instance] windowHeight];
		[[self window] setFrame:NSMakeRect(0.0, 0.0, w, h) display:NO];
		[[self window] center];
	}
}


- (void)customizeUserAgent {
	// produces something like:
	// Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_5; en-us) AppleWebKit/525.18 (KHTML, like Gecko) Appcelerator Titanium
	[webView setApplicationNameForUserAgent:@"Appcelerator Titanium"];	
}


- (void)showWindowIfFirst {
	// we don't show the first window until the first page has loaded. avoids seeing ugly loading on launch
	if ([self isFirst] && ![[self window] isVisible]) {
		[[self window] makeKeyAndOrderFront:self];
	}
}


#pragma mark -
#pragma mark WebFrameLoadDelegate

- (void)webView:(WebView *)wv didStartProvisionalLoadForFrame:(WebFrame *)frame {
	
}


- (void)webView:(WebView *)wv didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame {
	if (frame != [webView mainFrame]) return;
	[self handleLoadError:error];
}


- (void)webView:(WebView *)wv didCommitLoadForFrame:(WebFrame *)frame {
	
}


- (void)webView:(WebView *)wv didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame {
	
}


- (void)webView:(WebView *)wv didFinishLoadForFrame:(WebFrame *)frame {
	if (frame != [webView mainFrame]) return;
	[self performSelector:@selector(showWindowIfFirst) withObject:nil afterDelay:0.0]; // show window in next runloop
}


- (void)webView:(WebView *)wv didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame {
	if (frame != [webView mainFrame]) return;
	[self handleLoadError:error];
}


- (void)webView:(WebView *)wv windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject {
	if (wv != webView) return;
	TIJavaScriptObject *javaScriptObject = [[TIJavaScriptObject alloc] initWithWebView:webView];
	[windowScriptObject setValue:javaScriptObject forKey:@"TiNative"];
	[javaScriptObject include:@"titanium/titanium.js"];
	[javaScriptObject include:@"titanium/plugins.js"];
	[javaScriptObject release];
}


#pragma mark -
#pragma mark WebPolicyDelegate

- (void)webView:(WebView *)wv decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener {
	WebNavigationType navType = [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue];
	
	if ([WebView _canHandleRequest:request]) {
		[listener use];
	} else if (navType == WebNavigationTypePlugInRequest) {
		[listener use];
	} else {
		// A file URL shouldn't fall through to here, but if it did,
		// it would be a security risk to open it.
		if (![[request URL] isFileURL]) {
			[[NSWorkspace sharedWorkspace] openURL:[request URL]];
		}
		[listener ignore];
	}
}


- (void)webView:(WebView *)wv decidePolicyForNewWindowAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request newFrameName:(NSString *)frameName decisionListener:(id<WebPolicyDecisionListener>)listener {
	
	if ([@"_blank" isEqualToString:frameName] || [@"_new" isEqualToString:frameName]) { // force new window
		[listener use];
		return;
	}
	
	// look for existing frame with this name. if found, use it
	WebFrame *existingFrame = [[TIAppDelegate instance] findFrameNamed:frameName];
	
	if (existingFrame) {
		// found an existing frame with frameName. use it, and suppress new window creation
		[[[existingFrame webView] window] makeKeyAndOrderFront:self];
		[existingFrame loadRequest:request];
		[listener ignore];
	} else {
		// no existing frame for frameName. allow a new window to be created
		[listener use];
	}
}


- (void)webView:(WebView *)wv decidePolicyForMIMEType:(NSString *)type request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener {
	id response = [[frame provisionalDataSource] response];
	
	if (response && [response respondsToSelector:@selector(allHeaderFields)]) {
		NSDictionary *headers = [response allHeaderFields];
		
		NSString *contentDisposition = [[headers objectForKey:@"Content-Disposition"] lowercaseString];
		if (contentDisposition && NSNotFound != [contentDisposition rangeOfString:@"attachment"].location) {
			if (![[[request URL] absoluteString] hasSuffix:@".user.js"]) { // don't download userscripts
				//[listener download];
				[listener ignore]; // ignoring for now. do we want a download manager?
				return;
			}
		}
		
		NSString *contentType = [[headers objectForKey:@"Content-Type"] lowercaseString];
		if (contentType && NSNotFound != [contentType rangeOfString:@"application/octet-stream"].location) {
			//[listener download];
			[listener ignore]; // ignoring for now. do we want a download manager?
			return;
		}
	}
	
	
	if ([[request URL] isFileURL]) {
		BOOL isDirectory = NO;
		[[NSFileManager defaultManager] fileExistsAtPath:[[request URL] path] isDirectory:&isDirectory];
		
		if (isDirectory) {
			[listener ignore];
		} else if ([WebView canShowMIMEType:type]) {
			[listener use];
		} else{
			[listener ignore];
		}
	} else if ([WebView canShowMIMEType:type]) {
		[listener use];
	} else {
		//[listener download];
		[listener ignore]; // ignoring for now. do we want a download manager?
	}
}


#pragma mark -
#pragma mark WebUIDelegate

- (void)webViewClose:(WebView *)wv {
	[[wv window] close];
}


- (void)webViewFocus:(WebView *)wv {
	[[wv window] makeKeyAndOrderFront:wv];
}


- (void)webViewUnfocus:(WebView *)wv {
	if ([[wv window] isKeyWindow] || [[[wv window] attachedSheet] isKeyWindow]) {
		[NSApp _cycleWindowsReversed:FALSE];
	}
}


- (NSResponder *)webViewFirstResponder:(WebView *)wv {
	return [[wv window] firstResponder];
}


- (void)webView:(WebView *)wv makeFirstResponder:(NSResponder *)responder {
	[[wv window] makeFirstResponder:responder];
}


- (NSString *)webViewStatusText:(WebView *)wv {
	return nil;
}


- (BOOL)webViewIsResizable:(WebView *)wv {
	return [[wv window] showsResizeIndicator];
}


- (void)webView:(WebView *)wv setResizable:(BOOL)resizable; {
	// FIXME: This doesn't actually change the resizability of the window,
	// only visibility of the indicator.
	[[wv window] setShowsResizeIndicator:resizable];
}


- (void)webView:(WebView *)wv setFrame:(NSRect)frame {
	[[wv window] setFrame:frame display:YES];
}


- (NSRect)webViewFrame:(WebView *)wv {
	NSWindow *window = [wv window];
	return window == nil ? NSZeroRect : [window frame];
}


- (BOOL)webViewAreToolbarsVisible:(WebView *)wv {
	return NO;
}


- (BOOL)webViewIsStatusBarVisible:(WebView *)wv {
	return NO;
}


- (WebView *)webView:(WebView *)wv createWebViewWithRequest:(NSURLRequest *)request {
	return [self webView:wv createWebViewWithRequest:request windowFeatures:nil];
}


- (WebView *)webView:(WebView *)wv createWebViewWithRequest:(NSURLRequest *)request windowFeatures:(NSDictionary *)features {
	// TODO handle fullscreen feature
	//BOOL fullscreen = [[features objectForKey:@"fullscreen"] boolValue];

	TIBrowserDocument *doc = [[TIAppDelegate instance] newDocumentWithRequest:request display:NO];
	
	// handle frame features
	id xObj = [features objectForKey:@"x"];
	id yObj = [features objectForKey:@"y"];
	id wObj = [features objectForKey:@"width"];
	id hObj = [features objectForKey:@"height"];
	
	NSWindow *window = [[doc browserWindowController] window];
	NSRect winFrame = [window frame];
	NSRect screenFrame = [[window screen] frame];

	CGFloat y = yObj ? [yObj floatValue] : winFrame.origin.y;
	CGFloat w = wObj ? [wObj floatValue] : winFrame.size.width;
	CGFloat h = hObj ? [hObj floatValue] : winFrame.size.height;

	// Cocoa screen coords are from bottom left. but web coords are from top left. must convert origin.x
	CGFloat x = winFrame.origin.x;
	if (xObj) {
		x = [xObj floatValue];
		x = screenFrame.size.height - x - h;
	}

	[window setFrame:NSMakeRect(x, y, w, h) display:NO];
	
	// handle resizable feature
	// for some reason, this is always reported as 1 by WebKit. dunno why
//	BOOL resizable = [[features objectForKey:@"resizable"] boolValue];
//	[window setShowsResizeIndicator:resizable];
	
	// handle scrollbars feature
	BOOL scrollbarsVisible = [[features objectForKey:@"scrollbarsVisible"] boolValue];
	[[[[doc webView] mainFrame] frameView] setAllowsScrolling:scrollbarsVisible];

	return [doc webView];
}


- (void)webViewShow:(WebView *)wv {
	[[wv window] makeKeyAndOrderFront:self];
}


- (void)webView:(WebView *)wv runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
	NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
								 message,								// message
								 NSLocalizedString(@"OK", @""),			// default button
								 nil,									// alt button
								 nil);									// other button	
}


- (BOOL)webView:(WebView *)wv runJavaScriptConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
	NSInteger result = NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
													message,								// message
													NSLocalizedString(@"OK", @""),			// default button
													NSLocalizedString(@"Cancel", @""),		// alt button
													nil);
	return NSAlertDefaultReturn == result;	
}


- (NSString *)webView:(WebView *)wv runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText initiatedByFrame:(WebFrame *)frame {
	TIJavaScriptPromptWindowController *promptController = [[[TIJavaScriptPromptWindowController alloc] initWithWindowNibName:@"JavaScriptPromptWindow"] autorelease];
	[promptController setLabelText:prompt];
	[promptController setUserText:defaultText];
	
	[promptController showWindow:self];

	NSInteger result = [NSApp runModalForWindow:[promptController window]];
	
	if (NSOKButton == result) {
		return [promptController userText];
	} else {
		return nil;
	}
}


- (BOOL)webView:(WebView *)wv runBeforeUnloadConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
	NSInteger result = NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
													message,								// message
													NSLocalizedString(@"OK", @""),			// default button
													NSLocalizedString(@"Cancel", @""),		// alt button
													nil);
	return NSAlertDefaultReturn == result;	
}


- (void)webView:(WebView *)wv runOpenPanelForFileButtonWithResultListener:(id <WebOpenPanelResultListener>)resultListener {
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	[openPanel beginSheetForDirectory:nil 
								 file:nil 
					   modalForWindow:[self window]
						modalDelegate:self
					   didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) 
						  contextInfo:resultListener];	
}


- (void)openPanelDidEnd:(NSSavePanel *)openPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo {
	id <WebOpenPanelResultListener>resultListener = (id <WebOpenPanelResultListener>)contextInfo;
	if (NSOKButton == returnCode) {
		[resultListener chooseFilename:[openPanel filename]];
	}
}


- (BOOL)webView:(WebView *)wv shouldReplaceUploadFile:(NSString *)path usingGeneratedFilename:(NSString **)filename {
	return NO;
}


- (NSString *)webView:(WebView *)wv generateReplacementFile:(NSString *)path {
	return nil;
}


- (BOOL)webView:(WebView *)wv shouldBeginDragForElement:(NSDictionary *)element dragImage:(NSImage *)dragImage mouseDownEvent:(NSEvent *)mouseDownEvent mouseDraggedEvent:(NSEvent *)mouseDraggedEvent {
	return YES;
}


- (NSUInteger)webView:(WebView *)wv dragDestinationActionMaskForDraggingInfo:(id <NSDraggingInfo>)draggingInfo {
	return WebDragDestinationActionAny;
}


- (void)webView:(WebView *)webView willPerformDragDestinationAction:(WebDragDestinationAction)action forDraggingInfo:(id <NSDraggingInfo>)draggingInfo {
}


- (NSUInteger)webView:(WebView *)wv dragSourceActionMaskForPoint:(NSPoint)point; {
	return WebDragSourceActionAny;
}


#pragma mark -
#pragma mark Accessors

- (BOOL)isFirst {
	TIBrowserDocument *doc = (TIBrowserDocument *)[self document];
	return [doc isFirst];
}


- (WebView *)webView {
	return webView;
}

@end
