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

#import "TIAppDelegate.h"
#import "TIBrowserDocument.h"
#import "TIBrowserWindowController.h"
#import "TIPreferencesWindowController.h"
#import "WebViewPrivate.h"
#import "WebInspector.h"
#import <WebKit/WebKit.h>

static TIBrowserDocument *firstDocument = nil;

TIBrowserWindowController *TIFirstController() {
	return [firstDocument browserWindowController];
}

TIBrowserWindowController *TIFrontController() {
	TIBrowserDocument *doc = [[TIAppDelegate instance] currentDocument];
	if (doc) {
		return [doc browserWindowController];
	} else {
		return nil;
	}
}

static BOOL containsElement(NSXMLElement *el, NSString *name) {
	return [[el elementsForName:name] count] > 0;
}

static NSXMLElement *getElement(NSXMLElement *el, NSString *name) {
	return [[el elementsForName:name] objectAtIndex:0];
}

static NSString *elementText(NSXMLElement *el, NSString *name) {
	return [getElement(el, name) stringValue];
}

static NSString *attrText(NSXMLElement *el, NSString *name) {
	return [[el attributeForName:name] stringValue];
}

@interface TIBrowserDocument (Friend)
- (void)setIsFirst:(BOOL)yn;
@end

@interface TIAppDelegate (Private)
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

@implementation TIAppDelegate

+ (id)instance {
	return [NSApp delegate];
}


+ (void)initialize {
	[self setupDefaults];
}


+ (void)setupDefaults {
	NSString *path = [[NSBundle mainBundle] pathForResource:@"DefaultValues" ofType:@"plist"];

	NSMutableDictionary *defaultValues = [NSMutableDictionary dictionaryWithContentsOfFile:path];
	
	[[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:defaultValues];
	
	[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
	[[NSUserDefaults standardUserDefaults] synchronize];
}


- (id)init {
	self = [super init];
	if (self != nil) {
		[self registerForAppleEvents];
	}
	return self;
}


- (void)dealloc {
	[self unregisterForAppleEvents];
	[self setEndpoint:nil];
	[self setAppName:nil];
	[self setWindowTitle:nil];
	[self setStartPath:nil];
	[super dealloc];
}


- (void)awakeFromNib {
	[self parseTiAppXML];
	[self updateAppNameInMainMenu];
	[self setupWebPreferences];
}


- (void)applicationDidFinishLaunching:(NSNotification *)n {
	[self loadFirstPage];
}


#pragma mark -
#pragma mark Actions

- (IBAction)showPreferencesWindow:(id)sender {
	[[TIPreferencesWindowController instance] showWindow:sender];
}


// we can use this for total customization of the About window.
// OR, just allow the developer to provide a 'Credits.rtf' file in the app bundle
// and remove this. that will show those credits in the default About window template
- (IBAction)showAboutWindow:(id)sender {
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


- (IBAction)showWebInspector:(id)sender {
	[[self webInspectorForFrontWindowController] show:sender];
}


- (IBAction)showErrorConsole:(id)sender {
	[[self webInspectorForFrontWindowController] showConsole:sender];
}


- (IBAction)showNetworkTimeline:(id)sender {
	[[self webInspectorForFrontWindowController] showTimeline:sender];	
}


- (IBAction)toggleFullScreen:(id)sender {
	NSScreen *screen = nil;
	id webArchive = nil;
	WebView *wv = nil;
	
	TIBrowserWindowController *winController = TIFrontController();

	BOOL enteringFullScreen = ![self isFullScreen];
	
	if (winController) {
		screen = [[winController window] screen];
		
		wv = [winController webView];
		[wv stopLoading:self];
		webArchive = [[[wv mainFrame] dataSource] webArchive];

		[[winController document] close];
	}	
	
	if (enteringFullScreen) {
		[self setIsFullScreen:YES];
	} 
	// dont setIsFullScreen:NO, cuz that is handled automatically by -[TIBrowserDocument close]
	// so that closing a fullscreen window returns the main menu bar

	if (!screen) {
		screen = [NSScreen mainScreen];
	}
	
	TIBrowserDocument *doc = [self openUntitledDocumentAndDisplay:YES error:nil];
	winController = [doc browserWindowController];
	
	if ([self isFullScreen]) {
		NSRect frame = [screen frame];
		[[winController window] setFrame:frame display:YES];
	}
	
	wv = [winController webView];
	[wv stopLoading:self];
	if (webArchive) {
		[[wv mainFrame] loadArchive:webArchive];
	}
}


#pragma mark -
#pragma mark Public

- (TIBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request display:(BOOL)display {
	NSError *err = nil;
	TIBrowserDocument *newDoc = [self openUntitledDocumentAndDisplay:display error:&err];
	
	if (err) {
		return nil;
	}
	
	if (!display) {
		[newDoc makeWindowControllers];
		[[newDoc browserWindowController] window]; // force window loading from nib
	}
	
	[newDoc loadRequest:request];
	
	return newDoc;
}


- (WebFrame *)findFrameNamed:(NSString *)frameName {
	// look for existing frame in any open browser document with this name.
	WebFrame *existingFrame = nil;
	
	for (TIBrowserDocument *doc in [[TIAppDelegate instance] documents]) {
		existingFrame = [[[doc webView] mainFrame] findFrameNamed:frameName];
		if (existingFrame) {
			break;
		}
	}
	
	return existingFrame;
}


#pragma mark -
#pragma mark NSAppDelegate

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
	return NO; // this causes the app to *not* open an untitled browser window on launch
}


#pragma mark -
#pragma mark NSDocumentController



#pragma mark -
#pragma mark NSApplicationDelegate

- (BOOL)application:(NSApplication *)app openFile:(NSString *)filename {
	NSURL *URL = nil;
	@try {
		URL = [NSURL fileURLWithPath:filename];
	} @catch (NSException *e) {
		URL = [NSURL URLWithString:filename];
	}
	NSURLRequest *request = [NSURLRequest requestWithURL:URL];
	[self newDocumentWithRequest:request display:YES];
	return YES;
}


- (void)application:(NSApplication *)app openFiles:(NSArray *)filenames {
	for (NSString *filename in filenames) {
		[self application:app openFile:filename];
	}
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
	if ([URLString hasPrefix:tiScheme]) {
		
		//NSString *filePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:substringFromIndex:[tiScheme length]];

		NSString *s = [URLString substringFromIndex:[tiScheme length]];
		NSString *resPath = [[NSBundle mainBundle] resourcePath];
//		NSString *scriptPath = [[resPath stringByAppendingPathComponent:@"public"] stringByAppendingPathComponent:s];
		NSString *scriptPath = [resPath stringByAppendingPathComponent:s];
		URLString = [NSString stringWithContentsOfFile:scriptPath];
		//URLString = [NSString stringWithFormat:@"http://%@", [URLString substringFromIndex:[tiScheme length]]];

		NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
									 s,								// message
									 NSLocalizedString(@"OK", @""),			// default button
									 nil,									// alt button
									 nil);									// other button	
		
	}

	NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:URLString]];
	[self newDocumentWithRequest:request display:YES];
}


