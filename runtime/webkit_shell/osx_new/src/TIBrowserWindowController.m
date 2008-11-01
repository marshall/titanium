//
//  TIBrowserWindowController.m
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "TIBrowserWindowController.h"
#import "TIAppDelegate.h"
#import "TIBrowserDocument.h"
#import "TIJavaScriptObject.h"
#import <WebKit/WebKit.h>

typedef enum {
    WebNavigationTypePlugInRequest = WebNavigationTypeOther + 1
} WebExtraNavigationType;

@interface WebView (TIPrivateAdditions)
+ (BOOL)_canHandleRequest:(NSURLRequest *)request;
@end

@interface TIBrowserWindowController (Private)
- (void)openPanelDidEnd:(NSSavePanel *)openPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo;
@end

@implementation TIBrowserWindowController

- (id)initWithWindowNibName:(NSString *)nibName {
	self = [super initWithWindowNibName:nibName];
	if (self != nil) {
		
	}
	return self;
}


- (void)dealloc {	
	[super dealloc];
}


- (void)awakeFromNib {
	[[self window] center];
}


- (void)loadRequest:(NSURLRequest *)request {
	[[webView mainFrame] loadRequest:request];
}


- (void)handleLoadError:(NSError *)err {
	// TODO
	NSLog(@"Load Error: %@", err);
}


- (void)includeScript:(NSString*)path {
	DOMDocument *doc = [webView mainFrameDocument];
	if (!doc) return;

	DOMNodeList *headEls = [doc getElementsByTagName:@"head"];
	if (![headEls length]) return;
	
	DOMElement *headEl = (DOMElement *)[headEls item:0];
	if (!headEl) return;
	
	DOMElement *scriptEl = [doc createElement:@"script"];
	[scriptEl setAttribute:@"src" value:path];
	[scriptEl setAttribute:@"type" value:@"text/javascript"];
	//DOMText *text = [doc createTextNode:@""];
	//[script appendChild:text];

	[headEl appendChild:scriptEl];
}


#pragma mark -
#pragma mark WebFrameLoadDelegate

- (void)webView:(WebView *)sender didStartProvisionalLoadForFrame:(WebFrame *)frame {
	
}


- (void)webView:(WebView *)sender didFailProvisionalLoadWithError:(NSError *)error forFrame:(WebFrame *)frame {
	if (webView != sender) return;
	[self handleLoadError:error];
}


- (void)webView:(WebView *)sender didCommitLoadForFrame:(WebFrame *)frame {
	
}


- (void)webView:(WebView *)sender didReceiveTitle:(NSString *)title forFrame:(WebFrame *)frame {
	
}


- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame {
	
}


- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame {
	if (webView != sender) return;
	[self handleLoadError:error];
}


- (void)webView:(WebView *)sender windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject {
	if (webView != sender) return;
	TIJavaScriptObject *javaScriptObject = [[TIJavaScriptObject alloc] initWithWebView:webView];
	[windowScriptObject setValue:javaScriptObject forKey:@"TiNative"];
	[javaScriptObject include:@"titanium/titanium.js"];
	[javaScriptObject include:@"titanium/plugins.js"];
	[javaScriptObject release];
}


#pragma mark -
#pragma mark WebPolicyDelegate

- (void)webView:(WebView *)webView decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener {
    WebNavigationType navType = [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue];
	
    if ([WebView _canHandleRequest:request]) {
		[listener use];
    } else if (navType == WebNavigationTypePlugInRequest) {
        [listener use];
    } else {
		// A file URL shouldn't fall through to here, but if it did,
		// it would be a security risk to open it.
		if (![[request URL] isFileURL]) {
			[[NSWorkspace sharedWorkspace] openURL:[request URL]];
        }
		[listener ignore];
    }
}


- (void)webView:(WebView *)webView decidePolicyForNewWindowAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request newFrameName:(NSString *)frameName decisionListener:(id<WebPolicyDecisionListener>)listener {
	[listener use];
}


