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
#import "TiController.h"
#import "TiProtocol.h"
#import "AppProtocol.h"

@class WebPluginDatabase;
@class WebBasePluginPackage;

static TiController* instance = nil;

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

@implementation TiController

- (void)showSplash
{
	splashController = [[NSWindowController alloc] init];
	NSWindow *win = [splashController initWithWindowNibName:@"SplashWindow"];
	[splashController showWindow:win];
	// cause the splash screen to first come on top of other window
	[NSApp arrangeInFront:win];
	[NSApp activateIgnoringOtherApps:YES];
}

- (void)hideSplash
{
	if (splashController)
	{
		[splashController release];
		splashController=nil;
	}
}

- (id)init
{
	self = [super init];
	if (self != nil)
	{
		TRACE(@"TiController::init = %x",self);
		instance = self;
		[self showSplash];
		arguments = [[TiAppArguments alloc] init];
		windowConfigs = [[NSMutableArray alloc] init];
		[self loadApplicationID];
		[self loadApplicationXML];
	}
	return self;
}

- (void)dealloc
{
	TRACE(@"TiController::dealloc = %x",self);
	[arguments release];
	arguments=nil;
	for (int c=0;c<[windowConfigs count];c++)
	{
		id o = [windowConfigs objectAtIndex:c];
		[o release];
	}
	[windowConfigs release];
	windowConfigs=nil;
	[appName release];
	appName = nil;
	[appID release];
	appID = nil;
	[appGUID release];
	appGUID = nil;
	[appVersion release];
	appVersion = nil;
	[appUpdateURL release];
	appUpdateURL = nil;
    [super dealloc];
}

+ (TiController*) instance
{
	return instance;
}

- (void) registerProtocols
{
	[TiProtocol registerSpecialProtocol];
	[AppProtocol registerSpecialProtocol];
}

- (void)activate:(TiDocument*)doc
{
	activeDocument = doc;
}

- (void)deactivate:(TiDocument*)doc
{
	activeDocument = nil;
}

- (void)showInspector
{
	if (activeDocument)
	{
		[activeDocument showInspector];
	}
}

