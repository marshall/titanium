//
//  AppWindowController.m
//  webkit_shell
//
//  Created by Marshall on 9/30/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import "AppWindowController.h"

extern int argCount;
extern char** args;

@implementation AppWindowController

BOOL containsElement(NSXMLElement* el, NSString *name)
{
	return [[el elementsForName:name] count] > 0;
}

NSXMLElement* getElement(NSXMLElement* el, NSString* name)
{
	return [[el elementsForName:name] objectAtIndex:0];
}

NSString* elementText(NSXMLElement* el, NSString* name)
{
	return [getElement(el, name) stringValue];
}

NSString* attrText(NSXMLElement* el, NSString* name)
{
	return [[el attributeForName:name] stringValue];
}

- (void)parseTiAppXml
{
	NSError *error;
	NSString *appXmlPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingString:@"/tiapp.xml"];
	NSLog(@"loading xml: %@\n", appXmlPath);
	
	NSURL *url = [NSURL fileURLWithPath:appXmlPath];
	NSXMLDocument *doc = [[NSXMLDocument alloc] initWithContentsOfURL:url options:0 error:&error];
	if (error != NULL) {
		NSLog(@"error getting xml doc: %@\n", error);
		return;
	}
	
	NSXMLElement *root = [doc rootElement];
	
	NSLog(@"setting name\n");
	[titanium_js setAppName: elementText(root, @"name")];
	if (containsElement(root, @"appc:endpoint"))
	{
		NSLog(@"setting endpoint\n");
		[titanium_js setEndpoint:elementText(root, @"appc:endpoint")];
	}
	
	NSLog(@"getting window element\n");
	NSXMLElement *window = getElement(root, @"window");
	NSLog(@"getting width\n");
	int width = atoi([attrText(window, @"width") UTF8String]);
	NSLog(@"getting height\n");
	int height = atoi([attrText(window, @"height") UTF8String]);
	
	[titanium_js setWindowDimensions:width height:height];
	NSLog(@"getting title\n");
	[titanium_js setWindowTitle:elementText(window, @"title")];
	
	NSLog(@"getting start path\n");
	[titanium_js setStartPath:elementText(window, @"start")];
}

- (void)includeScript:(NSString*)path
{
	NSLog(@"include script: %@\n", path);
	
	//WebFrame* mainFrame = [webView mainFrame];
	DOMDocument *doc = [[webView windowScriptObject] valueForKey:@"document"];
	DOMNode *head = [[webView windowScriptObject] evaluateWebScript:@"document.getElementsByTagName('head')[0]"];
	
	NSLog(@"doc = %@\n", doc);
	
	DOMElement *script = [doc createElement:@"script"];
	[script setAttribute:@"src" value:path];
	[script setAttribute:@"type" value:@"text/javascript"];
	//DOMText *text = [doc createTextNode:@""];
	//[script appendChild:text];
	
	NSLog(@"head = %@\n", head);
	
	[head appendChild:script];
}

- (void)loadJsPlugins
{
	[titanium_js include:@"titanium/titanium.js"];
	[titanium_js include:@"titanium/plugins.js"];
}

- (void)webView:(WebView*)sender windowScriptObjectAvailable:(WebScriptObject*)windowScriptObject
{
    [windowScriptObject setValue: titanium_js forKey: @"TiNative"];
	[self loadJsPlugins];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	NSURL *url;
	titanium_js = [[TitaniumJS alloc] init];
	[titanium_js setWebView:webView];
	
	[self parseTiAppXml];
	
	NSString *relativeFile = [titanium_js getStartPath];
	if (relativeFile != nil)
	{
		NSString *fullPath = [[[[NSBundle mainBundle] resourcePath] stringByAppendingString:@"/"] stringByAppendingString:relativeFile];
		url = [NSURL fileURLWithPath:fullPath];
	}
	
	if (argCount == 3) {
		if (strcmp(args[1], "-file") == 0) {
			NSString *pathString = [[NSString alloc] initWithUTF8String:args[2]];
			url = [NSURL fileURLWithPath:pathString];
		}
		else if (strcmp(args[1], "-url") == 0) {
			NSString *urlString = [[NSString alloc] initWithUTF8String:args[2]];
			url = [NSURL URLWithString:urlString];
		}
	}
	
	NSURLRequest *request = [NSURLRequest requestWithURL:url];
	
	[webView setFrameLoadDelegate: self];
	[[webView mainFrame] loadRequest:request];

}

- (IBAction)loadApp:(id)sender {
	}

@end
