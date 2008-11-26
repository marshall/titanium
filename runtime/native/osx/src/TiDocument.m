/**
 * This file is part of Appcelerator's Titanium project.
 *
 * Copyright 2008 Appcelerator, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */
#import <WebKit/WebKit.h>
#import "TiDocument.h"
#import "TiObject.h"
#import "TiController.h"
#import "TiWindowConfig.h"
#import "TiWindow.h"
#import "WebViewPrivate.h"
#import "WebViewInternal.h"

@class WebPluginDatabase;

@interface NSApplication (DeclarationStolenFromAppKit)
- (void)_cycleWindowsReversed:(BOOL)reversed;
@end

@implementation TiDocument

- (id) init
{
	self = [super init];
	if (self != nil)
	{
		TRACE(@"TiDocument::init =%x",self);
	}
	return self;
}
- (void)dealloc
{
	TRACE(@"TiDocument::dealloc =%x",self);
	
    [webView close];
    [url release];
    [super dealloc];

	NSDocumentController *c = [NSDocumentController sharedDocumentController];
	NSArray *docs = [c documents];
	if ([docs count] == 0)
	{
		TRACE(@"Last application window has closed, exiting the application");
		// once we have no more active windows, we're going to shutdown -- this probably should be configurable at some point
		[NSApp terminate:self];
	}
}

- (id)webView
{
    return webView;
}

- (TiWindow*) window
{
	NSArray* windowControllers = [self windowControllers];
	NSWindowController *c = [windowControllers objectAtIndex:0];
	return (TiWindow*)[c window];
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"TiDocument";
}

- (void)loadURL:(NSURL *)URL
{
	[url release];
	url = [URL copy];
	TRACE(@"TiDocument::loadURL=>%@, webview=%x",[URL absoluteString],webView);
    [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:URL]];
}

- (void)customizeUserAgent 
{
	// produces something like:
	// Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_5; en-us) AppleWebKit/525.18 (KHTML, like Gecko) Titanium/1.0
	NSString *shortVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
	if (!shortVersion) shortVersion = @"0.0";
	NSString *version = [NSString stringWithFormat:@"Titanium/%@", shortVersion];
	[webView setApplicationNameForUserAgent:version];	
	TRACE(@"TiDocument::customizeUserAgent set to: %@ for %x",[webView applicationNameForUserAgent],self);
}

- (void)setupWebPreferences 
{
	TRACE(@"TiDocument::setupWebPreferences %x",self);
	WebPreferences *webPrefs = [WebPreferences standardPreferences];
	// This indicates that WebViews in this app will not browse multiple pages, but rather show a small number.
	// this reduces memory cache footprint significantly.
	
	// if we expect to browse a slightly larger number of documents, we might set this to WebCacheModelDocumentBrowser instead
	// that would increase memory cache footprint some tho.
	[webPrefs setCacheModel:WebCacheModelDocumentViewer];
	
	[webPrefs setPlugInsEnabled:YES]; // ?? this disallows Flash content
	[webPrefs setJavaEnabled:NO]; // ?? this disallows Java Craplets
	[webPrefs setJavaScriptEnabled:YES];
	[webView setPreferences:webPrefs];
	
	[webPrefs release];
}

- (void)show
{
	TRACE(@"TiDocument::show = %x",self);
	[[self window] makeKeyAndOrderFront:[self window]];
}

