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

#import "TiAppDelegate.h"
#import "TiBrowserDocument.h"
#import "TiBrowserWindowController.h"
#import "TiPreferencesWindowController.h"
#import "TiSplashScreen.h"
#import "TiSplashScreenWindowController.h"
#import "WebViewPrivate.h"
#import "WebInspector.h"
#import <WebKit/WebKit.h>

static TiBrowserDocument *firstDocument = nil;

TiBrowserWindowController *TIFirstController() 
{
	return [firstDocument browserWindowController];
}

TiBrowserWindowController *TIFrontController() 
{
	TiBrowserDocument *doc = [[TiAppDelegate instance] currentDocument];
	if (doc) 
	{
		return [doc browserWindowController];
	}
	return nil;
}

static BOOL containsElement(NSXMLElement *el, NSString *name) 
{
	NSArray *array = [el elementsForName:name];
	if (!array) return false;
	return [array count] > 0;
}

static NSXMLElement *getElement(NSXMLElement *el, NSString *name) 
{
	NSArray *array = [el elementsForName:name];
	if (!array || [array count] == 0) return nil;
	return [array objectAtIndex:0];
}

static NSString *elementText(NSXMLElement *el, NSString *name) 
{
	NSXMLElement *elem = getElement(el, name);
	return elem ? [elem stringValue] : nil;
}

static NSString *attrText(NSXMLElement *el, NSString *name) 
{
	NSXMLNode *node = [el attributeForName:name];
	if (!node) return nil;
	return [node stringValue];
}

static BOOL toBoolean (NSString* value, BOOL def)
{
	if (value)
	{
		if ([value compare:@"true"] == NSOrderedSame)
		{
			return true;
		}
		return false;
	}
	return def;
}

static CGFloat toFloat (NSString* value, CGFloat def)
{
	if (value)
	{
		if ([value compare:@"1.0"] == NSOrderedSame)
		{
			return 1.0;
		}
		return [value floatValue];
	}
	return def;
}


@interface TiBrowserDocument (Friend)
- (void)setIsFirst:(BOOL)yn;
@end

@interface TiAppDelegate (Private)
+ (void)setupDefaults;

- (void)registerForAppleEvents;
- (void)unregisterForAppleEvents;
- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event replyEvent:(NSAppleEventDescriptor *)replyEvent;
- (void)parseTiAppXML;
- (void)loadFirstPage;
- (void)updateAppNameInMainMenu;
- (void)setupWebPreferences;
- (WebInspector *)webInspectorForFrontWindowController;
@end

@implementation TiAppDelegate

+ (id)instance 
{
	return [NSApp delegate];
}


+ (void)initialize 
{
	[self setupDefaults];
}


