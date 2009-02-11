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

	if (config->IsFullScreen())
	{
		[self setFullScreen:YES];
	}
	
	userWindow = new SharedBoundObject(uw);

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

	NSMenu *windowMenu = [[[NSApp mainMenu] itemWithTitle:NSLocalizedString(@"Window",@"")] submenu];
	NSMenuItem *showInspector = [windowMenu itemWithTitle:NSLocalizedString(@"Show Inspector", @"")];
	// if (host->IsDebugMode())
	// {
	    [showInspector setEnabled:YES];
	    [showInspector setAction:@selector(showInspector)];
	// }
	// else
	// {
	//     [showInspector setHidden:YES];
	//     NSMenuItem *showInspectorSep = [windowMenu itemWithTitle:@"Show Inspector Separator"];
	//     [showInspectorSep setHidden:YES];
	// }
	closed = NO;
}
- (void)dealloc
{
	KR_DUMP_LOCATION
	delete userWindow;
	[inspector release];
	[delegate release];
	delegate = nil;
	[webView release];
	webView = nil;
	[super dealloc];
}
- (SharedBoundObject)userWindow
{
	return *userWindow;
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
	[self updateConfig];
}
- (void)windowDidMove:(NSNotification*)notification
{
	[self updateConfig];
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
- (void)fadeOut
{
    CGAcquireDisplayFadeReservation(1.0, &tok);
    CGDisplayFade(tok, 0.5, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0, 0, 0, TRUE);
}

- (void)fadeIn
{
    CGDisplayFade(tok, 1, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal, 0, 0, 0, TRUE);
    CGReleaseDisplayFadeReservation(tok);
}
- (void)setFullScreen:(BOOL)yn
{
	NSMutableDictionary *options = [NSMutableDictionary dictionaryWithObjectsAndKeys:
	                            [NSNumber numberWithBool:NO], NSFullScreenModeAllScreens, nil];
	
	[self fadeOut];
	
	if (yn)
	{
		// figure out which screen to display on
		NSScreen *screen = [self activeScreen];
		[webView enterFullScreenMode:screen withOptions:options];
	}
	else
	{
		// we hide and then later rehide so that the shadow
		// on the window will actually refresh correctly
		[self orderOut:nil];
		[webView exitFullScreenModeWithOptions:options];
	}
	[self makeKeyAndOrderFront:self];	
 	[self makeFirstResponder:webView];

	[self fadeIn];
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
	NSURL *url = [TiApplication normalizeURL:[NSString stringWithCString:config->GetURL().c_str()]];
    [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
}
- (void)close
{
	KR_DUMP_LOCATION
	if (!closed)
	{
		closed = YES;
		[webView close];
		[super close];
		SharedPtr<UserWindow> uw = userWindow->cast<UserWindow>();
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
@end
