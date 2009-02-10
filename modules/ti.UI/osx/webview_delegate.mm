/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"

#define TRACE  NSLog

@interface NSApplication (DeclarationStolenFromAppKit)
- (void)_cycleWindowsReversed:(BOOL)reversed;
@end

@implementation WebViewDelegate

- (void)setup 
{
	AppConfig *appConfig = AppConfig::Instance();
	std::string appid = appConfig->GetAppID();
	NSString *appID = [NSString stringWithCString:appid.c_str()];
	
	[webView setPreferencesIdentifier:appID];
	
	WebPreferences *webPrefs = [[WebPreferences alloc] initWithIdentifier:appID];
	// This indicates that WebViews in this app will not browse multiple pages, but rather show a small number.
	// this reduces memory cache footprint significantly.

	[webPrefs setCacheModel:WebCacheModelDocumentBrowser];
	[webPrefs setDeveloperExtrasEnabled:host->IsDebugMode()];
	[webPrefs setPlugInsEnabled:YES]; 
	[webPrefs setJavaEnabled:NO]; // ?? this disallows Java Craplets
	[webPrefs setJavaScriptEnabled:YES];
	if ([webPrefs respondsToSelector:@selector(setDatabasesEnabled:)])
	{
		[webPrefs setDatabasesEnabled:YES];
	}
	if ([webPrefs respondsToSelector:@selector(setLocalStorageEnabled:)])
	{
		[webPrefs setLocalStorageEnabled:YES];
	}
	[webPrefs setDOMPasteAllowed:YES];
	
	// Setup the DB to store it's DB under our data directory for the app
	NSString *datadir = [NSString stringWithCString:kroll::FileUtils::GetApplicationDataDirectory(appid).c_str()];
	[webPrefs _setLocalStorageDatabasePath:datadir];
	
	//TODO: make sure this is OK
	[webPrefs setFullDocumentTeardownEnabled:YES];

	[webView setPreferences:webPrefs];

	[webPrefs release];

	// this stuff adjusts the webview/window for chromeless windows.
	WindowConfig *o = [window config];
	
	if (o->IsUsingScrollbars())
	{
		[[[webView mainFrame] frameView] setAllowsScrolling:YES];
	}
	else
	{
		[[[webView mainFrame] frameView] setAllowsScrolling:NO];
	}
	if (o->IsResizable() && !o->IsUsingChrome())
	{
		[window setShowsResizeIndicator:YES];
	}
	else
	{
		[window setShowsResizeIndicator:NO];
	}
	
	[webView setAutoresizingMask:NSViewHeightSizable | NSViewWidthSizable];
	[webView setShouldCloseWithWindow:NO];
}

-(id)initWithWindow:(NativeWindow*)win host:(Host*)h
{
	self = [super init];
	if (self!=nil)
	{
		window = win;
		host = h;
		webView = [window webView];
		[self setup];
		[webView setFrameLoadDelegate:self];
		[webView setUIDelegate:self];
		[webView setResourceLoadDelegate:self];
		[webView setPolicyDelegate:self];
		[webView setScriptDebugDelegate:self];
		
		SharedBoundObject global = host->GetGlobalObject();
		double version = global->Get("version")->ToDouble();
		NSString *useragent = [NSString stringWithFormat:@"%s/%0.2f",PRODUCT_NAME,version];
		[webView setApplicationNameForUserAgent:useragent];
	}
	return self;
}

-(void)dealloc
{
	KR_DUMP_LOCATION
	
	// just for safety, remove our main JS key
	NSString *tiKey = [NSString stringWithCString:GLOBAL_NS_VARNAME];
	[windowJS removeWebScriptKey:tiKey];
	
	[url release];
	[super dealloc];
}

-(void)show
{
	WindowConfig *config = [window config];
	config->SetVisible(true);
    [window makeKeyAndOrderFront:nil];	
}