- (void)customizeWebView 
{
	TRACE(@"TiDocument::customizeWebView = %x",self);
	
	// this stuff adjusts the webview/window for chromeless windows.
	TiWindow *win = (TiWindow*)[self window];
	TiWindowConfig *o = [win config];
	
	if ([o isScrollbars])
	{
		[[[webView mainFrame] frameView] setAllowsScrolling:YES];
	}
	else
	{
		[[[webView mainFrame] frameView] setAllowsScrolling:NO];
	}
	if ([o isResizable] && [o isChrome]==NO)
	{
		[[self window] setShowsResizeIndicator:YES];
	}
	else
	{
		[[self window] setShowsResizeIndicator:NO];
	}
	
	// set the background to clear so that transparency can work
	if ([o getTransparency] <  1.0f)
	{
		[webView setBackgroundColor:[NSColor clearColor]];
	}
	
	[self setupWebPreferences];
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
	TRACE(@"TiDocument::windowControllerDidLoadNib %x",self);
	
    [super windowControllerDidLoadNib:aController];

    // Set the WebView delegates
    [webView setFrameLoadDelegate:self];
    [webView setUIDelegate:self];
    [webView setResourceLoadDelegate:self];
	
	WebBasePluginPackage* pluginPackage = [webView _pluginForMIMEType:@"application/x-gears-titanium"];
	NSLog(@"path = %@", [pluginPackage path]);


	
//	WebBasePluginPackage* pluginPackage = [webView _pluginForMIMEType:@"application/x-gears-titanium"];
//	NSLog(@"pluginDescription=%@",[pluginPackage pluginDescription]);
//	NSArray * a = [[WebPluginDatabase sharedDatabase] plugins];
//	for (int c=0;c<[a count];c++)
//	{
//		WebBasePluginPackage *p = [a objectAtIndex:c];
//		NSLog(@"plugin=%@ %@",[p name], [p pluginDescription]);
//	}
	


	// customize webview
	[self customizeWebView];
	[self customizeUserAgent];
}

- (void)close
{
	TRACE(@"TiDocument::close = %x",self);
    [webView close];
    [super close];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    return [[[webView mainFrame] dataSource] data];
}

- (BOOL)readFromURL:(NSURL *)URL ofType:(NSString *)type error:(NSError **)error
{
	[TiController error:@"readFromURL not implemented"];
    return NO;
}

- (BOOL)readFromFile:(NSString *)path ofType:(NSString *)type
{
	[TiController error:@"readFromFile not implemented"];
	return NO;
}


- (NSURL *)url
{
    return url;
}

// WebFrameLoadDelegate Methods
#pragma mark -
#pragma mark WebFrameLoadDelegate

- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) {
    }
}

- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) {
    }
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
		TRACE(@"TiDocument::didFinishLoadForFrame: %x",self);
    }
}

- (void)webView:(WebView *)sender didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
		NSString *err = [NSString stringWithFormat:@"Error loading URL: %@. %@", url,[error localizedDescription]];
		[TiController error:err];
    }
}

- (void)webView:(WebView *)sender didClearWindowObject:(WebScriptObject *)windowScriptObject forFrame:(WebFrame*)frame 
{
	TRACE(@"TiDocument::didClearWindowObject = %x",self);

	// ti is null the first time through
	// on a page reload or transition, we'll enter here again 
	// and we need to just re-use the existing object
	if (ti == nil)
	{
		ti = [[TiObject alloc] initWithWindow:[self window]];
	}
	
	// set our main ti object
	[windowScriptObject setValue:ti forKey:@"tiRuntime"];
	
	// load our main titanium JS plug
	if ([[[TiController instance] arguments] debug])
	{
		[[ti App] include:@"ti://titanium-debug.js"];
	}
	else
	{
#ifdef DEBUG
		[[ti App] include:@"ti://titanium-debug.js"];
#else
		[[ti App] include:@"ti://titanium.js"];
#endif
	}
}

// WebUIDelegate Methods
#pragma mark -
#pragma mark WebUIDelegate

- (WebView *)webView:(WebView *)sender createWebViewWithRequest:(NSURLRequest *)request
{
	[TiController error:@"createWebViewWithRequest not currently implemented"];
	return nil;
}

- (void)webViewShow:(WebView *)sender
{
	TRACE(@"TiDocument::webViewShow = %x",self);
    id myDocument = [[NSDocumentController sharedDocumentController] documentForWindow:[sender window]];
    [myDocument showWindows];
}


// WebResourceLoadDelegate Methods
#pragma mark -
#pragma mark WebResourceLoadDelegate

- (void)webViewClose:(WebView *)wv 
{
	TRACE(@"TiDocument::webViewClose = %x",self);
	[[wv window] close];
}


- (void)webViewFocus:(WebView *)wv 
{
	TRACE(@"TiDocument::webViewFocus = %x",self);
	[[wv window] makeKeyAndOrderFront:wv];
}


- (void)webViewUnfocus:(WebView *)wv 
{
	TRACE(@"TiDocument::webViewUnfocus = %x",self);
	if ([[wv window] isKeyWindow] || [[[wv window] attachedSheet] isKeyWindow]) 
	{
		[NSApp _cycleWindowsReversed:FALSE];
	}
}


- (NSResponder *)webViewFirstResponder:(WebView *)wv 
{
	return [[wv window] firstResponder];
}


