/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#import "Controller.h"
#import <string>

#define RUNTIME_UUID_FRAGMENT @"uuid=A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID_FRAGMENT @"uuid=1ACE5D3A-2B52-43FB-A136-007BD166CFD0"

@implementation Controller

-(NSProgressIndicator*)progress
{
	return progress;
}

-(void)updateMessage:(NSString*)msg
{
	[textField setStringValue:msg];
}

-(NSMutableDictionary*)urls
{
	return urls;
}

-(NSArray*)files
{
	return files;
}

-(NSString*)directory
{
	return directory;
}

-(NSString*)installDirectory
{
	return installDirectory;
}

-(void)bailWithMessage: (NSString *) errorString;
{
	NSLog(@"Bailing with error: %@",errorString);
	NSRunCriticalAlertPanel(nil, errorString, @"Cancel", nil, nil);
	[NSApp terminate:nil];
}

-(void)generateDirectory:(NSString *) newDirectoryPath;
{
	NSFileManager * theFM = [[NSFileManager alloc] init];
	BOOL isDirectory;
	BOOL isExistent = [theFM fileExistsAtPath:newDirectoryPath isDirectory:&isDirectory];
	if (!isExistent) {
		[self generateDirectory:[newDirectoryPath stringByDeletingLastPathComponent]];
		[theFM createDirectoryAtPath:newDirectoryPath attributes:nil];
	} else if (!isDirectory) {
		NSString * errorMessage = [NSString stringWithFormat:@"Installer tried to create the folder \"%@\", but found a file in its place.",newDirectoryPath];
		[self performSelectorOnMainThread:@selector(bailWithMessage:) withObject:errorMessage waitUntilDone:YES];
	}
}

-(void)install:(NSString*)file 
{
	NSArray* fileParts = [file componentsSeparatedByString:@"/"];
	NSString* trimmed = [[fileParts lastObject] stringByReplacingOccurrencesOfString:@".zip" withString: @""];
	NSArray* parts = [trimmed componentsSeparatedByString:@"-"];
	NSString* name;
	NSString* version;
	BOOL isModule;
	if ((int)[parts count] == 3)
	{
		// part 0 should be "module"
		isModule = YES;
		name = [parts objectAtIndex:1];
		version = [parts objectAtIndex:2];
	}
	else if ((int)[parts count] == 2)
	{
		isModule = NO;
		name = [parts objectAtIndex:0];
		version = [parts objectAtIndex:1];
	}
	else
	{
		NSLog(trimmed);
		NSLog(file);
		return;
	}
	[self install:file isModule:isModule withName:name withVersion:version];
}

-(void)install:(NSString*)file isModule:(BOOL)isModule  withName:(NSString *)name withVersion:(NSString*)version
{
	NSString* installDir = [self installDirectory];
	NSString* destDir;
	if (isModule) // this is a module
		destDir = [NSString stringWithFormat:@"%@/modules/osx/%@/%@", installDir, name, version];
	else // this is the runtime
		destDir = [NSString stringWithFormat:@"%@/runtime/osx/%@", installDir, version];

#ifdef DEBUG	
	NSLog(@"name=%@,version=%@,module=%d",name,version,isModule);
#endif
	NSLog(@"Installing %@ into %@", file, destDir);
	[self generateDirectory:destDir];
	std::string cmdline = "/usr/bin/ditto --noqtn -x -k --rsrc ";
	cmdline+="\"";
	cmdline+=[file UTF8String];
	cmdline+="\" \"";
	cmdline+=[destDir UTF8String];
	cmdline+="\"";
	system(cmdline.c_str());
#ifdef DEBUG
	NSLog(@"After unzip %@ to %@",file,destDir);
#endif

}

-(void)install:(NSString *)file forUrl:(NSURL *)url
{
	NSArray *parts = [[[url query] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding] componentsSeparatedByString:@"&"];
	BOOL isRuntime = NO;
	BOOL isModule = NO;
	NSString *subtype = nil;
	NSString *version = nil;
	
	NSEnumerator * partsEnumerator = [parts objectEnumerator];
	NSString *thisPart;
	while ((thisPart = [partsEnumerator nextObject])){
		if ([thisPart isEqualToString:RUNTIME_UUID_FRAGMENT]){
			isRuntime = YES;
			continue;
		}

		if ([thisPart isEqualToString:MODULE_UUID_FRAGMENT]){
			isModule = YES;
			continue;
		}
		
		if ([thisPart hasPrefix:@"name="]){
			subtype = [thisPart substringFromIndex:5];
			continue;
		}

		if ([thisPart hasPrefix:@"version="]){
			version = [thisPart substringFromIndex:8];
			continue;
		}
	}
	[self install:file isModule:isModule withName:subtype withVersion:version];

}