+ (void)setupDefaults 
{
	NSString *path = [[NSBundle mainBundle] pathForResource:@"DefaultValues" ofType:@"plist"];

	NSMutableDictionary *defaultValues = [NSMutableDictionary dictionaryWithContentsOfFile:path];
	
	[[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:defaultValues];
	
	[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
	[[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)showSplash
{
	splashController = [[TiSplashScreenWindowController alloc] init];
	NSWindow *win = [splashController initWithWindowNibName:@"SplashWindow"];
	[splashController showWindow:win];
}

- (id)init 
{
	self = [super init];
	if (self != nil) 
	{
		[self showSplash];
		arguments = [[TiAppArguments alloc] init];
		windowOptions = [[NSMutableArray alloc] init];
		[self registerForAppleEvents];
	}
	return self;
}


- (void)dealloc 
{
	[self unregisterForAppleEvents];
	if (activeWindowOption != nil)
	{
		[activeWindowOption release];
	}
	for (int c=0;c<[windowOptions count];c++)
	{
		TiWindowOptions *o = [windowOptions objectAtIndex:c];
		[o release];
	}
	[windowOptions dealloc];
	[self setEndpoint:nil];
	[self setAppName:nil];
	[super dealloc];
}

- (void)hideSplash
{
	if (splashController)
	{
		[splashController close];
		[splashController release];
		splashController = nil;
	}
}

- (void)awakeFromNib 
{
	[self parseTiAppXML];
	
	if ([arguments devLaunch]) {
		[self initDevEnvironment];
	}
	
	[self updateAppNameInMainMenu];
	[self setupWebPreferences];
}


- (void)applicationDidFinishLaunching:(NSNotification *)n 
{
	[self loadFirstPage];
	[self hideSplash];
}


#pragma mark -
#pragma mark Actions

- (IBAction)showPreferencesWindow:(id)sender 
{
	[[TiPreferencesWindowController instance] showWindow:sender];
}


// we can use this for total customization of the About window.
// OR, just allow the developer to provide a 'Credits.rtf' file in the app bundle
// and remove this. that will show those credits in the default About window template
- (IBAction)showAboutWindow:(id)sender 
{
	NSMutableDictionary *dict = [NSMutableDictionary dictionary];

	NSAttributedString *as = [[NSAttributedString alloc] initWithString:@"These are Credits"];
	[dict setObject:as forKey:@"Credits"];
	[as release];
	
	[dict setObject:[self appName] forKey:@"ApplicationName"];

//	NSImage *image = [NSImage imageNamed:@"NSApplicationIcon"];
//	if (image) [dict setObject: forKey:@"ApplicationIcon"];
	
	NSString *copyright = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"NSHumanReadableCopyright"];
	if (copyright) [dict setObject:copyright forKey:@"Copyright"];
	
	NSString *version = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
	if (version) [dict setObject:version forKey:@"Version"];
	
	NSString *shortVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
	if (shortVersion) [dict setObject:shortVersion forKey:@"ApplicationVersion"];

	[NSApp orderFrontStandardAboutPanelWithOptions:dict];
}


- (IBAction)showWebInspector:(id)sender 
{
	[[self webInspectorForFrontWindowController] show:sender];
}


- (IBAction)showErrorConsole:(id)sender 
{
	[[self webInspectorForFrontWindowController] showConsole:sender];
}


- (IBAction)showNetworkTimeline:(id)sender 
{
	[[self webInspectorForFrontWindowController] showTimeline:sender];	
}


- (IBAction)toggleFullScreen:(id)sender 
{
	/*
	NSScreen *screen = nil;
	id webArchive = nil;
	WebView *wv = nil;
	
	TiBrowserWindowController *winController = TIFrontController();

	BOOL enteringFullScreen = ![windowOptions isFullscreen];
	
	if (winController) {
		screen = [[winController window] screen];
		
		wv = [winController webView];
		[wv stopLoading:self];
		webArchive = [[[wv mainFrame] dataSource] webArchive];

		[[winController document] close];
	}	
	
	if (enteringFullScreen) {
	//	[self setIsFullScreen:YES];
		//FIXME FIXME
		
	} 
	// dont setIsFullScreen:NO, cuz that is handled automatically by -[TIBrowserDocument close]
	// so that closing a fullscreen window returns the main menu bar

	if (!screen) {
		screen = [NSScreen mainScreen];
	}
	
	TiBrowserDocument *doc = [self openUntitledDocumentAndDisplay:YES error:nil];
	winController = [doc browserWindowController];

	
	if ([windowOptions isFullscreen]) {
		NSRect frame = [screen frame];
		[[winController window] setFrame:frame display:YES];
	}
	
	wv = [winController webView];
	[wv stopLoading:self];
	if (webArchive) {
		[[wv mainFrame] loadArchive:webArchive];
	}
	 */
}


#pragma mark -
#pragma mark Public

- (void)error:(NSString *)error
{
	NSLog(@"Applicaiton Error: %@\n", error);
	NSString *msg = [@"" stringByAppendingFormat:@"The application has encountered an error:\n\n%@", error];
	NSAlert *alert = [[NSAlert alloc] init];
	[alert setMessageText:msg];
	[alert setAlertStyle:NSWarningAlertStyle];
	[alert runModal];
	[alert release];
}

- (TiBrowserDocument *)newDocumentWithOptions:(NSURLRequest *)request options:(TiWindowOptions*)opts
{
	[self error:@"newDocumentWithOptions not implemented"];
	return nil;
//	TiBrowserDocument *newDoc = [[TiBrowserDocument alloc] init];
//	[newDoc setOptions:opts];
//	[self addDocument:newDoc];
//	
//	bool show = YES;
//	
//	if (show) {
//		[newDoc makeWindowControllers];
//		[[newDoc browserWindowController] window]; // force window loading from nib
//	}
//	
//	[newDoc loadRequest:request];
//	
//	return newDoc;
}

- (void)setActiveWindowOption:(TiWindowOptions*)o
{
	if (activeWindowOption != nil )
	{
		[activeWindowOption release];
		activeWindowOption = nil;
	}
	
	if (o != nil)
	{
		activeWindowOption = o;
		[activeWindowOption retain];
	}
}

- (TiWindowOptions*)getActiveWindowOption
{
	if (activeWindowOption != nil)
	{
		[activeWindowOption retain];
	}
	return activeWindowOption;
}


- (TiBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request display:(BOOL)display 
{
	NSError *err = nil;

	TiBrowserDocument *newDoc = [[TiBrowserDocument alloc] init];
	TiWindowOptions *opts = [self findWindowOptionsForURLSpec:request];
	
	if (opts==nil)
	{
		opts = [self findInitialWindowOptions];
	}
	
	[newDoc setOptions:opts];
	[opts release];
	
	[self addDocument:newDoc];
	
	if (err) 
	{
		[self error:@"Error opening document"];
		return nil;
	}
	
	TiBrowserWindowController *winController = TIFrontController();
	if ([winController isClosing])
	{
		NSDocument *doc = [winController document];
		[doc close];
	}
	
	if (!display) 
	{
		[newDoc makeWindowControllers];
		[[newDoc browserWindowController] window]; // force window loading from nib
	}
	
	[newDoc loadRequest:request];
	[newDoc release];
	
	return newDoc;
}

- (WebFrame *)findFrameNamed:(NSString *)frameName 
{
	// look for existing frame in any open browser document with this name.
	WebFrame *existingFrame = nil;
	
	for (TiBrowserDocument *doc in [[TiAppDelegate instance] documents]) 
	{
		existingFrame = [[[doc webView] mainFrame] findFrameNamed:frameName];
		if (existingFrame) 
		{
			break;
		}
	}
	
	return existingFrame;
}


#pragma mark -
#pragma mark NSAppDelegate

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender 
{
	return NO; // this causes the app to *not* open an untitled browser window on launch
}


#pragma mark -
#pragma mark NSDocumentController



#pragma mark -
#pragma mark NSApplicationDelegate

- (BOOL)application:(NSApplication *)app openFile:(NSString *)filename 
{
	[self error:@"openFile called but not implemented"];
	
//	NSURL *URL = nil;
//	@try {
//		URL = [NSURL fileURLWithPath:filename];
//	} @catch (NSException *e) {
//		URL = [NSURL URLWithString:filename];
//	}
//	NSURLRequest *request = [NSURLRequest requestWithURL:URL];
//	[self newDocumentWithRequest:request display:YES];
	return YES;
}


- (void)application:(NSApplication *)app openFiles:(NSArray *)filenames 
{
	[self error:@"openFiles called but not implemented"];
//	for (NSString *filename in filenames) {
//		[self application:app openFile:filename];
//	}
}


#pragma mark -
#pragma mark Private 

- (void)registerForAppleEvents {
	[[NSAppleEventManager sharedAppleEventManager] setEventHandler:self
													   andSelector:@selector(handleGetURLEvent:replyEvent:)
													 forEventClass:kInternetEventClass
														andEventID:kAEGetURL];
}


- (void)unregisterForAppleEvents {
	[[NSAppleEventManager sharedAppleEventManager] removeEventHandlerForEventClass:kInternetEventClass andEventID:kAEGetURL];
}


- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event replyEvent:(NSAppleEventDescriptor *)replyEvent {
	NSString *URLString = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];

	
	NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
								 URLString,								// message
								 NSLocalizedString(@"OK", @""),			// default button
								 nil,									// alt button
								 nil);									// other button	
	
	NSString *tiScheme = @"ti://";
	if ([URLString hasPrefix:tiScheme]) 
	{
		NSString *s = [URLString substringFromIndex:[tiScheme length]];
		NSString *resPath = [[NSBundle mainBundle] resourcePath];
		NSString *scriptPath = [resPath stringByAppendingPathComponent:s];
		URLString = [NSString stringWithContentsOfFile:scriptPath];

		NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
									 s,										// message
									 NSLocalizedString(@"OK", @""),			// default button
									 nil,									// alt button
									 nil);									// other button	
		
	}

	NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:URLString]];
	[self newDocumentWithRequest:request display:YES];
}