-(void)closePrecedent
{
	//TODO:
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

-(DOMElement*)findAnchor:(DOMNode*)node
{
	while (node)
	{
		if ([node nodeType] == 1 && [[node nodeName] isEqualToString:@"A"])
		{
			return (DOMElement*)node;
		}
		node = [node parentNode];
	}
	return nil;
}

-(BOOL)newWindowAction:(NSDictionary*)actionInformation request:(NSURLRequest*)request listener:(id < WebPolicyDecisionListener >)listener
{
	NSDictionary* elementDick = [actionInformation objectForKey:WebActionElementKey];
#ifdef DEBUG	
	for (id key in elementDick)
	{
		NSLog(@"window action - key = %@",key);
	}
#endif 
	DOMNode *target = [elementDick objectForKey:WebElementDOMNodeKey];
	DOMElement *anchor = [self findAnchor:target];
	
	TRACE(@"newWindowAction target=%@, anchor=%@",target,anchor);
	
	if (anchor)
	{
		NSString *target = [anchor getAttribute:@"target"];
		if (target)
		{
			if ([target isEqualToString:@"ti:systembrowser"])
			{
				NSURL *newURL = [request URL];
				[[NSWorkspace sharedWorkspace] openURL:newURL];
				[listener ignore];
				return NO;
			}
		}
	}

	NSString *protocol = [[actionInformation objectForKey:WebActionOriginalURLKey] scheme]; 
	NSURL *newURL = [request URL];
	if ([newURL isEqual:url])
	{
		[listener use];
		return NO;
	}
	
	if ([protocol isEqual:@"app"])
	{
		
		// if ([[TiController instance] shouldOpenInNewWindow])
		// {
		// 	// if we're trying to open an internal page, we essentially need to always open a 
		// 	// new document and later close the old document.  we have to do this because 
		// 	// each document could have a different window spec.
		// 	
		// 	TiDocument *doc = [[TiController instance] createDocument:newURL visible:YES config:nil];
		// 	[doc setPrecedent:self];
		// 	
		// 	//TODO: window opens slightly offset from current doc, make sure we 
		// 	//get the bounds from self and set on doc
		// 	[listener ignore];
		// }
		// else
		// {
		// 	// tell him to open in the same document and set our new URL
		// 	[self setURL:newURL];
		// 	[listener use];
		// }
		[self setURL:newURL];
		[listener use];
	}
	else if ([protocol isEqual:@"http"] || [protocol isEqual:@"https"])
	{
		// TODO: we need to probalby make this configurable to support
		// opening the URL in the system browser (code below). for now 
		// we just open inside the same frame
		//[[NSWorkspace sharedWorkspace] openURL:newURL];
		[listener use];
	}
	return YES;
}

#pragma mark -
#pragma mark WebPolicyDelegate

- (void)webView:(WebView *)sender decidePolicyForNewWindowAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request newFrameName:(NSString *)frameName decisionListener:(id < WebPolicyDecisionListener >)listener
{
	if (NO == [self newWindowAction:actionInformation request:request listener:listener])
	{
		return;
	}
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
		TRACE(@"Attempting to navigate to the same URL: %@",newURL);
		[listener use];
		return ;
	}
	
	if ([protocol isEqual:@"app"])
	{
		// we only care about loading new TiDocuments if this is the main frame,
		// otherwise we're an internal frame of some kind
		if (frame != [[frame webView] mainFrame]) {
			[listener use];
			[self setURL:newURL];
			return;
		}
		
		// if ([[TiController instance] shouldOpenInNewWindow])
		// {
		// 	// if we're trying to open an internal page, we essentially need to always open a 
		// 	// new document and later close the old document.  we have to do this because 
		// 	// each document could have a different window spec.
		// 	
		// 	TiDocument *doc = [[TiController instance] createDocument:newURL visible:YES config:nil];
		// 	[doc setPrecedent:self];
		// 	
		// 	//TODO: window opens slightly offset from current doc, make sure we 
		// 	//get the bounds from self and set on doc
		// 	[listener ignore];
		// }
		// else
		// {
		// 	// tell him to open in the same document and set our new URL
		// 	[self setURL:newURL];
		// 	[listener use];
		// }
		[self setURL:newURL];
		[listener use];
	}
	else if ([protocol isEqual:@"http"] || [protocol isEqual:@"https"])
	{
		if (NO == [self newWindowAction:actionInformation request:request listener:listener])
		{
			return;
		}
		
		[self setURL:newURL];
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
		scriptCleared = NO;
    }
}

- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
		// set the title on the config in case they
		// programatically set the title on the window
		// so that it correctly is reflected in the config 
		WindowConfig *config = [window config];
		std::string t = std::string([title UTF8String]);
		config->SetTitle(t);
		[window setTitle:title];
    }
}
- (void)inject:(WebScriptObject *)windowScriptObject context:(JSGlobalContextRef)context
{
	TRACE(@"inject called");
	
	JSContextGroupRef group = JSContextGetGroup(context);
	KJSUtil::RegisterGlobalContext(group, context);

	// Produce a delegating object to represent the top-level
	// Titanium object. When a property isn't found in this object
	// it will look for it in global_tibo.
	SharedBoundObject global_tibo = host->GetGlobalObject();
	BoundObject* ti_object = new DelegateStaticBoundObject(global_tibo);
	SharedBoundObject shared_ti_obj = SharedBoundObject(ti_object);

	SharedValue ui_api_value = ti_object->Get("UI");
	if (ui_api_value->IsObject())
	{
		// Create a delegate object for the UI API.
		SharedBoundObject ui_api = ui_api_value->ToObject();
		BoundObject* delegate_ui_api = new DelegateStaticBoundObject(ui_api);

		// Place currentWindow in the delegate.
		SharedBoundObject shared_user_window = [window userWindow];
		SharedValue user_window_val = Value::NewObject(shared_user_window);
		delegate_ui_api->Set("currentWindow", user_window_val);

		// Place currentWindow.createWindow in the delegate.
		SharedValue create_window_value = shared_user_window->Get("createWindow");
		delegate_ui_api->Set("createWindow", create_window_value);

		ti_object->Set("UI", Value::NewObject(delegate_ui_api));
	}
	else
	{
		std::cerr << "Could not find UI API point!" << std::endl;
	}

	// Place the Titanium object into the window's global object
	JSObjectRef global_object = JSContextGetGlobalObject(context);
	BoundObject *global_bound_object = new KJSBoundObject(context, global_object);
	SharedValue ti_object_value = Value::NewObject(shared_ti_obj);
	global_bound_object->Set(GLOBAL_NS_VARNAME, ti_object_value);
	
	// we need to place window and document into our shared global
	SharedBoundObject shared_global_bound_object = global_bound_object;
	SharedValue win_object_value = Value::NewObject(shared_global_bound_object);
	ti_object->Set("window",win_object_value);
	
	SharedValue doc_value = global_bound_object->Get("document");
	ti_object->Set("document",doc_value);
	
	scriptCleared = YES;
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
		NSURL *theurl =[[[frame dataSource] request] URL];
		TRACE(@"webview_delegate::didFinishLoadForFrame: %x for url: %@",self,theurl);

		if (!scriptCleared)
		{
			TRACE(@"page loaded with no <script> tags, manually injecting Titanium runtime", scriptCleared);
			JSGlobalContextRef context = [frame globalContext];
			[self inject:[frame windowObject] context:context];
		}
		
		if (![theurl isEqual:url])
		{
			[self setURL:theurl];
		}
		[self closePrecedent];
		
		if (initialDisplay==NO)
		{
			initialDisplay=YES;
			// cause the initial window to show since it was initially opened hidden
			// so you don't get the nasty wide screen while content is loading
			[window performSelector:@selector(frameLoaded) withObject:nil afterDelay:.005];
		}
    }
}

- (void)webView:(WebView *)sender didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
    // Only report feedback for the main frame.
    if (frame == [sender mainFrame]) 
	{
		if ([error code]==-999 && [[error domain] isEqual:NSURLErrorDomain])
		{
			//this is OK, this is a cancel to a pending web load request and can be ignored...
			return;
		}
		NSString *err = [NSString stringWithFormat:@"Error loading URL: %@. %@", url,[error localizedDescription]];
		TRACE(@"error: %@",err);
		
		// in this case we need to ensure that the window is showing if not initially shown
		if (initialDisplay==NO)
		{
			initialDisplay=YES;
			[window performSelector:@selector(frameLoaded) withObject:nil afterDelay:.005];
		}
    }
}

