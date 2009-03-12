/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "native_window.h"
#import <Carbon/Carbon.h>
#import "../user_window.h"

@implementation NativeWindow
- (BOOL)canBecomeKeyWindow
{
	return YES;
}
- (BOOL)canBecomeMainWindow
{
	return YES;
}
- (void)setupDecorations:(WindowConfig*)cfg host:(Host*)host userwindow:(UserWindow*)uw
{
	config = cfg;
	userWindow = uw;

	[self setTitle:[NSString stringWithCString:config->GetTitle().c_str()]];
	[self setOpaque:false];
	[self setHasShadow:true];
	[self setBackgroundColor:[NSColor clearColor]];
	
	// turn on/off zoom button to control app maximize behavior
	[[self standardWindowButton:NSWindowZoomButton] setHidden:!config->IsMaximizable()];
	
	// only center if we haven't provided coordinates in setup
	if (config->GetX() < 0 || config->GetY() < 0)
	{
		[self center];
	}
	
    webView = [[WebView alloc] init];
	delegate = [[WebViewDelegate alloc] initWithWindow:self host:host];
    [self setContentView:webView];
    [self setDelegate:self];
	[self setTransparency:config->GetTransparency()];
	[self setInitialFirstResponder:webView];
	[self setShowsResizeIndicator:config->IsResizable()];

	NSMenu *windowMenu = [[[NSApp mainMenu] itemWithTitle:NSLocalizedString(@"Window",@"")] submenu];
	NSMenuItem *showInspector = [windowMenu itemWithTitle:NSLocalizedString(@"Show Inspector", @"")];

	if (host->IsDebugMode())
	{
		if (!showInspector)
		{
			[windowMenu addItem:[NSMenuItem separatorItem]];
			showInspector = [windowMenu addItemWithTitle:@"Show Inspector" action:NULL keyEquivalent:@""];
		}
	    [showInspector setEnabled:YES];
	    [showInspector setAction:@selector(showInspector)];
		[showInspector setKeyEquivalentModifierMask:NSCommandKeyMask|NSAlternateKeyMask];
		[showInspector setKeyEquivalent:@"c"];
	}
	else
	{	//While 10.5 allows for setHidden, 10.4 doesn't have such. And why hide when removing works?
		if (showInspector != nil) [windowMenu removeItem:showInspector];
	    NSMenuItem *showInspectorSep = [windowMenu itemWithTitle:@"Show Inspector Separator"];
		if (showInspectorSep != nil) [windowMenu removeItem:showInspectorSep];
	}
	closed = NO;
}
- (void)dealloc
{
	[inspector release];
	[delegate release];
	delegate = nil;
	[webView release];
	webView = nil;
	[super dealloc];
}
- (UserWindow*)userWindow
{
	return userWindow;
}
- (void)showInspector
{
	if (inspector==nil)
	{
		inspector = [[WebInspector alloc] initWithWebView:webView];
		[inspector detach:self];
	}
	[inspector show:self];
}
- (void)windowWillClose:(NSNotification *)notification
{
	KR_DUMP_LOCATION
	if (inspector)
	{
		[inspector close:self];
		[inspector release];
		inspector = nil;
	}
	config->SetVisible(false);
}
- (NSSize)windowWillResize:(NSWindow *) window toSize:(NSSize)newSize
{
	//TODO: refactor to use setMin/setMax on window
	if (config->IsResizable())
	{
		// if we're resizable, we need to resize within the constraints of the 
		// windows min/max width/height
		
		double minWidth = config->GetMinWidth();
		double maxWidth = config->GetMaxWidth();
		double minHeight = config->GetMinHeight();
		double maxHeight = config->GetMaxHeight();

		if (newSize.width >= minWidth && newSize.width <= maxWidth && 
			newSize.height >= minHeight && newSize.height <= maxHeight)
		{
			return newSize;
		}
	}
	return [window frame].size;
}
- (void)updateConfig
{
	NSRect frame = [self frame];
	config->SetWidth(frame.size.width);
	config->SetHeight(frame.size.height);
	//FIXME: so x,y but need to translate
}
- (void)windowDidResize:(NSNotification*)notification
{
	[self fireWindowEvent:RESIZED];
	[self updateConfig];
}
- (void)windowDidMove:(NSNotification*)notification
{
	[self fireWindowEvent:MOVED];
	[self updateConfig];
}
- (void)windowDidBecomeKey:(NSNotification*)notification
{
	[self fireWindowEvent:FOCUSED];
	OSXUserWindow* uw = static_cast<OSXUserWindow*>(userWindow);
	uw->Focused();
}
- (void)windowDidResignKey:(NSNotification*)notification
{
	[self fireWindowEvent:UNFOCUSED];
	OSXUserWindow* uw = static_cast<OSXUserWindow*>(userWindow);
	uw->Unfocused();
}
- (void)windowDidMiniaturize:(NSNotification*)notification
{
	[self fireWindowEvent:MINIMIZED];
}
- (void)windowDidDeminiaturize:(NSNotification*)notification
{
	[self fireWindowEvent:MAXIMIZED];
}
- (void)setTransparency:(double)transparency
{
	[self setAlphaValue:transparency];
	if (transparency < 1.0)
	{
		[webView setBackgroundColor:[NSColor clearColor]];
	}
	else
	{
		[webView setBackgroundColor:[NSColor whiteColor]];
	}
}
- (NSScreen *)activeScreen
{
    NSArray *screens = [NSScreen screens];

    /* if we've only got one screen then return it */
    if ([screens count] <= 1) 
	{
		return [NSScreen mainScreen];
	}
	
	NSScreen* screen = [self deepestScreen];
	if (screen != nil)
	{
		return screen;
	}
	
	return [NSScreen mainScreen];
}
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen 
{
	if (fullscreen)
	{
		return frameRect;
	}
   	return [super constrainFrameRect:frameRect toScreen:screen];
}

