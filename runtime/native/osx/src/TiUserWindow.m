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
#import "TiUserWindow.h"
#import "TiDocument.h"
#import "TiController.h"
 
@implementation TiUserWindow

- (id)initWithWebview:(WebView*)wv
{
	self = [super init];
	TRACE(@"TiUserWindow::initWithWebview = %x",self);
	if (self != nil)
	{
		window = nil; // don't retain
		webView = wv;  // don't retain
		pending = [[TiWindowConfig alloc] init];
	}
	return self;
}

- (id) initWithWindow:(TiWindow *)win
{
	self = [super init];
	TRACE(@"TiUserWindow::initWithWindow = %x",self);
	if (self != nil)
	{
		// don't retain these
		window = win; 
		webView = [TiController getWebView:window];
	}
	return self;
}

- (void)dealloc 
{
	TRACE(@"TiUserWindow::dealloc = %x, parent = %x",self,parent);
	[pending release];
	pending = nil;
	[doc release];
	doc = nil;
	if (parent)
	{
		[parent removeChildWindow:self];
		parent = nil;
	}
	[pendingHTML release];
	pendingHTML = nil;
	webView = nil;
	window = nil;
	[super dealloc];
}

- (void)setFactory:(TiWindowFactory*)afactory
{
	factory = afactory;
}

- (NSString *)description 
{
	return @"[TiUserWindow native]";
}

- (BOOL) hasWindow
{
	return (window != nil);
}

- (void)setContent:(NSString*)content
{
	if ([self hasWindow])
	{
		[[webView mainFrame] loadHTMLString:content baseURL:nil];
	}
	else
	{
		[pendingHTML release];
		pendingHTML = [content copy];
	}
}

- (NSString*)getContent
{
	if ([self hasWindow])
	{
		NSData *data = [[[webView mainFrame] dataSource] data];
		if (data==nil)
		{
			return nil;
		}
		int length = [data length];
		char *b = malloc(sizeof(char)*length);
		[data getBytes:b length:length];
		NSString *copy = [NSString stringWithCString:b length:length];
		free(b);
		b=nil;
		return copy;
	}
	else
	{
		return pendingHTML;
	}
}

- (NSString*)getTitle 
{
	if ([self hasWindow])
	{
		return [window title];
	}
	return [pending getTitle];
}

- (void)setTitle:(NSString*)title
{
	if ([self hasWindow])
	{
		[window setTitle:title];
	}
	else
	{
		[pending setTitle:title];
	}
}

- (CGFloat)getTransparency
{
	if ([self hasWindow])
	{
		return [window alphaValue];
	}
	return [pending getTransparency];
}

- (void)setTransparency:(CGFloat)alphaValue
{
	if ([self hasWindow])
	{
		[window setAlphaValue:alphaValue];
	}
	else
	{
		[pending setTransparency:alphaValue];
	}
}