- (void)webView:(WebView *)wv makeFirstResponder:(NSResponder *)responder 
{
	[[wv window] makeFirstResponder:responder];
}


- (NSString *)webViewStatusText:(WebView *)wv 
{
	return nil;
}


- (BOOL)webViewIsResizable:(WebView *)wv 
{
	return [[wv window] showsResizeIndicator];
}


- (void)webView:(WebView *)wv setResizable:(BOOL)resizable; 
{
	[[wv window] setShowsResizeIndicator:resizable];
}


- (void)webView:(WebView *)wv setFrame:(NSRect)frame 
{
	TRACE(@"TiDocument::setFrame = %x",self);
	[[wv window] setFrame:frame display:YES];
}


- (NSRect)webViewFrame:(WebView *)wv 
{
	NSWindow *window = [wv window];
	return window == nil ? NSZeroRect : [window frame];
}


- (BOOL)webViewAreToolbarsVisible:(WebView *)wv 
{
	return NO;
}


- (BOOL)webViewIsStatusBarVisible:(WebView *)wv 
{
	return NO;
}

- (id)webView:(WebView *)sender identifierForInitialRequest:(NSURLRequest *)request fromDataSource:(WebDataSource *)dataSource
{
    // Return some object that can be used to identify this resource
	TRACE(@"TiDocument::identifierForInitialRequest = %x",self);
	return nil;
}

-(NSURLRequest *)webView:(WebView *)sender resource:(id)identifier willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)redirectResponsefromDataSource:(WebDataSource *)dataSource
{
	TRACE(@"TiDocument::willSendRequest = %x",self);
    return request;
}

-(void)webView:(WebView *)sender resource:(id)identifier didFailLoadingWithError:(NSError *)error fromDataSource:(WebDataSource *)dataSource
{
}

-(void)webView:(WebView *)sender resource:(id)identifier didFinishLoadingFromDataSource:(WebDataSource *)dataSource
{
}

- (void)webView:(WebView *)wv runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame 
{
	NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
								 message,								// message
								 NSLocalizedString(@"OK", @""),			// default button
								 nil,									// alt button
								 nil);									// other button	
}


- (BOOL)webView:(WebView *)wv runJavaScriptConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame 
{
	NSInteger result = NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
													message,								// message
													NSLocalizedString(@"OK", @""),			// default button
													NSLocalizedString(@"Cancel", @""),		// alt button
													nil);
	return NSAlertDefaultReturn == result;	
}


- (void)webView:(WebView *)wv runOpenPanelForFileButtonWithResultListener:(id <WebOpenPanelResultListener>)resultListener 
{
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	[openPanel beginSheetForDirectory:nil 
								 file:nil 
					   modalForWindow:[self window]
						modalDelegate:self
					   didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) 
						  contextInfo:resultListener];	
}


- (void)openPanelDidEnd:(NSSavePanel *)openPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo 
{
	id <WebOpenPanelResultListener>resultListener = (id <WebOpenPanelResultListener>)contextInfo;
	if (NSOKButton == returnCode) {
		[resultListener chooseFilename:[openPanel filename]];
	}
}


- (BOOL)webView:(WebView *)wv shouldReplaceUploadFile:(NSString *)path usingGeneratedFilename:(NSString **)filename 
{
	return NO;
}


- (NSString *)webView:(WebView *)wv generateReplacementFile:(NSString *)path 
{
	return nil;
}


- (BOOL)webView:(WebView *)wv shouldBeginDragForElement:(NSDictionary *)element dragImage:(NSImage *)dragImage mouseDownEvent:(NSEvent *)mouseDownEvent mouseDraggedEvent:(NSEvent *)mouseDraggedEvent 
{
	return YES;
}


- (NSUInteger)webView:(WebView *)wv dragDestinationActionMaskForDraggingInfo:(id <NSDraggingInfo>)draggingInfo 
{
	return WebDragDestinationActionAny;
}


- (void)webView:(WebView *)webView willPerformDragDestinationAction:(WebDragDestinationAction)action forDraggingInfo:(id <NSDraggingInfo>)draggingInfo 
{
}


- (NSUInteger)webView:(WebView *)wv dragSourceActionMaskForPoint:(NSPoint)point
{
	return WebDragSourceActionAny;
}

@end
