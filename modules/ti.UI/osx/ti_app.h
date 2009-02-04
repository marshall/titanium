/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "preinclude.h"
#import <Cocoa/Cocoa.h>
#import "app_protocol.h"
#import "ti_protocol.h"
#import "../ui_module.h"

@interface TiApplication : NSObject
{
	NSString *appid;
}
+(NSURL*)normalizeURL:(NSString *)url;
+(NSString*)appID;

@end


void OSXInitialize();