- (void)updateAppNameInMainMenu 
{
	// fixup App Menu
	NSMenu *appMenu = [[[NSApp mainMenu] itemWithTitle:@"Titanium"] submenu];
	
	//TODO: not refershing the title of the application for some reason
	[[appMenu supermenu] setTitle:appName];

	NSMenuItem *aboutItem = [appMenu itemWithTitle:NSLocalizedString(@"About Titanium", @"")];
	[aboutItem setTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", @""), appName]];
	
	NSMenuItem *hideItem = [appMenu itemWithTitle:NSLocalizedString(@"Hide Titanium", @"")];
	[hideItem setTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Hide", @""), appName]];
	
	NSMenuItem *quitItem = [appMenu itemWithTitle:NSLocalizedString(@"Quit Titanium", @"")];
	[quitItem setTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", @""), appName]];
	
	NSMenu *helpMenu = [[[NSApp mainMenu] itemWithTitle:NSLocalizedString(@"Help", @"")] submenu];
	
	NSMenuItem *helpItem = [helpMenu itemWithTitle:NSLocalizedString(@"Titanium Help", @"")];
	[helpItem setTitle:[NSString stringWithFormat:@"%@ %@", appName, NSLocalizedString(@"Help", @"")]];
	
	NSMenu *menu = [[[NSApp mainMenu] itemWithTitle:@"Titanium"] submenu];
	[menu setTitle:appName];
	
	//set up debugger window menu item
	BOOL showDebugInspector = [arguments debug];

#ifdef DEBUG
	showDebugInspector = YES;
#endif
	
	NSMenu *windowMenu = [[[NSApp mainMenu] itemWithTitle:NSLocalizedString(@"Window",@"")] submenu];
	NSMenuItem *showInspector = [windowMenu itemWithTitle:NSLocalizedString(@"Show Inspector", @"")];
	if (showDebugInspector)
	{
		[showInspector setEnabled:YES];
		[showInspector setAction:@selector(showInspector)];
	}	
	else
	{
		[showInspector setHidden:YES];
		NSMenuItem *showInspectorSep = [windowMenu itemWithTitle:@"Show Inspector Separator"];
		[showInspectorSep setHidden:YES];
	}
}

- (TiWindowConfig*) findInitialWindowConfig
{
	TiWindowConfig* opt = [windowConfigs objectAtIndex:0];
	return opt;
}

- (TiWindowConfig*) findWindowConfigForURLSpec:(NSURL*)URL
{
	NSString *url = [URL absoluteString]; 
	TiWindowConfig *opt = nil;
	for (int c=0;c<[windowConfigs count];c++)
	{
		opt = [windowConfigs objectAtIndex:c];
		if ([opt urlMatches:url])
		{
			return opt;
		}
	}
	return nil;
}

- (TiWindowConfig*) pendingConfig
{
	return pendingConfig;
}

- (void) resetPendingConfig
{
	[pendingConfig release];
	pendingConfig = nil;
}

- (void) setPendingConfig:(TiWindowConfig*)config
{
	[self resetPendingConfig];
	pendingConfig = config;
	[pendingConfig retain];
}

- (void)setupDefaults
{
	NSString *path = [[NSBundle mainBundle] pathForResource:@"DefaultValues" ofType:@"plist"];
	
	NSMutableDictionary *defaultValues = [NSMutableDictionary dictionaryWithContentsOfFile:path];
	
	[[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:defaultValues];
	
	[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
	[[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)awakeFromNib
{
	TRACE(@"TiController::awakeFromNib = %x",self);
	TRACE(@"gears titanium plugin has been loaded? %@", ([[WebPluginDatabase sharedDatabase] isMIMETypeRegistered:@"application/x-gears-titanium"])?@"true":@"false");

	[self setupDefaults];
	[self updateAppNameInMainMenu];
	[self registerProtocols];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
	// prevent document controller from opening a new untitled document when we startup
	return NO;
}

+ (void)documentOpened:(TiDocument*)doc
{
	// called by TiDocument when it's first opened and shown, we will just hide the splash if not already hidden
	[[TiController instance] hideSplash];
}

- (TiDocument*) createDocument:(NSURL*)url visible:(BOOL)visible config:(TiWindowConfig*)cfg;
{
	TRACE(@"TiController::createDocument for url: %@",[url absoluteString]);
	TiWindowConfig *config = cfg;
	if (config == nil)
	{
		config = [self findWindowConfigForURLSpec:url];
		if (config==nil)
		{
			config = [self findInitialWindowConfig];
		}
	}
	TRACE(@"TiController::createDocument - using pending config named: %@",[config getID]);
	[self setPendingConfig: config];
	TiDocument *doc = [[NSDocumentController sharedDocumentController] openUntitledDocumentOfType:@"HTML Document" display:NO];
	[doc setInitialShow:visible];
	[doc window]; // cause the window to be loaded from NIB
	if (visible)
	{
		[doc loadURL:url];
	}
	return doc;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	TRACE(@"TiController::applicationDidFinishLaunching");

	TiWindowConfig *config = [self findInitialWindowConfig];
	NSString *urlString = nil;
	BOOL appurl = YES;
	NSString *url = [config getURL];
	
	if ([url hasPrefix:@"http://"] || [url hasPrefix:@"https://"])
	{
		// allow absolute external URLs
		urlString = [config getURL];
		appurl = NO;
	}
	
	if (appurl)
	{
		if ([url hasPrefix:@"app:"])
		{
			NSURL *u = [NSURL URLWithString:url];
			NSString *host = [u host];
			urlString = [NSString stringWithFormat:@"app://%@/%@",appID,host];
		}
		else
		{
			urlString = [NSString stringWithFormat:@"app://%@/%@",appID,[url stringByStandardizingPath]];
		}
	}
	TRACE(@"my new url: %@",urlString);
	NSURL *URL = [NSURL URLWithString:urlString];
	[[TiController instance] createDocument:URL visible:YES config:nil];
}


- (NSString *)appName 
{
	return appName;
}

- (NSString*)appID
{
	return appID;
}

- (NSString*)appGUID
{
	return appGUID;
}

- (NSString*)appVersion
{
	return appVersion;
}

- (NSString*)appUpdateURL
{
	return appUpdateURL;
}

- (TiAppArguments*)arguments
{
	return arguments;
}

+ (void)error:(NSString *)error
{
	TRACE(@"Titanium Application Error: %@\n", error);
	NSString *msg = [@"" stringByAppendingFormat:@"The application has encountered an error:\n\n%@", error];
	NSAlert *alert = [[NSAlert alloc] init];
	[alert setMessageText:msg];
	[alert setAlertStyle:NSWarningAlertStyle];
	[alert runModal];
	[alert release];
}

+ (TiDocument*) getDocument:(TiWindow*)window
{
	return [[NSDocumentController sharedDocumentController] documentForWindow:window];
}

+ (WebView*) getWebView:(TiWindow*)window
{
	return [[TiController getDocument:window] webView];
}

- (void)loadApplicationXML
{
	NSString *appXMLPath = [[NSBundle mainBundle] pathForResource:@"tiapp" ofType:@"xml"];
	if ([arguments tiAppXml] != nil) 
	{
		appXMLPath = [arguments tiAppXml];
	}
	
	if (appXMLPath==nil)
	{
		[TiController error:@"Error loading tiapp.xml. It could not be found in the proper location."];
		[NSApp terminate:nil];
		return;
	}
	TRACE(@"Tiapp.xml found at %@",appXMLPath);
	
	NSError *error = nil;
	NSURL *furl = [NSURL fileURLWithPath:appXMLPath];

	TRACE(@"Tiapp.xml URL is %@",[furl absoluteString]);
	
	NSXMLDocument *doc = [[[NSXMLDocument alloc] initWithContentsOfURL:furl options:0 error:&error] autorelease];
	
	if (error) 
	{
		NSString* err = [NSString stringWithFormat:@"Error encountered loading tiapp.xml. ",[error localizedDescription]];
		[TiController error:@"Error loading tiapp.xml. It could not be found in the proper location."];
		[err release];
		[NSApp terminate:nil];
		return;
	}
	
	NSXMLElement *root = [doc rootElement];
	
	appName = [elementText(root, @"name") copy];
	appID = [elementText(root,@"id") copy];
	appVersion = [elementText(root,@"version") copy];
	appUpdateURL = [elementText(root,@"updatesite") copy];
	
	TRACE(@"Loading tiapp.xml - found appname = %@",[self appName]);
	
	//	if (containsElement(root, @"appc:endpoint")) 
	//	{
	//		[self setEndpoint:elementText(root, @"appc:endpoint")];
	//	}
	
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
		
		TRACE(@"initial window: %fx%f, min: %fx%f, max: %fx%f\n",width,height,minWidth,minHeight,maxWidth,maxHeight);
		
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
		
		TiWindowConfig *options = [[TiWindowConfig alloc] init];
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
		
		[windowConfigs addObject:options];
		[options release]; // add retains
	}
}

- (BOOL)shouldOpenInNewWindow
{
	// if we only have one window spec, we don't need to open new urls in a 
	// new document, we can just reuse the existing document on each load
	return [windowConfigs count] > 1;
}

- (void)loadApplicationID
{
	// we load this generated file which was packaged with the application when it was created
	// this GUID is guaranteed to be unique for this app (and all apps)
	NSString *basePath = [[NSBundle mainBundle] resourcePath];
	NSString *resourcePath = [basePath stringByAppendingPathComponent:@"aid"];
	appGUID = [[[NSString stringWithContentsOfFile:resourcePath] stringByReplacingOccurrencesOfString:@"\n" withString:@""] copy];
	TRACE(@"application ID: %@",appGUID);
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app
{
	return NO;
}

+ (NSString*) applicationID
{
	return [[TiController instance] appID];
}

+ (NSString*) applicationGUID
{
	return [[TiController instance] appGUID];
}

+ (NSString*) applicationVersion
{
	return [[TiController instance] appVersion];
}


+ (NSString*) applicationUpdateURL
{
	return [[TiController instance] appUpdateURL];
}

+ (NSURL*) formatURL: (NSString*)str 
{
	// if it looks like a URL, just use it
	if ([str hasPrefix:@"app:"] || [str hasPrefix:@"http://"] || [str hasPrefix:@"https://"] || [str hasPrefix:@"ti://"])
	{
		return [NSURL URLWithString:str];
	}
	if ([str hasPrefix:@"/"])
	{
		return [NSURL URLWithString:[NSString stringWithFormat:@"app://%@%@",[TiController applicationID],str]];
	}
	return [NSURL URLWithString:[NSString stringWithFormat:@"app://%@/%@",[TiController applicationID],str]];
}

@end