- (void)webView:(WebView *)sender didClearWindowObject:(WebScriptObject *)windowScriptObject forFrame:(WebFrame*)frame 
{
	JSGlobalContextRef context = [frame globalContext];
	[self inject:windowScriptObject context:context];
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
	// TiDocument *newDoc = [[TiController instance] createDocument:newurl visible:YES config:nil];
	// [newDoc setPrecedent:self];
	// return [newDoc webView];
	return nil;
}

- (void)webViewShow:(WebView *)sender
{
	KR_DUMP_LOCATION
	TRACE(@"webview_delegate::webViewShow = %x",self);
	//TODO: so we need to deal with this at all?
}


- (void)webViewClose:(WebView *)wv 
{
	KR_DUMP_LOCATION
	[[wv window] close];
	WindowConfig *config = [window config];
	config->SetVisible(NO);
	
	if (inspector)
	{
		[inspector webViewClosed];
	}
}


- (void)webViewFocus:(WebView *)wv 
{
	KR_DUMP_LOCATION
	// [[TiController instance] activate:self];
	[[wv window] makeKeyAndOrderFront:wv];
}


- (void)webViewUnfocus:(WebView *)wv 
{
	KR_DUMP_LOCATION
	if ([[wv window] isKeyWindow] || [[[wv window] attachedSheet] isKeyWindow]) 
	{
		[NSApp _cycleWindowsReversed:FALSE];
	}
	// [[TiController instance] deactivate:self];
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
	WindowConfig *config = [window config];
	return config->IsResizable();
}


- (void)webView:(WebView *)wv setResizable:(BOOL)resizable; 
{
	WindowConfig *config = [window config];
	config->SetResizable(resizable);
	[[wv window] setShowsResizeIndicator:resizable];
}


- (void)webView:(WebView *)wv setFrame:(NSRect)frame 
{
	TRACE(@"webview_delegate::setFrame = %x",self);
	[[wv window] setFrame:frame display:YES];
}


- (NSRect)webViewFrame:(WebView *)wv 
{
	NSWindow *w = [wv window];
	return w == nil ? NSZeroRect : [w frame];
}


- (BOOL)webViewAreToolbarsVisible:(WebView *)wv 
{
	return NO;
}


- (BOOL)webViewIsStatusBarVisible:(WebView *)wv 
{
	return NO;
}

// WebResourceLoadDelegate Methods
#pragma mark -
#pragma mark WebResourceLoadDelegate

- (id)webView:(WebView *)sender identifierForInitialRequest:(NSURLRequest *)request fromDataSource:(WebDataSource *)dataSource
{
    // Return some object that can be used to identify this resource
	// we just ignore this for now
	return nil;
}

-(NSURLRequest *)webView:(WebView *)sender resource:(id)identifier willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)redirectResponsefromDataSource:(WebDataSource *)dataSource
{
	TRACE(@"webview_delegate::willSendRequest = %x",self);
    return request;
}

-(void)webView:(WebView *)sender resource:(id)identifier didFailLoadingWithError:(NSError *)error fromDataSource:(WebDataSource *)dataSource
{
	TRACE(@"webview_delegate::didFailLoadingWithError = %@", [error localizedDescription]);
}

-(void)webView:(WebView *)sender resource:(id)identifier didFinishLoadingFromDataSource:(WebDataSource *)dataSource
{
}

- (void)webView:(WebView *)wv runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame 
{
	TRACE(@"alert = %@",message);
	
	NSRunInformationalAlertPanel([window title],	// title
								 message,								// message
								 NSLocalizedString(@"OK", @""),			// default button
								 nil,									// alt button
								 nil);									// other button	
}