- (void)open
{
	if (window!=nil)
	{
		[[webView windowScriptObject] setException:@"window has already been opened"];
		return;
	}
	TRACE(@"TiUserWindow::open called %x, retain count=%d",self,[self retainCount]);
	
	TRACE(@"Open URL: %@",[pending getURL]);
	NSString *url = [[pending getURL] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
	//NOTE: do we need to do this for others???
	TRACE(@"URL scrubbed: %@",url);
	url = [url stringByReplacingOccurrencesOfString:@"%" withString:@"%22"];
	NSURL *theurl = [NSURL URLWithString:url];
	
	if (theurl==nil)
	{
		// invalid URL!
		TRACE(@"Invalid URL used for user window: %@",url);
		[[webView windowScriptObject] setException:@"Invalid URL passed to Window. Makes sure the URL conforms to RFC 2396"];
		return;
	}
	
	doc = [[TiController instance] createDocument:theurl visible:NO config:pending];
	window = [doc window];
	if (![pending isVisible])
	{
		[self hide:NO];
	}
	webView = [doc webView];
	if ([pending getX]>=0 || [pending getY]>=0)
	{
		[[webView windowScriptObject] evaluateWebScript:[NSString stringWithFormat:@"moveTo(%f,%f)",[pending getX],[pending getY]]];
	}
	[doc loadURL:theurl];
}

- (void)setParent:(TiDocument*)p
{
	TRACE(@"TiUserWindow::setParent for %x called with %x", self, p);
	parent = p; // don't retain
}

- (void)destroy
{
	TRACE(@"TiUserWindow::destroy %x", self);
	if ([self hasWindow])
	{
		[self close];
	}
}

- (void)close 
{
	TRACE(@"TiUserWindow::close %x", self);
	[factory removeWindow:self];
	if (parent)
	{
		[parent removeChildWindow:self];
		parent = nil;
	}
	if ([self hasWindow])
	{
		[[TiController getDocument:window] close];
	}
	else
	{
		[[webView windowScriptObject] setException:@"cannot close a window that hasn't been opened"];
	}
}

- (void)hide:(BOOL)animate
{
	if ([self hasWindow])
	{
		TRACE(@"hide called with: %d",animate);
		if (animate)
		{
			// let the JS layer do the animation
			WebScriptObject* scope = [[webView windowScriptObject] evaluateWebScript:@"ti.Extras"];
			if (scope == nil)
			{
				scope = [[[parent webView] windowScriptObject] evaluateWebScript:@"ti.Extras"];
			}
			[scope callWebScriptMethod:@"fadeOutWindow" withArguments:[NSArray arrayWithObject:self]];
		}
		else
		{
			[window orderOut:nil]; // to hide it
		}
	}
	else
	{
		[[webView windowScriptObject] setException:@"cannot hide a window that hasn't been opened"];
	}
}

- (void)show:(BOOL)animate 
{
	if ([self hasWindow])
	{
		if (animate)
		{
			// let the JS layer do the animation
			WebScriptObject* scope = [[webView windowScriptObject] evaluateWebScript:@"ti.Extras"];
			[scope callWebScriptMethod:@"fadeInWindow" withArguments:[NSArray arrayWithObject:self]];
		}
		[NSApp arrangeInFront:window];
		[window makeKeyAndOrderFront:window]; 
		[NSApp activateIgnoringOtherApps:YES];
	}
	else
	{
		[[webView windowScriptObject] setException:@"cannot show a window that hasn't been opened"];
	}
}

- (void)activate
{
	if ([self hasWindow])
	{		
	}
	else
	{
		[[webView windowScriptObject] setException:@"cannot activate a window that hasn't been opened"];
	}
}

- (void)minimize
{
	if ([self hasWindow])
	{		
	}
	else
	{
		[[webView windowScriptObject] setException:@"cannot minimize a window that hasn't been opened"];
	}
}

- (void)maximize
{
	if ([self hasWindow])
	{		
	}
	else
	{
		[[webView windowScriptObject] setException:@"cannot maximize a window that hasn't been opened"];
	}
}

-(BOOL)isUsingChrome
{
	if ([self hasWindow])
	{
		return [[window config] isChrome];
	}
	else
	{
		return [pending isChrome];
	}
}

- (void)setUsingChrome:(BOOL)yn
{
	if ([self hasWindow])
	{
	}
	else
	{
		[pending setChrome:yn];
	}
}

- (BOOL)isUsingScrollbars
{
	if ([self hasWindow])
	{
		return [[window config] isScrollbars];
	}
	else
	{
		return [pending isScrollbars];
	}
}

- (void)setUsingScrollbars:(BOOL)yn
{
	if ([self hasWindow])
	{
	}
	else
	{
		[pending setScrollbars:yn];
	}
}


- (BOOL)isFullscreen
{
	if ([self hasWindow])
	{
		return [[window config] isFullscreen];
	}
	else
	{
		return [pending isFullscreen];
	}
}

- (void)setFullscreen:(BOOL)yn
{
	if ([self hasWindow])
	{
	}
	else
	{
		[pending setFullscreen:yn];
	}
}

- (NSString*)getURL
{
	if ([self hasWindow])
	{
		return [[window config] getURL];
	}
	else
	{
		return [pending getURL];
	}
}

- (void)setURL:(NSString *)url
{
	if ([self hasWindow])
	{
		TiDocument *adoc = [TiController getDocument:window];
		[[window config] setURL:url];
		[adoc loadURL:[NSURL URLWithString:url]];
	}
	else
	{
		[pending setURL:url];
	}
}

- (NSString*)getID
{
	if ([self hasWindow])
	{
		return [[window config] getID];
	}
	else
	{
		return [pending getID];
	}
}

- (void)setID:(NSString*)anid
{
	[pending setID:anid]; // can only be called by the factory, not by app
}

- (TiBounds*)getBounds
{
	TiBounds* bounds = [[TiBounds alloc] init];
	bounds.x = [self getX];
	bounds.y = [self getY];
	bounds.width = [self getWidth];
	bounds.height = [self getHeight];
	return bounds;
}

- (void)setBounds:(TiBounds*)bounds
{
	[self setX:[bounds x]];
	[self setY:[bounds y]];
	[self setWidth:[bounds width]];
	[self setHeight:[bounds height]];
}


- (CGFloat)getX
{
	if ([self hasWindow])
	{
		NSString *v = [[webView windowScriptObject] evaluateWebScript:@"screenX"];
		return v == nil ? 0 : [v floatValue];
	}
	else
	{
		return [pending getX];
	}
}

- (void)setX:(CGFloat)newx
{
	if ([self hasWindow])
	{
		[[webView windowScriptObject] evaluateWebScript:[NSString stringWithFormat:@"moveTo(%f,screenY)",newx]];
	}
	else
	{
		[pending setX:newx];
	}
}

- (CGFloat)getY
{
	if ([self hasWindow])
	{
		NSString *v = [[webView windowScriptObject] evaluateWebScript:@"screenY"] ;
		return v == nil ? 0 : [v floatValue];
	}
	else
	{
		return [pending getY];
	}
}

- (void)setY:(CGFloat)newy
{
	if ([self hasWindow])
	{
		[[webView windowScriptObject] evaluateWebScript:[NSString stringWithFormat:@"moveTo(screenX,%f)",newy]];
	}
	else
	{
		[pending setY:newy];
	}
}

- (CGFloat)getWidth
{
	if ([self hasWindow])
	{
		NSString *v = [[webView windowScriptObject] evaluateWebScript:@"outerWidth"];
		return v == nil ? 0 : [v floatValue];
	}
	else
	{
		return [pending getWidth];
	}
}

- (void)setWidth:(CGFloat)width
{
	if ([self hasWindow])
	{
		[[webView windowScriptObject] evaluateWebScript:[NSString stringWithFormat:@"resizeTo(%f,outerHeight)",width]];
	}
	else
	{
		[pending setWidth:width];
	}
}

- (CGFloat)getHeight
{
	if ([self hasWindow])
	{
		NSString *v = [[webView windowScriptObject] evaluateWebScript:@"outerHeight"];
		return v == nil ? 0 : [v floatValue];
	}
	else
	{
		return [pending getHeight];
	}
}

- (void)setHeight:(CGFloat)height
{
	if ([self hasWindow])
	{
		[[webView windowScriptObject] evaluateWebScript:[NSString stringWithFormat:@"resizeTo(outerWidth,%f)",height]];
	}
	else
	{
		return [pending setHeight:height];
	}
}

- (BOOL)isResizable
{
	if ([self hasWindow])
	{
		return [[window config] isResizable];
	}
	else
	{
		return [pending isResizable];
	}
}

- (void)setResizable:(BOOL)yn
{
	if ([self hasWindow])
	{
		//FIXME: do we need to do anything else??
		[[window config] setResizable:yn];
	}
	else
	{
		[pending setResizable:yn];
	}
}

- (BOOL)isMaximizable
{
	if ([self hasWindow])
	{
		return [[window config] isMaximizable];
	}
	else
	{
		return [pending isMaximizable];
	}
}

- (void)setMaximizable:(BOOL)yn
{
	if ([self hasWindow])
	{
		[[window config] setMaximizable:yn];
		[[window standardWindowButton:NSWindowZoomButton] setHidden:yn==NO];
	}
	else
	{
		[pending setMaximizable:yn];
	}
}

- (BOOL)isMinimizable
{
	if ([self hasWindow])
	{
		return [[window config] isMinimizable];
	}
	else
	{
		return [pending isMinimizable];
	}
}

- (void)setMinimizable:(BOOL)yn
{
	if ([self hasWindow])
	{
		[[window config] setMinimizable:yn];
		[[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:yn==NO];
	}
	else
	{
		[pending setMinimizable:yn];
	}
}

- (BOOL)isCloseable
{
	if ([self hasWindow])
	{
		return [[window config] isCloseable];
	}
	else
	{
		return [pending isCloseable];
	}
}

- (void)setCloseable:(BOOL)yn
{
	if ([self hasWindow])
	{
		[[window config] setCloseable:yn];
		[[window standardWindowButton:NSWindowCloseButton] setHidden:yn==NO];
	}
	else
	{
		[pending setCloseable:yn];
	}
}

- (BOOL)isVisible
{
	if ([self hasWindow])
	{
		return [[window config] isVisible];
	}
	else
	{
		return [pending isVisible];
	}
}

- (void)setVisible:(BOOL)yn
{
	if ([self hasWindow])
	{
		[[window config] setVisible:yn];
		if (yn)
		{
			[self show:false];
		}
		else
		{
			[self hide:false];
		}
	}
	else
	{
		[pending setVisible:yn];
	}
}

- (void)setIcon:(NSString*)icon
{
	//TODO:
}

- (NSString*)getIcon
{
	//TODO:
	return nil;
}

+ (NSString *) webScriptNameForSelector:(SEL)sel{
	if (sel == @selector(show:))
	{
		return @"show";
	}
	else if (sel == @selector(open))
	{
		return @"open";
	}
	else if (sel == @selector(close))
	{
		return @"close";
	}
	else if (sel == @selector(activate))
	{
		return @"activate";
	}
	else if (sel == @selector(minimize))
	{
		return @"minimize";
	}
	else if (sel == @selector(maximize))
	{
		return @"maximize";
	}
	else if (sel == @selector(hide:))
	{
		return @"hide";
	}
	else if (sel == @selector(setURL:))
	{
		return @"setURL";
	}
	else if (sel == @selector(getURL))
	{
		return @"getURL";
	}
	else if (sel == @selector(setIcon:))
	{
		return @"setIcon";
	}
	else if (sel == @selector(getIcon))
	{
		return @"getIcon";
	}
	else if (sel == @selector(getTitle))
	{
		return @"getTitle";
	}
	else if (sel == @selector(setTitle:))
	{
		return @"setTitle";
	}
	else if (sel == @selector(setTransparency:))
	{
		return @"setTransparency";
	}
	else if (sel == @selector(getTransparency))
	{
		return @"getTransparency";
	}
	else if (sel == @selector(isUsingChrome))
	{
		return @"isUsingChrome";
	}
	else if (sel == @selector(setUsingChrome:))
	{
		return @"setUsingChrome";
	}
	else if (sel == @selector(getID))
	{
		return @"getID";
	}
	else if (sel == @selector(getX))
	{
		return @"getX";
	}
	else if (sel == @selector(setX:))
	{
		return @"setX";
	}
	else if (sel == @selector(getY))
	{
		return @"getY";
	}
	else if (sel == @selector(setY:))
	{
		return @"setY";
	}
	else if (sel == @selector(getWidth))
	{
		return @"getWidth";
	}
	else if (sel == @selector(setWidth:))
	{
		return @"setWidth";
	}
	else if (sel == @selector(getHeight))
	{
		return @"getHeight";
	}
	else if (sel == @selector(setHeight:))
	{
		return @"setHeight";
	}
	else if (sel == @selector(getBounds))
	{
		return @"getBounds";
	}
	else if (sel == @selector(setBounds:))
	{
		return @"setBounds";
	}
	else if (sel == @selector(isResizable))
	{
		return @"isResizable";
	}
	else if (sel == @selector(setResizable:))
	{
		return @"setResizable";
	}
	else if (sel == @selector(isMaximizable))
	{
		return @"isMaximizable";
	}
	else if (sel == @selector(setMaximizable:))
	{
		return @"setMaximizable";
	}
	else if (sel == @selector(isMinimizable))
	{
		return @"isMinimizable";
	}
	else if (sel == @selector(setMinimizable:))
	{
		return @"setMinimizable";
	}
	else if (sel == @selector(isCloseable))
	{
		return @"isCloseable";
	}
	else if (sel == @selector(setCloseable:))
	{
		return @"setCloseable";
	}
	else if (sel == @selector(isFullscreen))
	{
		return @"isFullscreen";
	}
	else if (sel == @selector(setFullscreen:))
	{
		return @"setFullscreen";
	}
	else if (sel == @selector(isVisible))
	{
		return @"isVisible";
	}
	else if (sel == @selector(setVisible:))
	{
		return @"setVisible";
	}
	else if (sel == @selector(isUsingScrollbars))
	{
		return @"isUsingScrollbars";
	}
	else if (sel == @selector(setUsingScrollbars:))
	{
		return @"setUsingScrollbars";
	}
	else if (sel == @selector(getBounds))
	{
		return @"getBounds";
	}
	else if (sel == @selector(setBounds:))
	{
		return @"setBounds";
	}
	else if (sel == @selector(setContent:))
	{
		return @"setContent";
	}
	else if (sel == @selector(getContent))
	{
		return @"getContent";
	}
 	return nil;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel{
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name{
	return YES;
}

@end