- (void)parseTiAppXML {
	NSString *appXMLPath = [[NSBundle mainBundle] pathForResource:@"tiapp" ofType:@"xml"];
	
	NSError *error = nil;
	NSURL *furl = [NSURL fileURLWithPath:appXMLPath];
	
	NSXMLDocument *doc = [[[NSXMLDocument alloc] initWithContentsOfURL:furl options:0 error:&error] autorelease];
	
	if (error) {
		NSLog(@"error getting xml doc: %@", error);
		return;
	}
	
	NSXMLElement *root = [doc rootElement];
	
	[self setAppName:elementText(root, @"name")];
	
	if (containsElement(root, @"appc:endpoint")) {
		[self setEndpoint:elementText(root, @"appc:endpoint")];
	}
	
	NSXMLElement *window = getElement(root, @"window");
	CGFloat width = [attrText(window, @"width") floatValue];
	CGFloat height = [attrText(window, @"height") floatValue];
	[self setWindowWidth:width height:height];
	
	[self setWindowTitle:elementText(window, @"title")];
	[self setStartPath:elementText(window, @"start")];
}

+ (void)setFirstDocument:(TIBrowserDocument *)document
{
	firstDocument = document;
	[firstDocument setIsFirst:YES];
}


- (void)loadFirstPage {
	NSArray *args = [[NSProcessInfo processInfo] arguments];
	NSURL *url = nil;
	
	if ([args count] == 3) {
		NSString *arg1 = [args objectAtIndex:1];
		NSString *arg2 = [args objectAtIndex:2];
		
		if ([arg1 isEqualToString:@"-file"]) {
			url = [NSURL fileURLWithPath:arg2];
		} else if ([arg1 isEqualToString:@"-url"]) {
			url = [NSURL URLWithString:arg2];
		}
	} else {
		NSString *relativePath = [self startPath];
		NSLog(@"starting with path:%@\n", relativePath);
		
		if ([relativePath length]) {
			NSString *fullPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:relativePath];
			url = [NSURL fileURLWithPath:fullPath];
		}
	}
	
	if (url) {
		NSURLRequest *request = [NSURLRequest requestWithURL:url];
		
		// set display:NO so we don't show the first window until the first page has loaded.
		// this avoids seeing ugly loading on launch. show window in webView:didFinishLoadForFrame:
		firstDocument = [self newDocumentWithRequest:request display:NO];
		[firstDocument setIsFirst:YES];
	} else {
		NSLog(@"Error: could not load base url");
	}
}


- (void)updateAppNameInMainMenu {
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


- (void)setupWebPreferences {
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


- (WebInspector *)webInspectorForFrontWindowController {
	TIBrowserWindowController *winController = TIFrontController();
	return [[winController webView] inspector];
}


#pragma mark -
#pragma mark Accessors

- (BOOL)isFullScreen {
	return isFullScreen;
}


- (void)setIsFullScreen:(BOOL)yn {
	isFullScreen = yn;
	// hide the system main menu
	[NSMenu setMenuBarVisible:!yn];
}


- (CGFloat)windowWidth {
	return windowWidth;
}


- (CGFloat)windowHeight {
	return windowHeight;
}


- (void)setWindowWidth:(CGFloat)w height:(CGFloat)h {
	windowWidth = w;
	windowHeight = h;
}


- (NSString *)endpoint {
	return [[endpoint copy] autorelease];
}


- (void)setEndpoint:(NSString*)s {
	if (endpoint != s) {
		[endpoint autorelease];
		endpoint = [s copy];
	}
}


- (NSString *)appName {
	return [[appName copy] autorelease];
}


- (void)setAppName:(NSString*)s {
	if (appName != s) {
		[appName autorelease];
		appName = [s copy];
	}
}


- (NSString *)windowTitle {
	return [[windowTitle copy] autorelease];
}


- (void)setWindowTitle:(NSString *)s {
	if (windowTitle != s) {
		[windowTitle autorelease];
		windowTitle = [s copy];
	}
}


- (NSString *)startPath {
	return [[startPath copy] autorelease];
}


- (void)setStartPath:(NSString*)s {
	if (startPath != s) {
		[startPath autorelease];
		startPath = [s copy];
	}
}

@end
