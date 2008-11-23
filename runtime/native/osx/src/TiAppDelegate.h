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

#import <Cocoa/Cocoa.h>
#import "TiWindowOptions.h"
#import "TiSplashScreenWindowController.h"
#import "TiAppArguments.h"

@class TiAppDelegate;
@class TiBrowserDocument;
@class TiBrowserWindowController;
@class WebFrame;

TiBrowserWindowController *TIFirstController();
TiBrowserWindowController *TIFrontController();

@interface TiAppDelegate : NSDocumentController {
	NSString *endpoint;
	NSString *appName;
	NSMutableArray *windowOptions;
	TiWindowOptions *activeWindowOption;
	TiSplashScreenWindowController *splashController;
	TiAppArguments *arguments;
}
+ (id)instance;

- (IBAction)showPreferencesWindow:(id)sender;
- (IBAction)showAboutWindow:(id)sender;
- (IBAction)showWebInspector:(id)sender;
- (IBAction)showErrorConsole:(id)sender;
- (IBAction)showNetworkTimeline:(id)sender;
- (IBAction)toggleFullScreen:(id)sender;

- (TiBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request display:(BOOL)display;
- (TiBrowserDocument *)newDocumentWithOptions:(NSURLRequest *)request options:(TiWindowOptions*)options;

- (void)error:(NSString *)error;

- (WebFrame *)findFrameNamed:(NSString *)frameName;

- (void)parseTiAppXML;
- (void)initDevEnvironment;

- (TiWindowOptions*) findInitialWindowOptions;
- (TiWindowOptions*) findWindowOptionsForURLSpec:(NSURLRequest*)urlrequest;

- (void)setActiveWindowOption:(TiWindowOptions*)o;
- (TiWindowOptions*)getActiveWindowOption;


//TODO: review these
- (NSString *)endpoint;
- (void)setEndpoint:(NSString *)s;
- (NSString *)appName;
- (void)setAppName:(NSString *)s;
- (TiAppArguments *)arguments;

@end
