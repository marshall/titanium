//
//  TIAppDelegate.m
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "TIAppDelegate.h"
#import "TIBrowserDocument.h"
#import "TIBrowserWindowController.h"
#import "TIPreferencesWindowController.h"
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
- (void)parseTiAppXml;
- (void)loadFirstPage;
- (void)updateAppNameInMainMenu;
- (void)setupWebPreferences;
@end

@implementation TIAppDelegate

+ (id)instance {
	return [NSApp delegate];
}


- (id)init {
	self = [super init];
	if (self != nil) {
	}
	return self;
}


- (void)dealloc {
	[self setEndpoint:nil];
	[self setAppName:nil];
	[self setWindowTitle:nil];
	[self setStartPath:nil];
	[super dealloc];
}


- (void)awakeFromNib {
	[self parseTiAppXml];
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
#pragma mark Private

- (void)parseTiAppXml {
	NSString *appXmlPath = [[NSBundle mainBundle] pathForResource:@"tiapp" ofType:@"xml"];
	
	NSError *error = nil;
	NSURL *furl = [NSURL fileURLWithPath:appXmlPath];
	
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


- (void)loadFirstPage {
	NSArray *args = [[NSProcessInfo processInfo] arguments];
	NSURL *url = nil;
	
	if ([args count] == 3) {
		NSString *arg1 = [args objectAtIndex:1];
		if ([arg1 isEqualToString:@"-file"]) {
			NSString *pathString = [args objectAtIndex:2];
			url = [NSURL fileURLWithPath:pathString];
			
		} else if ([arg1 isEqualToString:@"-url"]) {
			NSString *urlString = [args objectAtIndex:2];
			url = [NSURL URLWithString:urlString];
		}
	} else {
		NSString *relativePath = [self startPath];
		
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
	
	[webPrefs setPlugInsEnabled:NO]; // ?? this disallows Flash content
	[webPrefs setJavaEnabled:NO]; // ?? this disallows Java Craplets
	[webPrefs setJavaScriptEnabled:YES];
}


#pragma mark -
#pragma mark Accessors

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
