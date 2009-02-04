/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import "ti_app.h"

@implementation TiApplication
@end

static TiApplication* app = [[TiApplication alloc] init];

void OSXInitialize()
{
	[TiProtocol registerSpecialProtocol];
	[AppProtocol registerSpecialProtocol];
	NSApplication *nsapp = [NSApplication sharedApplication];
	[nsapp setDelegate:app];
	[NSBundle loadNibNamed:@"MainMenu" owner:nsapp];
}