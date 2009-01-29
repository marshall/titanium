/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#import "Controller.h"


@implementation Controller

-(void)simulate
{
	[progress setDoubleValue:[progress doubleValue]+1.0];
}

-(NSProgressIndicator*)progress
{
	return progress;
}

-(void)updateMessage:(NSString*)msg
{
	[textField setStringValue:msg];
}

-(NSArray*)urls
{
	return urls;
}

-(NSString*)directory
{
	return directory;
}

-(void)download:(Controller*)controller 
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSArray *u = [controller urls];
	int count = [u count];
	NSString *dir = [controller directory];
	
	for (int c=0;c<count;c++)
	{
		NSURL *url = [u objectAtIndex:c];
		[controller updateMessage:[NSString stringWithFormat:@"Downloading %d of %d",c+1,count]];
		Downloader *downloader = [[Downloader alloc] initWithURL:url progress:[controller progress]];
		while ([downloader isDownloadComplete] == NO)
		{
			[NSThread sleepForTimeInterval:0.2]; // this could be more elegant, but it works
		}
		NSString *filename = [[url path] lastPathComponent];
		NSData *data = [downloader data];
		NSString *path = [NSString stringWithFormat:@"%@/%@",dir,filename];
		// write out our data
		[data writeToFile:path atomically:YES];
		[downloader release];
	}
	
	[pool release];
	[NSApp terminate:self];
}

-(void)dealloc
{
	[urls release];
	[directory release];
	[super dealloc];
}

-(void)awakeFromNib
{ 
	[NSApp setDelegate:self];
	[textField setStringValue:@"Connecting to download site..."];
	[progress setUsesThreadedAnimation:NO];
	[progress setIndeterminate:NO];
	[progress setMinValue:0.0];
	[progress setMaxValue:100.0];
	[progress setDoubleValue:0.0];
	[window center];

	NSProcessInfo *p = [NSProcessInfo processInfo];
	NSArray *args = [p arguments];
	NSLog(@"arguments = %@",args);
 
	int count = [args count];
	
	NSString *appname = count > 1 ? [args objectAtIndex:1] : @"Application Installer";
	NSString *title = count > 2 ? [args objectAtIndex:2] : @"Additional application libraries required";
	NSString *message = count > 3 ? [args objectAtIndex:3] : @"The application needs to download additional libraries to continue.";
	
	NSString *appTitle = [NSString stringWithFormat:@"%@ Installer",appname];
	
	// dynamically set the window based on the name of the app
	[window setTitle:appTitle];
	
	// figure out where the caller wants us to write the files once download
	directory = [[args objectAtIndex:4] stringByExpandingTildeInPath];
	[directory retain];
	
	NSFileManager *fm = [NSFileManager defaultManager];
	BOOL dir = NO;
	if (![fm fileExistsAtPath:directory isDirectory:&dir] && dir == NO)
	{
		[fm createDirectoryAtPath:directory attributes:nil];
	}
	
	// slurp in the URLS
	urls = [[NSMutableArray alloc] init];
	for (int c=5;c<count;c++)
	{
		NSURL *url = [NSURL URLWithString:[args objectAtIndex:c]];
		[urls addObject:url];
	}

	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Continue"];
	[alert addButtonWithTitle:@"Cancel"];
	[alert setMessageText:title];
	[alert setInformativeText:message];
	[alert setAlertStyle:NSInformationalAlertStyle];
	
	if ([alert runModal] != NSAlertFirstButtonReturn) 
	{
		[NSApp terminate:nil];
		return;
	}
	[alert release];

	[window makeKeyAndOrderFront:nil];
}

-(IBAction)cancel:(id)sender
{
	[button setEnabled:NO];
	[progress setDoubleValue:100.0];
	[textField setStringValue:@"Cancelling..."];
	[NSApp terminate:self];
}

- (void) applicationDidFinishLaunching:(NSNotification *) notif
{
	[CURLHandle curlHelloSignature:@"XxXx" acceptAll:YES];	// to get CURLHandle registered for handling URLs
	[NSThread detachNewThreadSelector:@selector(download:) toTarget:self withObject:self];
}

- (void) applicationWillTerminate:(NSNotification *) notif
{
	[CURLHandle curlGoodbye];	// to clean up
}

@end