- (TiWindowOptions*) findInitialWindowOptions
{
	TiWindowOptions* opt = [windowOptions objectAtIndex:0];
//	[opt retain];
	return opt;
}

- (TiWindowOptions*) findWindowOptionsForURLSpec:(NSURLRequest*)urlrequest
{
	NSURL *URL = [urlrequest URL];
	NSString *url = [URL absoluteString]; 
	TiWindowOptions *opt = nil;
	for (int c=0;c<[windowOptions count];c++)
	{
		opt = [windowOptions objectAtIndex:c];
		if ([opt urlMatches:url])
		{
			[opt retain];
			break;
		}
	}
	return opt;
}


- (void)parseTiAppXML 
{
	NSString *appXMLPath = [[NSBundle mainBundle] pathForResource:@"tiapp" ofType:@"xml"];
	if ([arguments tiAppXml] != nil) 
	{
		appXMLPath = [arguments tiAppXml];
	}
	
	NSError *error = nil;
	NSURL *furl = [NSURL fileURLWithPath:appXMLPath];
	
	NSXMLDocument *doc = [[[NSXMLDocument alloc] initWithContentsOfURL:furl options:0 error:&error] autorelease];
	
	if (error) {
		NSLog(@"error getting xml doc: %@", error);
		return;
	}
	
	NSXMLElement *root = [doc rootElement];
	
	[self setAppName:elementText(root, @"name")];
	
	if (containsElement(root, @"appc:endpoint")) 
	{
		[self setEndpoint:elementText(root, @"appc:endpoint")];
	}

	NSArray *array = [root elementsForName:@"window"];
	for (int c = 0; c < [array count]; c++) 
	{
		NSXMLElement *window = [array objectAtIndex:c];
		NSString *windowID = elementText(window,@"id");
		
		CGFloat width = toFloat(elementText(window,@"width"),500);
		CGFloat height = toFloat(elementText(window,@"height"),500);
		
		// default is no minimum
		CGFloat minWidth = toFloat(elementText(window,@"min-width"),0);
		CGFloat minHeight = toFloat(elementText(window,@"min-height"),0);
		
		// default is very large maximum
		CGFloat maxWidth = toFloat(elementText(window,@"max-width"),9000);
		CGFloat maxHeight = toFloat(elementText(window,@"max-height"),9000);
		
		NSLog(@"initial window: %fx%f, min: %fx%f, max: %fx%f\n",width,height,minWidth,minHeight,maxWidth,maxHeight);
		
		NSString *title = elementText(window, @"title");
		NSString *url = elementText(window, @"url");		
		bool chrome = toBoolean(elementText(window, @"chrome"),YES);		
		bool scrollbars = YES;
		if (chrome)
		{
			NSXMLElement *c = getElement(window, @"chrome");
			scrollbars = [attrText(c, @"scrollbars") boolValue];
		}
		bool maximizable = toBoolean(elementText(window, @"maximizable"),YES);		
		bool minimizable = toBoolean(elementText(window, @"minimizable"),YES);		
		bool resizable = toBoolean(elementText(window, @"resizable"),YES);		
		bool closeable = toBoolean(elementText(window, @"closeable"),YES);		
		CGFloat transparency = toFloat(elementText(window, @"transparency"),1.0);		
		
		TiWindowOptions *options = [[TiWindowOptions alloc] init];
		[options setID:windowID];
		[options setWidth:width];
		[options setHeight:height];
		[options setMinWidth:minWidth];
		[options setMinHeight:minHeight];
		[options setMaxWidth:maxWidth];
		[options setMaxHeight:maxHeight];
		[options setTitle:title];
		[options setURL:url];
		[options setChrome:chrome];
		[options setMaximizable:maximizable];
		[options setMinimizable:minimizable];
		[options setCloseable:closeable];
		[options setResizable:resizable];
		[options setTransparency:transparency];
		[options setScrollbars:scrollbars];

		[windowOptions addObject:options];
		[options release]; // add retains
	}
	
	TiWindowOptions *first = [self findInitialWindowOptions];
	[self setActiveWindowOption: first];
	[first release];
}

