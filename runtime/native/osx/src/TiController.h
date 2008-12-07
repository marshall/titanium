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
#import <WebKit/WebKit.h>
#import "Ti.h"
#import "TiDocument.h"
#import "TiAppArguments.h"
#import "TiWindowConfig.h"
#import "WebInspector.h"

@interface TiController : NSObject
{
	NSWindowController *splashController;
	TiAppArguments *arguments;
	NSMutableArray *windowConfigs;
	NSString *appName;
	NSString *appID;
	NSString *appGUID;
	NSString *appUpdateURL;
	NSString *appVersion; 
	TiWindowConfig *pendingConfig;
	TiDocument *activeDocument;
}
- (void)dealloc;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
- (void)loadApplicationID;
- (void)loadApplicationXML;

- (NSString*)appName;
- (NSString*)appID;
- (NSString*)appGUID;
- (NSString*)appVersion;
- (NSString*)appUpdateURL;
- (TiAppArguments*)arguments;
- (TiDocument*) createDocument:(NSURL*)url visible:(BOOL)visible config:(TiWindowConfig*)config;
- (BOOL)shouldOpenInNewWindow;
- (TiWindowConfig*) pendingConfig;
- (void) resetPendingConfig;
- (void)activate:(TiDocument*)doc;
- (void)deactivate:(TiDocument*)doc;

+ (void)error:(NSString*)message;
+ (TiController*) instance;
+ (TiDocument*) getDocument:(TiWindow*)window;
+ (WebView*) getWebView:(TiWindow*)window;
+ (NSString*) applicationID;
+ (NSString*) applicationGUID;
+ (NSString*) applicationVersion;
+ (NSString*) applicationUpdateURL;

+ (NSURL*) formatURL: (NSString*)str;
+ (void)documentOpened:(TiDocument*)doc;

@end
