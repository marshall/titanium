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
- (void)setupDecorations:(WindowConfig*)cfg host:(Host*)h userwindow:(UserWindow*)uw
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
	delegate = [[WebViewDelegate alloc] initWithWindow:self host:h];
    [self setContentView:webView];
    [self setDelegate:self];
	[self setTransparency:config->GetTransparency()];
}
- (void)dealloc
{
	KR_DUMP_LOCATION
	delete userWindow;
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
- (void)windowWillClose:(NSNotification *)notification
{
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
- (void)setFullScreen:(BOOL)yn
{
	NSMutableDictionary *options = [[[NSMutableDictionary alloc] init] autorelease];
	if (yn)
	{
		NSNumber *value = [NSNumber numberWithInt:NSNormalWindowLevel];
		[options setValue:value forKey:NSFullScreenModeWindowLevel];
		NSScreen *screen = [NSScreen mainScreen];
		[webView enterFullScreenMode:screen withOptions:options];
	}
	else
	{
		[webView exitFullScreenModeWithOptions:options];
	}
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
	NSURL *url = [NSURL URLWithString:[NSString stringWithCString:config->GetURL().c_str()]];
    [[webView mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
}
- (void)close
{
	[super close];
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
	    [self makeKeyAndOrderFront:self];	
		[NSApp arrangeInFront:self];
		if (config->IsFullScreen())
		{
			[self setFullScreen:YES];
		}
	}
}
@end
