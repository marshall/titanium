/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import "ti_app.h"
#import "osx_ui_binding.h"
#import "osx_menu_item.h"

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
	  	if ([nsurl query] != nil) {
	  		urlString = [NSString stringWithFormat:@"%@?%@",urlString,[nsurl query]];
	  	}
	  	
		nsurl = [NSURL URLWithString:urlString];
	}
	NSLog(@"LOADING URL => %@",nsurl);
	return nsurl;
}
- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	OSXUIBinding *ui = static_cast<OSXUIBinding*>(binding);
	SharedPtr<MenuItem> item = ui->GetDockMenu();
	if (!item.isNull())
	{
		SharedPtr<ti::OSXMenuItem> osx_menu = item.cast<ti::OSXMenuItem>();
		NSMenu *menu = ti::OSXUIBinding::MakeMenu(osx_menu);
		[menu setAutoenablesItems:NO];
		[menu autorelease];
		return menu;
	}
	return nil;
}
- (id)initWithBinding:(ti::UIBinding*)b
{
	self = [super init];
	if (self)
	{
		binding = b;
	}
	return self;
}
@end

