/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import "ti_app.h"

@implementation TiApplication
+(NSString*)appID
{
	AppConfig *config = AppConfig::Instance();
	return [NSString stringWithCString:config->GetAppID().c_str()];
}
+(NSURL*)normalizeURL:(NSString *)url
{
	NSURL *nsurl = [NSURL URLWithString:url];
	NSString *urlString = [nsurl absoluteString];
	BOOL appurl = YES;
	
	if ([urlString hasPrefix:@"http://"] || [urlString hasPrefix:@"https://"])
	{
	    // allow absolute external URLs
	    appurl = NO;
	}
	if (appurl)
	{
		NSString *appID = [TiApplication appID];
		if ([urlString hasPrefix:@"app:"])
	  	{
	    	NSString *host = [nsurl host];
	    	urlString = [NSString stringWithFormat:@"app://%@/%@",appID,host];
	  	}
	  	else
	  	{
	    	urlString = [NSString stringWithFormat:@"app://%@/%@",appID,[urlString stringByStandardizingPath]];
	  	}
		nsurl = [NSURL URLWithString:urlString];
	}
	NSLog(@"LOADING URL => %@",nsurl);
	return nsurl;
}
@end

void OSXInitialize()
{
	[TiProtocol registerSpecialProtocol];
	[AppProtocol registerSpecialProtocol];
	TiApplication *app = [[[TiApplication alloc] init] autorelease];
	NSApplication *nsapp = [NSApplication sharedApplication];
	[nsapp setDelegate:app];
	[NSBundle loadNibNamed:@"MainMenu" owner:nsapp];
}