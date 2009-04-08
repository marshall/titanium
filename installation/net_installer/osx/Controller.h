/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#import <Cocoa/Cocoa.h>
#import "Downloader.h"


@interface Controller : NSObject {
	IBOutlet NSTextField* textField;
	IBOutlet NSButton* button;
	IBOutlet NSProgressIndicator* progress;
	IBOutlet NSImageView* image;
	IBOutlet NSWindow* window;
	NSMutableDictionary *urls;
	NSMutableArray *files;
	NSString *directory;
	NSString *installDirectory;
}

-(IBAction)cancel:(id)sender;
-(void)updateMessage:(NSString*)msg;
-(NSMutableDictionary*)urls;
-(NSArray*)files;
-(NSString*)directory;
-(NSString*)installDirectory;
-(void)downloadAndInstall:(Controller*)controller;
-(void)install:(NSString*)file;
-(void)install:(NSString *)file forUrl:(NSURL *)url;
-(void)install:(NSString*)file isModule:(BOOL)isModule  withName:(NSString *)name withVersion:(NSString*)version;
-(void)download;

@end
