/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#import <Cocoa/Cocoa.h>
#import "Downloader.h"
#import "CURLHandle.h"
#import "CURLHandle+extras.h"


@interface Controller : NSObject {
	IBOutlet NSTextField* textField;
	IBOutlet NSButton* button;
	IBOutlet NSProgressIndicator* progress;
	IBOutlet NSImageView* image;
	IBOutlet NSWindow* window;
	NSMutableArray *urls;
	NSString *directory;
}

-(IBAction)cancel:(id)sender;
-(void)updateMessage:(NSString*)msg;
-(NSArray*)urls;
-(NSString*)directory;

@end
