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

static TIBrowserDocument *firstDocument = nil;

TIBrowserWindowController *TIFirstController() {
	return [firstDocument browserWindowController];
}

TIBrowserWindowController *TIFrontController() {
	TIBrowserDocument *doc = (TIBrowserDocument *)[[TIAppDelegate instance] currentDocument];
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
@end

@implementation TIAppDelegate

+ (id)instance {
	return (TIAppDelegate *)[NSApp delegate];
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

- (TIBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request makeKey:(BOOL)makeKey {
	TIBrowserDocument *oldDoc = [self currentDocument];
	TIBrowserDocument *newDoc = [self openUntitledDocumentAndDisplay:makeKey error:nil];
	
	if (!makeKey) {
		[newDoc makeWindowControllers];
	}
	
	[newDoc loadRequest:request];
	
	if (!makeKey) {
		NSWindow *oldWindow = [[[oldDoc windowControllers] objectAtIndex:0] window];
		NSWindow *newWindow = [[[newDoc windowControllers] objectAtIndex:0] window];;
		[newWindow orderWindow:NSWindowBelow relativeTo:oldWindow.windowNumber];
	}
	
	return newDoc;
}


#pragma mark -
#pragma mark NSDocumentController

- (id)makeUntitledDocumentOfType:(NSString *)typeName error:(NSError **)outError {
	id result = [super makeUntitledDocumentOfType:typeName error:outError];
	
	if ([typeName isEqualToString:@"BrowserDocument"]) {
		TIBrowserDocument *doc = (TIBrowserDocument *)result;

		// the first time a browser document is created, keep a reference to it as the 'firstDocument'
		static BOOL hasBeenCalled = NO;
		
		@synchronized (self) {
			if (!hasBeenCalled) {
				hasBeenCalled = YES;
				firstDocument = doc;
				[doc setIsFirst:YES];
			}
		}
	}
	
	return result;
}


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
		TIBrowserWindowController *winController = TIFrontController();
		[winController loadRequest:request];
	} else {
		NSLog(@"Error: could not load base url");
	}
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