- (void)setFullScreen:(BOOL)yn
{
	if (yn)
	{
		fullscreen = YES;
		savedFrame = [self frame];
		
		// adjust to remove toolbar from view on window
		NSView *toolbarView = [[self toolbar] valueForKey:@"toolbarView"];
		float toolbarHeight = [toolbarView frame].size.height;
		if (![[self toolbar] isVisible]) {
			toolbarHeight = 0;
		}
		float windowBarHeight = [self frame].size.height - ([[self contentView] frame].size.height + toolbarHeight);
		NSRect frame = [[NSScreen mainScreen] frame];
		frame.size.height += windowBarHeight;

		SetSystemUIMode(kUIModeAllHidden,kUIOptionAutoShowMenuBar);
		[self setFrame:frame display:YES animate:YES];
		[self fireWindowEvent:FULLSCREENED];		
	    [self setShowsResizeIndicator:NO];
	}
	else
	{
		fullscreen = NO;
		[self setFrame:savedFrame display:YES animate:YES];
		SetSystemUIMode(kUIModeNormal,0);
		[self setShowsResizeIndicator:config->IsResizable()];
		[self fireWindowEvent:UNFULLSCREENED];
	}
	[self makeKeyAndOrderFront:nil];	
 	[self makeFirstResponder:webView];
}
- (WebView*)webView
{
	return webView;
}
- (WindowConfig*)config
{
	return config;
}
- (void)open
{
	if (!requiresDisplay && config->IsVisible())
	{
		// if we call open and we're initially visible
		// we need to basically set requires display which
		// will cause the window to be shown once the url is loaded
		requiresDisplay = YES;
	}
	std::string url_str = AppConfig::Instance()->InsertAppIDIntoURL(config->GetURL());
	NSURL* url = [NSURL URLWithString: [NSString stringWithCString:url_str.c_str()]];
	[[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
	[self fireWindowEvent:OPENED];
}
- (void)close
{
	if (!closed)
	{
		closed = YES;
		[self fireWindowEvent:CLOSED];
		[webView close];
		[super close];
		OSXUserWindow *uw = static_cast<OSXUserWindow*>(userWindow);
		uw->Close();
	}
}
- (void)setInitialWindow:(BOOL)yn
{
	// this is a boolean to indicate that when the frame is loaded,
	// we should go ahead and display the window
	requiresDisplay = yn;
}
- (void)frameLoaded
{
	if (requiresDisplay)
	{
		requiresDisplay = NO;
		config->SetVisible(true);
		
		[NSApp arrangeInFront:self];
		[self makeKeyAndOrderFront:self];
		[NSApp activateIgnoringOtherApps:YES];
		
		if (config->IsFullScreen())
		{
			[self setFullScreen:YES];
		}
	}
}
- (void)fireWindowEvent:(UserWindowEvent)event
{
	OSXUserWindow *uw = static_cast<OSXUserWindow*>(userWindow);
	uw->FireEvent(event);
}

@end