- (void)webView:(WebView *)webView decidePolicyForMIMEType:(NSString *)type request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener {
    id response = [[frame provisionalDataSource] response];
	
    if (response && [response respondsToSelector:@selector(allHeaderFields)]) {
        NSDictionary *headers = [response allHeaderFields];
        
        NSString *contentDisposition = [[headers objectForKey:@"Content-Disposition"] lowercaseString];
        if (contentDisposition && NSNotFound != [contentDisposition rangeOfString:@"attachment"].location) {
			if (![[[request URL] absoluteString] hasSuffix:@".user.js"]) { // don't download userscripts
				//[listener download];
				[listener ignore]; // ignoring for now. do we want a download manager?
				return;
			}
        }
        
        NSString *contentType = [[headers objectForKey:@"Content-Type"] lowercaseString];
        if (contentType && NSNotFound != [contentType rangeOfString:@"application/octet-stream"].location) {
			//[listener download];
			[listener ignore]; // ignoring for now. do we want a download manager?
            return;
        }
    }
	
	
    if ([[request URL] isFileURL]) {
        BOOL isDirectory = NO;
        [[NSFileManager defaultManager] fileExistsAtPath:[[request URL] path] isDirectory:&isDirectory];
        
        if (isDirectory) {
            [listener ignore];
        } else if ([WebView canShowMIMEType:type]) {
            [listener use];
        } else{
            [listener ignore];
        }
    } else if ([WebView canShowMIMEType:type]) {
        [listener use];
    } else {
		//[listener download];
		[listener ignore]; // ignoring for now. do we want a download manager?
    }
}


#pragma mark -
#pragma mark WebUIDelegate

- (WebView *)webView:(WebView *)sender createWebViewWithRequest:(NSURLRequest *)request {
	TIBrowserDocument *doc = [[TIAppDelegate instance] newDocumentWithRequest:request makeKey:YES];
	return [doc webView];
}


- (void)webView:(WebView *)sender runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
	NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
								 message,								// message
								 NSLocalizedString(@"OK", @""),			// default button
								 nil,									// alt button
								 nil);									// other button	
}


- (BOOL)webView:(WebView *)sender runJavaScriptConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
	NSInteger result = NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
													message,								// message
													NSLocalizedString(@"OK", @""),			// default button
													NSLocalizedString(@"Cancel", @""),		// alt button
													nil);
	return NSAlertDefaultReturn == result;	
}


- (NSString *)webView:(WebView *)sender runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText initiatedByFrame:(WebFrame *)frame {
	// TODO
	return nil;
}


- (BOOL)webView:(WebView *)sender runBeforeUnloadConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WebFrame *)frame {
	NSInteger result = NSRunInformationalAlertPanel(NSLocalizedString(@"JavaScript", @""),	// title
													message,								// message
													NSLocalizedString(@"OK", @""),			// default button
													NSLocalizedString(@"Cancel", @""),		// alt button
													nil);
	return NSAlertDefaultReturn == result;	
}


- (void)webView:(WebView *)sender runOpenPanelForFileButtonWithResultListener:(id <WebOpenPanelResultListener>)resultListener {
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	[openPanel beginSheetForDirectory:nil 
								 file:nil 
					   modalForWindow:[self window]
						modalDelegate:self
					   didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:) 
						  contextInfo:resultListener];	
}


- (void)openPanelDidEnd:(NSSavePanel *)openPanel returnCode:(int)returnCode contextInfo:(void *)contextInfo {
	id <WebOpenPanelResultListener>resultListener = (id <WebOpenPanelResultListener>)contextInfo;
	if (NSOKButton == returnCode) {
		[resultListener chooseFilename:[openPanel filename]];
	}
}


#pragma mark -
#pragma mark Accessors

- (WebView *)webView {
	return webView;
}

@end