- (void)initDevEnvironment 
{
}

+ (void)setFirstDocument:(TiBrowserDocument *)document
{
	firstDocument = document;
	[firstDocument setIsFirst:YES];
}


- (void)loadFirstPage 
{
	NSURL *url = nil;
	TiWindowOptions *options = [[TiAppDelegate instance] findInitialWindowOptions];
	NSString *relativePath = [options getURL];
	
	if (!relativePath)
	{
		relativePath = @"index.html";
	}
	
	NSLog(@"starting with path:%@\n", relativePath);
	
	if ([relativePath length]) 
	{
		NSString *fullPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:relativePath];
		if ([arguments devLaunch]) 
		{
			char *cwd = getcwd(NULL, 0);
			fullPath = [[NSString stringWithCString:cwd length:strlen(cwd)] stringByAppendingPathComponent:relativePath];
		}
		
		url = [NSURL fileURLWithPath:fullPath];
	}
	
	if (url) 
	{
		NSURLRequest *request = [NSURLRequest requestWithURL:url];
		
		// set display:NO so we don't show the first window until the first page has loaded.
		// this avoids seeing ugly loading on launch. show window in webView:didFinishLoadForFrame:
		firstDocument = [self newDocumentWithRequest:request display:NO];
		[firstDocument setIsFirst:YES];
	} 
	else 
	{
		NSString *urlStr = [url absoluteString]; 
		[self error:[@"Could not load appliction from: " stringByAppendingString:urlStr]];
		[NSApp quit];
	}
	
	[options release];
}


