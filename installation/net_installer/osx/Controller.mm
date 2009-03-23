/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#import "Controller.h"
#import <string>
#import "file_utils.h"
#import <zlib.h>

#if !USEURLREQUEST
#import "CURLHandle.h"
#import "CURLHandle+extras.h"
#endif


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

-(NSString*)installDirectory
{
	return installDirectory;
}

-(void)install:(NSString *)file destination:(NSString*)dir
{
	NSArray *parts = [[file lastPathComponent] componentsSeparatedByString:@"-"];
	if ([parts count] == 3)
	{
		NSString *type = [parts objectAtIndex:0];
		NSString *subtype = [parts objectAtIndex:1];
		NSString *version = [[parts objectAtIndex:2] stringByDeletingPathExtension];
		/**
		 * directories:
		 *
		 * /runtime/<version>/<files>
		 * /modules/<name>/<version>
		 */
		NSString *destdir = nil;
		if ([type isEqualToString:@"runtime"])
		{
			destdir = [NSString stringWithFormat:@"%@/runtime/%@/%@",dir,subtype,version];
		}
		else if ([type isEqualToString:@"module"])
		{
			destdir = [NSString stringWithFormat:@"%@/modules/%@/%@",dir,subtype,version];
		}
		if (destdir)
		{
			[[NSFileManager defaultManager]createDirectoryAtPath:destdir attributes:nil];
			std::string src([file UTF8String]);
			std::string dest([destdir UTF8String]);
			kroll::FileUtils::Unzip(src,dest);
		}
	}
}
-(void)download:(Controller*)controller 
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSArray *u = [controller urls];
	int count = [u count];
	NSString *dir = [controller directory];
	NSMutableArray *files = [[[NSMutableArray alloc] init] autorelease];
	NSProgressIndicator *progressBar = [controller progress];
	
	for (int c=0;c<count;c++)
	{
		NSURL *url = [u objectAtIndex:c];
		[controller updateMessage:[NSString stringWithFormat:@"Downloading %d of %d",c+1,count]];
		Downloader *downloader = [[Downloader alloc] initWithURL:url progress:progressBar];
		while ([downloader isDownloadComplete] == NO)
		{
			[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]]; // this could be more elegant, but it works
		}
		NSString *filename = [[url path] lastPathComponent];
		NSData *data = [downloader data];
		NSString *path = [NSString stringWithFormat:@"%@/%@",dir,filename];
		// write out our data
		[data writeToFile:path atomically:YES];
		[downloader release];
		[files addObject:path];
	}
	
	[progressBar setIndeterminate:NO];
	[progressBar setMinValue:0.0];
	[progressBar setMaxValue:[files count]];
	[progressBar setDoubleValue:0.0];
	[controller updateMessage:[NSString stringWithFormat:@"Installing %d file%s",[files count],[files count]>1?"s":""]];
	
	NSString *destination = [controller installDirectory];
#ifdef DEBUG
	NSLog(@"installing to: %@, count: %d",destination,[files count]);
#endif
	
	for (int c=0;c<(int)[files count];c++)
	{
		[progressBar setDoubleValue:c+1];
		[controller updateMessage:[NSString stringWithFormat:@"Installing %d of %d file%s",c+1,[files count],[files count]>1?"s":""]];
		[controller install:[files objectAtIndex:c] destination:destination];
	}
	[progressBar setDoubleValue:[files count]];

	[controller updateMessage:@"Installation complete"];
	
	[pool release];
	[NSApp terminate:self];
}

-(void)dealloc
{
	[urls release];
	[directory release];
	[installDirectory release];
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
	
	NSBeep();

	NSProcessInfo *p = [NSProcessInfo processInfo];
	NSArray *args = [p arguments];
#ifdef DEBUG
	NSLog(@"arguments = %@",args);
#endif
	int count = [args count];
	
	NSString *appname = count > 1 ? [args objectAtIndex:1] : @"Application Installer";
	NSString *title = count > 2 ? [args objectAtIndex:2] : @"Additional application libraries required";
	NSString *message = count > 3 ? [args objectAtIndex:3] : @"The application needs to download additional libraries to continue.";
	
	NSString *appTitle = [NSString stringWithFormat:@"%@ Installer",appname];
	
	// dynamically set the window based on the name of the app
	[window setTitle:appTitle];
	
	// figure out where the caller wants us to write the files once download
	if (count >5) {
		directory = [[args objectAtIndex:4] stringByExpandingTildeInPath];
		[directory retain];

		// figure out where the caller wants us to install once download
		installDirectory = [[args objectAtIndex:5] stringByExpandingTildeInPath];
		[installDirectory retain];
	} else {
		NSRunCriticalAlertPanel(title, @"Sorry, but the Application Installer was not given enough information to determine which libraries need downloading.", @"Cancel", nil, nil);
		[NSApp terminate:nil];
		return;
	}

	
	NSFileManager *fm = [NSFileManager defaultManager];
	BOOL dir = NO;
	if (![fm fileExistsAtPath:directory isDirectory:&dir] && dir == NO)
	{
		[fm createDirectoryAtPath:directory attributes:nil];
	}
	
	// slurp in the URLS
	urls = [[NSMutableArray alloc] init];
	for (int c=6;c<count;c++)
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

	[NSApp arrangeInFront:window];
	[window makeKeyAndOrderFront:window];
	[NSApp activateIgnoringOtherApps:YES];
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
#if !USEURLREQUEST
	[CURLHandle curlHelloSignature:@"XxXx" acceptAll:YES];	// to get CURLHandle registered for handling URLs
#endif
	[NSThread detachNewThreadSelector:@selector(download:) toTarget:self withObject:self];
}

#if !USEURLREQUEST
- (void) applicationWillTerminate:(NSNotification *) notif
{
	[CURLHandle curlGoodbye];	// to clean up
}
#endif

@end