-(void)downloadAndInstall:(Controller*)controller 
{	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	int numURLs = (int) [[urls allKeys] count];
	int numFiles = (int) [files count];
	int count = numURLs + numFiles;
	int current = 0;

	if (numURLs > 0)
	{
		[controller download];
	}

	NSProgressIndicator *progressBar = [self progress];
	[progressBar setIndeterminate:NO];
	[progressBar setMinValue:0.0];
	[progressBar setMaxValue:count];
	[progressBar setDoubleValue:0.0];
	
	[controller updateMessage:[NSString stringWithFormat:@"Installing %d file%s", count, count>1?"s":""]];
	
	NSArray * urlsKeys = [urls allKeys];
	for (int c=0; c<(int)[urlsKeys count];c++)
	{
		count++;
		[progressBar setDoubleValue:current];
		[controller updateMessage:[NSString stringWithFormat:@"Installing %d of %d file%s", current, count, count>1?"s":""]];
		NSURL* url = [urlsKeys objectAtIndex:c];
		NSString* file = [urls objectForKey:url];
		[controller install:file forUrl:url];
	}

	for (int c = 0; c < (int)[files count]; c++)
	{
		current++;
		[progressBar setDoubleValue:current];
		[controller updateMessage:[NSString stringWithFormat:@"Installing %d of %d file%s", current, count, count>1?"s":""]];
		NSString* file = [files objectAtIndex:c];
		[controller install:file];
	}

	[progressBar setDoubleValue:count];
	[controller updateMessage:@"Installation complete"];
	NSLog(@"Installation is complete, exiting after installing %d files",count);

	[pool release];
	[NSApp terminate:self];
}

-(void)download
{
	NSArray *u = [[self urls] allKeys];
	int count = [u count];
	NSString *dir = [self directory];
	NSProgressIndicator *progressBar = [self progress];
	
	for (int c=0;c<count;c++)
	{
		NSURL *url = [u objectAtIndex:c];
		[self updateMessage:[NSString stringWithFormat:@"Downloading %d of %d",c+1,count]];
		Downloader *downloader = [[Downloader alloc] initWithURL:url progress:progressBar];
		while ([downloader isDownloadComplete] == NO)
		{
			[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]]; // this could be more elegant, but it works
		}
		NSString *filename = [downloader suggestedFileName];
		NSData *data = [downloader data];
		NSString *path = [NSString stringWithFormat:@"%@/%@",dir,filename];
		// write out our data
		BOOL isValidName = [filename length] > 5;
		BOOL isValidData = [data length] > 100;
		
		if (isValidData && isValidName) 
		{
			[data writeToFile:path atomically:YES];
			[[self urls] setObject:path forKey:url];
		} 
		else 
		{
			NSLog(@"Error in handling url \"%@\":",url);
			if (!isValidName) {
				NSLog(@"File name is too small to specify a file. \"%@\" was made instead.",filename);
			}
			if (!isValidData){
				NSString * dataString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
				NSLog(@"Response data was too small to be a file. Received \"%@\" instead.",dataString);
				[dataString release];
			}
			NSString* msg = @"Some files failed to download properly. Cannot continue.";
			[self performSelectorOnMainThread:@selector(bailWithMessage:) withObject:msg waitUntilDone:YES];
		}
		
		[downloader release];
	}
}

-(void)dealloc
{
	[urls release];
	[files release];
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
		[self bailWithMessage:@"Sorry, but the Application Installer was not given enough information to determine which libraries need downloading."];
		return;
	}

	
	NSFileManager *fm = [NSFileManager defaultManager];
	BOOL dir = NO;
	if (![fm fileExistsAtPath:directory isDirectory:&dir] && dir == NO)
	{
		[fm createDirectoryAtPath:directory attributes:nil];
	}
	
	BOOL allZips = YES;
	
	// slurp in the URLS
	urls = [[NSMutableDictionary alloc] init];
	files = [[NSMutableArray alloc] init];
	for (int c = 6; c < count; c++)
	{
		NSString* arg = [args objectAtIndex:c];
		if ([fm fileExistsAtPath:arg])
		{
			[files addObject: arg];
		}
		else
		{
			NSURL *url = [NSURL URLWithString:[args objectAtIndex:c]];
			[urls setObject:url forKey:url];
			allZips = NO;
		}
	}

	if (allZips == NO)
	{
		NSAlert *alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"Continue"];
		[alert addButtonWithTitle:@"Cancel"];
		[alert setMessageText:title];
		[alert setInformativeText:message];
		[alert setAlertStyle:NSInformationalAlertStyle];
		NSBeep();

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
	else
	{
		[NSApp arrangeInFront:window];
		[window makeKeyAndOrderFront:window];
	}
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
	[NSThread detachNewThreadSelector:@selector(downloadAndInstall:) toTarget:self withObject:self];
}

@end