- (void)updateAppNameInMainMenu 
{
	// fixup App Menu
	NSMenu *appMenu = [[[NSApp mainMenu] itemAtIndex:0] submenu];

	NSMenuItem *aboutItem = [appMenu itemWithTitle:NSLocalizedString(@"About Titanium", @"")];
	[aboutItem setTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", @""), appName]];

	NSMenuItem *hideItem = [appMenu itemWithTitle:NSLocalizedString(@"Hide Titanium", @"")];
	[hideItem setTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Hide", @""), appName]];

	NSMenuItem *quitItem = [appMenu itemWithTitle:NSLocalizedString(@"Quit Titanium", @"")];
	[quitItem setTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", @""), appName]];

	// fixup Help Menu
	NSMenu *helpMenu = [[[NSApp mainMenu] itemWithTitle:NSLocalizedString(@"Help", @"")] submenu];

	NSMenuItem *helpItem = [helpMenu itemWithTitle:NSLocalizedString(@"Titanium Help", @"")];
	[helpItem setTitle:[NSString stringWithFormat:@"%@ %@", appName, NSLocalizedString(@"Help", @"")]];
}


- (void)setupWebPreferences 
{
	WebPreferences *webPrefs = [WebPreferences standardPreferences];
	// This indicates that WebViews in this app will not browse multiple pages, but rather show a small number.
	// this reduces memory cache footprint significantly.
	
	// if we expect to browse a slightly larger number of documents, we might set this to WebCacheModelDocumentBrowser instead
	// that would increase memory cache footprint some tho.
	[webPrefs setCacheModel:WebCacheModelDocumentViewer];
	
	[webPrefs setPlugInsEnabled:YES]; // ?? this disallows Flash content
	[webPrefs setJavaEnabled:YES]; // ?? this disallows Java Craplets
	[webPrefs setJavaScriptEnabled:YES];
}


- (WebInspector *)webInspectorForFrontWindowController 
{
	TiBrowserWindowController *winController = TIFrontController();
	return [[winController webView] inspector];
}


#pragma mark -
#pragma mark Accessors

- (NSString *)endpoint 
{
	return [[endpoint copy] autorelease];
}


- (void)setEndpoint:(NSString*)s 
{
	if (endpoint != s) 
	{
		[endpoint autorelease];
		endpoint = [s copy];
	}
}


- (NSString *)appName 
{
	return [[appName copy] autorelease];
}


- (void)setAppName:(NSString*)s 
{
	if (appName != s) 
	{
		[appName autorelease];
		appName = [s copy];
	}
}

- (TiAppArguments *)arguments {
	return arguments;
}

@end
