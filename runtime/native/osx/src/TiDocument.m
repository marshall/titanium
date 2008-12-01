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
		childWindows = [[NSMutableArray alloc] init];
	}
	return self;
}
- (void)dealloc
{
	TRACE(@"TiDocument::dealloc =%x",self);
	[self closePrecedent];
	[userWindow destroy];
	[userWindow release];
	userWindow = nil;
	TRACE(@"TiDocument::dealloc = %x, child windows = %d",self,[childWindows count]);
	for (int c=0;c<[childWindows count];c++)
	{
		TiUserWindow *w = [childWindows objectAtIndex:c];
		[w destroy]; // destroy will call back and remove below
		w = nil;
	}
	[childWindows release];
	childWindows=nil;
	webView=nil;
    [url release];
    [super dealloc];
	
	if ([[[NSDocumentController sharedDocumentController] documents] count]==0)
	{
		TRACE(@"Last application window has closed, exiting the application");
		// once we have no more active documents, we're going to shutdown. we do this here instead of 
		// capturing applicationShouldTerminateAfterLastWindowClosed because the later will actually
		// do it when a window is hidden whereas this will ensure that the document is actually *closed*
		[NSApp terminate:self];
	}
}

- (void)addChildWindow:(TiUserWindow*)win
{
	[childWindows addObject:win];
	TRACE(@"addedChildWindow: %x (retain=%d) for %x",win,[win retainCount],self);
}

- (void)removeChildWindow:(TiUserWindow*)win
{
	[childWindows removeObject:win];
	TRACE(@"removeChildWindow: %x (retain=%d) for %x",win,[win retainCount],self);
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

- (TiUserWindow*)userWindow
{
	return userWindow;
}

- (NSString *)windowNibName
{
    return @"TiDocument";
}

- (void)loadURL:(NSURL *)URL
{
	[url release];
	url = [URL copy];
	TRACE(@"TiDocument::loadURL=>%@, webview=%x",[url absoluteString],webView);
    [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
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
	TRACE(@"TiDocument::setupWebPreferences exit %x",self);
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

	[webView setAutoresizingMask:NSViewHeightSizable | NSViewWidthSizable];
	
	[self setupWebPreferences];
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
	TRACE(@"TiDocument::windowControllerDidLoadNib %x",self);

	userWindow = [[TiUserWindow alloc] initWithWindow:[self window]];

    // Set the WebView delegates
    [webView setFrameLoadDelegate:self];
    [webView setUIDelegate:self];
    [webView setResourceLoadDelegate:self];
	[webView setPolicyDelegate:self];
	
	// customize webview
	[self customizeWebView];
	[self customizeUserAgent];
    [super windowControllerDidLoadNib:aController];
}

- (void)close
{
	TRACE(@"TiDocument::close = %x",self);

	if (webView)
	{
		[webView stopLoading:nil];
	}
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

-(void)setURL:(NSURL*)newURL
{
	TRACE(@"setURL: %@ called for %x",newURL,self);
	[url release];
	url = [newURL copy];
}

- (void)setPrecedent:(TiDocument*)doc
{
	closer = doc;
	[closer retain];
	// hide the window
	[[closer window] orderOut:nil];
	// unhook any listeners so we don't receive any more events
    [[closer webView] setFrameLoadDelegate:nil];
    [[closer webView] setUIDelegate:nil];
    [[closer webView] setResourceLoadDelegate:nil];
	[[closer webView] setPolicyDelegate:nil];
}

- (void)closePrecedent
{
	if (closer)
	{
		TRACE(@"Closing precedent doc = %x", closer);
		[[closer window] close];
		[closer release];
		closer=nil; 
	}
}

#pragma mark -
#pragma mark WebPolicyDelegate

- (void)webView:(WebView *)sender decidePolicyForNewWindowAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request newFrameName:(NSString *)frameName decisionListener:(id < WebPolicyDecisionListener >)listener
{
	[listener ignore];
}

- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary*) actionInformation request:(NSURLRequest*) request frame:(WebFrame*)frame decisionListener:(id <WebPolicyDecisionListener>)listener
{
	int type = [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue];
	
	switch (type)
	{
		case WebNavigationTypeBackForward:
		case WebNavigationTypeReload:
		{
			[listener use];
			return;
		}
		case WebNavigationTypeLinkClicked:
		case WebNavigationTypeFormSubmitted:
		case WebNavigationTypeFormResubmitted:
		{
			break;
		}
		case WebNavigationTypeOther:
		{
			break;
		}
		default:
		{
			[listener ignore];
			return;
		}
	}
		
	NSString *protocol = [[actionInformation objectForKey:WebActionOriginalURLKey] scheme]; 
	NSURL *newURL = [request URL];
	if ([newURL isEqual:url])
	{
		[listener use];
		return ;
	}
	
	if ([protocol isEqual:@"app"])
	{
		if ([[TiController instance] shouldOpenInNewWindow])
		{
			// if we're trying to open an internal page, we essentially need to always open a 
			// new document and later close the old document.  we have to do this because 
			// each document could have a different window spec.
			
			TiDocument *doc = [[TiController instance] createDocument:newURL visible:YES config:nil];
			[doc setPrecedent:self];
			
			//TODO: window opens slightly offset from current doc, make sure we 
			//get the bounds from self and set on doc
			[listener ignore];
		}
		else
		{
			// tell him to open in the same document and set our new URL
			[self setURL:newURL];
			[listener use];
		}
	}
	else if ([protocol isEqual:@"http"] || [protocol isEqual:@"https"])
	{
		// TODO: we need to probalby make this configurable to support
		// opening the URL in the system browser (code below). for now 
		// we just open inside the same frame
		//[[NSWorkspace sharedWorkspace] openURL:newURL];
		[listener use];
	}
	else
	{
		TRACE(@"Application attempted to navigate to illegal location: %@", newURL);
		[listener ignore];
	}
}

// WebFrameLoadDelegate Methods
#pragma mark -
#pragma mark WebFrameLoadDelegate

- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
    }
}

- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
		[[self window] setTitle:title];
    }
}

- (void)inject:(WebScriptObject *)windowScriptObject
{
	
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
	
	scriptCleared = YES;
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
		NSURL *theurl =[[[frame dataSource] request] URL];
		TRACE(@"TiDocument::didFinishLoadForFrame: %x for url: %@",self,theurl);

		if (!scriptCleared)
		{
			TRACE(@"page loaded with no <script> tags, manually injecting Titanium runtime", scriptCleared);
			[self inject:[frame windowObject]];
		}
		
		if (![theurl isEqual:url])
		{
			[self setURL:theurl];
		}
		[self closePrecedent];
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
	[self inject:windowScriptObject];
}

// WebUIDelegate Methods
#pragma mark -
#pragma mark WebUIDelegate

- (WebView *)webView:(WebView *)sender createWebViewWithRequest:(NSURLRequest *)request
{
	// this is called when you attempt to create a new child window from this document
	// for example using window.open
	NSURL *newurl = [request URL];
	if (newurl==nil)
	{
		// this will be null in certain cases where the browser want's to call loadURL
		// on the new webview and he will pass nil .... just open a blank document
		// and return
		newurl = [NSURL URLWithString:@"about:blank"];
	}
	TiDocument *newDoc = [[TiController instance] createDocument:newurl visible:YES config:nil];
	[newDoc setPrecedent:self];
	return [newDoc webView];
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
	// we just ignore this for now
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