- (BOOL)webView:(WebView *)wv runJavaScriptConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame 
{
	NSInteger result = NSRunInformationalAlertPanel([window title],	// title
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
					   modalForWindow:window
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
	NSLog(@"generateReplacementFile: %@",path);
	return nil;
}


- (BOOL)webView:(WebView *)wv shouldBeginDragForElement:(NSDictionary *)element dragImage:(NSImage *)dragImage mouseDownEvent:(NSEvent *)mouseDownEvent mouseDraggedEvent:(NSEvent *)mouseDraggedEvent 
{
	NSLog(@"shouldBeginDragForElement");
	return YES;
}


- (NSUInteger)webView:(WebView *)wv dragDestinationActionMaskForDraggingInfo:(id <NSDraggingInfo>)draggingInfo 
{
	NSLog(@"dragDestinationActionMaskForDraggingInfo");
	return WebDragDestinationActionAny;
}


- (void)webView:(WebView *)webView willPerformDragDestinationAction:(WebDragDestinationAction)action forDraggingInfo:(id <NSDraggingInfo>)draggingInfo 
{
	NSLog(@"willPerformDragDestinationAction: %d",action);
	NSPasteboard *pasteboard = [draggingInfo draggingPasteboard];
	NSLog(@"pasteboard types: %@",[pasteboard types]);
}


- (NSUInteger)webView:(WebView *)wv dragSourceActionMaskForPoint:(NSPoint)point
{
	NSLog(@"dragSourceActionMaskForPoint");
	return WebDragSourceActionAny;
}

#pragma mark -
#pragma mark WebScriptDebugDelegate

// some source was parsed, establishing a "source ID" (>= 0) for future reference
- (void)webView:(WebView *)webView       didParseSource:(NSString *)source
 baseLineNumber:(NSUInteger)lineNumber
		fromURL:(NSURL *)aurl
	   sourceId:(int)sid
	forWebFrame:(WebFrame *)webFrame
{
	TRACE(@"loading javascript from %@ with sid: %d",[aurl absoluteURL],sid);
	// NSString *key = [NSString stringWithFormat:@"%d",sid];
	// NSString *value = [NSString stringWithFormat:@"%@",(aurl==nil? @"<main doc>" : aurl)];
	// //TODO: trim off app://<aid>/
	// [javascripts setObject:value forKey:key];
}

// some source failed to parse
- (void)webView:(WebView *)webView  failedToParseSource:(NSString *)source
 baseLineNumber:(NSUInteger)lineNumber
		fromURL:(NSURL *)theurl
	  withError:(NSError *)error
	forWebFrame:(WebFrame *)webFrame
{
	TRACE(@"failed to parse javascript from %@ at lineNumber: %d, error: %@",[theurl absoluteURL],lineNumber,[error localizedDescription]);
}

// just entered a stack frame (i.e. called a function, or started global scope)
- (void)webView:(WebView *)webView    didEnterCallFrame:(WebScriptCallFrame *)frame
	   sourceId:(int)sid
		   line:(int)lineno
	forWebFrame:(WebFrame *)webFrame
{
}

// about to execute some code
- (void)webView:(WebView *)webView willExecuteStatement:(WebScriptCallFrame *)frame
	   sourceId:(int)sid
		   line:(int)lineno
	forWebFrame:(WebFrame *)webFrame
{
	// NOTE: this is very chatty and prints out each line as it's being executed
	//	TRACE(@"executing javascript lineNumber: %d",lineno);
}

// about to leave a stack frame (i.e. return from a function)
- (void)webView:(WebView *)webView   willLeaveCallFrame:(WebScriptCallFrame *)frame
	   sourceId:(int)sid
		   line:(int)lineno
	forWebFrame:(WebFrame *)webFrame
{
}

// exception is being thrown
- (void)webView:(WebView *)webView   exceptionWasRaised:(WebScriptCallFrame *)frame
	   sourceId:(int)sid
		   line:(int)lineno
	forWebFrame:(WebFrame *)webFrame
{
	// NSString *key = [NSString stringWithFormat:@"%d",sid];
	// for (id akey in javascripts)
	// {
	// 	if ([key isEqualToString:akey])
	// 	{
	// 		NSString *aurl = [javascripts objectForKey:akey];
	// 		TRACE(@"raising javascript exception at lineNumber: %d in %@ (%d)",lineno,aurl,sid);
	// 		break;
	// 	}
	// }
}

// return whether or not quota has been reached for a db (enabling db support)
- (void)webView:(WebView*)webView	frame:(WebFrame*)frame
	exceededDatabaseQuotaForSecurityOrigin:(id)origin
	database:(NSString*)dbName
{
	const unsigned long long defaultQuota = 5 * 1024 * 1024;
	
	[origin performSelector:@selector(setQuota:) withObject:[NSNumber numberWithInt:defaultQuota]];
}
@end


