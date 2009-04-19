/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#import "Controller.h"
#import <string>

#define RUNTIME_UUID_FRAGMENT @"uuid="RUNTIME_UUID
#define MODULE_UUID_FRAGMENT @"uuid="MODULE_UUID

@implementation Job
static int totalDownloads = 0;
static int totalJobs = 0;

-(void)dealloc
{
	if (url != nil)
		[url release];
	if (path != nil)
		[path release];
	[super dealloc];
}

-(Job*)initUpdate:(NSString*)pathOrURL
{
	self = [self init:pathOrURL];
	isUpdate = YES;
	return self;
}

-(Job*)init:(NSString*)pathOrURL
{
	self = [super init];

	NSFileManager *fm = [NSFileManager defaultManager];
	if ([fm fileExistsAtPath:pathOrURL])
	{
		url = nil;
		path = pathOrURL;
		[path retain];
	}
	else
	{
		url = [NSURL URLWithString:pathOrURL];
		[url retain];
		path = nil;
		totalDownloads++;
	}
	isUpdate = NO;
	totalJobs++;
	return self;
}

-(NSURL*)url
{
	return url;
}

-(NSString*)path
{
	return path;
}

-(void)setPath:(NSString*)newPath
{
	if (path != nil)
		[path release];
	path = newPath;
	[path retain];
}

-(BOOL)needsDownload
{
	return path == nil;
}

-(BOOL)isUpdate;
{
	return isUpdate;
}

-(int)totalDownloads;
{
	return totalDownloads;
}
-(int)totalJobs;
{
	return totalJobs;
}
@end

@implementation Controller

-(NSProgressIndicator*)progressBar
{
	return progressBar;
}

-(void)updateMessage:(NSString*)msg
{
	[progressText setStringValue:msg];
}


-(NSString*)temporaryDirectory
{
	return temporaryDirectory;
}

-(NSString*)installDirectory
{
	return installDirectory;
}

-(void)bailWithMessage:(NSString*)errorString;
{
	NSLog(@"Bailing with error: %@", errorString);
	NSRunCriticalAlertPanel(nil, errorString, @"Cancel", nil, nil);
	[NSApp terminate:nil];
}

-(void)createDirectory:(NSString*)path;
{
	NSFileManager* fm = [NSFileManager defaultManager];
	NSError* error = nil;
	BOOL isDirectory;
	BOOL exists = [fm fileExistsAtPath:path isDirectory:&isDirectory];

	if (!exists)
	{
		[fm createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:&error];
	}
	else if (!isDirectory)
	{
		NSString* msg = [NSString stringWithFormat:@"Installer tried to create the folder \"%@\", but found a file in its place.", path];
		[self performSelectorOnMainThread:@selector(bailWithMessage:) withObject:msg waitUntilDone:YES];
	}

	if (error != nil)
	{
		NSString* msg = [NSString stringWithFormat:@"Installer tried to create the folder \"%@\", but encountered error: %@", path, error];
		[self performSelectorOnMainThread:@selector(bailWithMessage:) withObject:msg waitUntilDone:YES];
	}
}

-(void)install:(Job*)job 
{
	NSString* name = @"unknown";
	NSString* version = @"unknown";
	BOOL isModule = YES;

	NSString* path = [job path];
	NSURL* url = [job url];
	if (url == nil)
	{
		// The file is either in the format of module-modname-version.zip for a
		// module or runtime-version.zip for the runtime, so we need to split
		// on '-' and count the parts to figure out what it is.
		NSArray* fileParts = [path componentsSeparatedByString:@"/"];
		NSString* trimmed = [[fileParts lastObject] stringByDeletingPathExtension];
		NSArray* parts = [trimmed componentsSeparatedByString:@"-"];
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
			// Unknown file!
			return;
		}
	}
	else
	{
		NSArray* parts = [[[url query] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding] componentsSeparatedByString:@"&"];
		NSEnumerator * partsEnumerator = [parts objectEnumerator];
		NSString *thisPart;
		while ((thisPart = [partsEnumerator nextObject]))
		{
			printf("%s\n", [thisPart UTF8String]);
			printf("%s\n\n", [RUNTIME_UUID_FRAGMENT UTF8String]);
			if ([thisPart isEqualToString:RUNTIME_UUID_FRAGMENT])
			{
				printf("found runtime uuid fragment\n");
				isModule = NO;
			}
			else if ([thisPart isEqualToString:MODULE_UUID_FRAGMENT])
			{
				printf("found module uuid fragment\n");
				isModule = YES;
			}
			else if ([thisPart hasPrefix:@"name="])
			{
				name = [thisPart substringFromIndex:5];
			}
			else if ([thisPart hasPrefix:@"version="])
			{
				version = [thisPart substringFromIndex:8];
			}
		}
	}

	NSString* destDir;
	if (isModule) 
	{
		destDir = [NSString stringWithFormat:@"%@/modules/osx/%@/%@", installDirectory, name, version];
	}
	else // This is the runtime
	{
		destDir = [NSString stringWithFormat:@"%@/runtime/osx/%@", installDirectory, version];
	}

#ifdef DEBUG	
	NSLog(@"name=%@,version=%@,module=%d",name,version,isModule);
#endif

	NSLog(@"Installing %@ into %@", path, destDir);
	[self createDirectory:destDir];
	std::string cmdline = "/usr/bin/ditto --noqtn -x -k --rsrc ";
	cmdline+="\"";
	cmdline+=[path UTF8String];
	cmdline+="\" \"";
	cmdline+=[destDir UTF8String];
	cmdline+="\"";
	system(cmdline.c_str());

#ifdef DEBUG
	NSLog(@"After unzip %@ to %@", path, destDir);
#endif
}

-(void)downloadAndInstall:(Controller*)controller 
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	// Download only those jobs which actually need to be downloaded
	int numJobs = (int) [jobs count];
	int current = 0;
	for (int i = 0; i < numJobs; i++)
	{
		Job* job = [jobs objectAtIndex:i];
		if ([job needsDownload])
		{
			current++;
			[self downloadJob:job atIndex:current];
		}
	}

	[progressBar setIndeterminate:NO];
	[progressBar setMinValue:0.0];
	[progressBar setMaxValue:[jobs count]];
	[progressBar setDoubleValue:0.0];

	const char* ext = numJobs > 1 ? "s" : "";
	[controller updateMessage:[NSString stringWithFormat:@"Installing %d file%s", numJobs, ext]];
	for (int i = 0; i < numJobs; i++)
	{
		Job* job = [jobs objectAtIndex:i];
		[progressBar setDoubleValue:current];
		[controller updateMessage:[NSString stringWithFormat:@"Installing %d of %d file%s", i, numJobs, ext]];
		[controller install:job];
	}

	[progressBar setDoubleValue:numJobs];
	[controller finishInstallation];
	[controller updateMessage:@"Installation complete"];
	NSLog(@"Installation is complete, exiting after installing %d files", numJobs);
	[pool release];
	[NSApp terminate:self];
}

-(void)finishInstallation
{
	// Write the .installed file
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString* ifile = [NSString stringWithUTF8String:app->path.c_str()];
	ifile  = [ifile stringByAppendingPathComponent:@".installed"];
	[fm createFileAtPath:ifile contents:[NSData data] attributes:nil];

	// Remove the update file if it exists
	if (updateFile != nil)
	{
		[fm removeFileAtPath:updateFile handler:nil];
	}
}

-(void)downloadJob:(Job*)job atIndex:(int)index
{
	[self updateMessage:[NSString stringWithFormat:@"Downloading %d of %d", index, [job totalDownloads]]];
	Downloader *downloader = [[Downloader alloc] initWithURL:[job url] progress:progressBar];
	while ([downloader isDownloadComplete] == NO)
	{
		// this could be more elegant, but it works
		[NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	}

	NSString *filename = [downloader suggestedFileName];
	NSData *data = [downloader data];
	NSString* errorMsg = @"Some files failed to download properly. Cannot continue.";
	if ([filename length] <= 5)
	{
		NSLog(@"Error in handling url \"%@\":", [job url]);
		NSLog(@"File name is too small to specify a file. \"%@\" was made instead.",filename);
		[downloader release];
		[self performSelectorOnMainThread:@selector(bailWithMessage:) withObject:errorMsg waitUntilDone:YES];
	}
	else if ([data length] <= 100)
	{
		NSLog(@"Error in handling url \"%@\":", [job url]);
		NSString * dataString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
		NSLog(@"Response data was too small to be a file. Received \"%@\" instead.",dataString);
		[dataString release];
		[downloader release];
		[self performSelectorOnMainThread:@selector(bailWithMessage:) withObject:errorMsg waitUntilDone:YES];
	}
	else
	{
		NSString *path = [[self temporaryDirectory] stringByAppendingPathComponent:filename];
		[data writeToFile:path atomically:YES];
		[job setPath:path];
		[downloader release];
	}
}

-(void)dealloc
{

	if (app != NULL)
		delete app;
	[jobs release];
	[installDirectory release];

	if (temporaryDirectory != nil)
	{
		NSFileManager *fm = [NSFileManager defaultManager];
		[fm removeFileAtPath:temporaryDirectory handler:nil];
		[temporaryDirectory release];
	}

	if (updateFile != nil)
	{
		[updateFile release];
	}

	[super dealloc];
}

-(void)awakeFromNib
{ 
	app = NULL;
	[NSApp setDelegate:self];

	updateFile = nil;
	NSString *appPath = nil;
	NSString *runtimeHome = nil;
	jobs = [[NSMutableArray alloc] init];

	NSArray* args = [[NSProcessInfo processInfo] arguments];
	int count = [args count];
	for (int i = 1; i < count; i++)
	{
		NSString* arg = [args objectAtIndex:i];
		if ([arg isEqual:@"-appPath"] && count > i+1)
		{
			appPath = [args objectAtIndex:i+1];
			i++;
		}
		else if ([arg isEqual:@"-updateFile"] && count > i+1)
		{
			updateFile = [args objectAtIndex:i+1];
			[updateFile retain];
			i++;
		}
		else if ([arg isEqual:@"-runtimeHome"] && count > i+1)
		{
			runtimeHome = [args objectAtIndex:i+1];
			i++;
		}
		else
		{
			[jobs addObject:[[Job alloc] init: arg]];
		}
	}

	if (appPath == nil || runtimeHome == nil)
	{
		[self bailWithMessage:@"Sorry, but the installer was not given enough information to continue."];
	}

	if (updateFile == nil)
	{
		app = BootUtils::ReadManifest([appPath UTF8String]);
	}
	else
	{
		app = BootUtils::ReadManifestFile([updateFile UTF8String], [appPath UTF8String]);
	}
	NSString *appName, *appVersion, *appPublisher, *appURL, *appImage;
	appName = appVersion = appPublisher = appURL = @"Unknown";
	appImage = nil;

	if (!app->name.empty())
		appName = [NSString stringWithUTF8String:app->name.c_str()];

	if (!app->version.empty() && updateFile == nil)
	{
		appVersion = [NSString stringWithUTF8String:app->version.c_str()];
	}
	else if (!app->version.empty())
	{
		appVersion = [NSString stringWithUTF8String:app->version.c_str()];
		appVersion = [appVersion stringByAppendingString:@" (Update)"];
	}

	if (!app->publisher.empty())
		appPublisher = [NSString stringWithUTF8String:app->publisher.c_str()];
	if (!app->url.empty())
		appURL = [NSString stringWithUTF8String:app->url.c_str()];
	if (!app->image.empty())
		appImage = [NSString stringWithUTF8String:app->image.c_str()];

	[progressAppName setStringValue:appName];
	[introAppName setStringValue:appName];
	[progressAppVersion setStringValue:appVersion];
	[introAppVersion setStringValue:appVersion];
	[progressAppPublisher setStringValue:appPublisher];
	[introAppPublisher setStringValue:appPublisher];
	[progressAppURL setStringValue:appURL];
	[introAppURL setStringValue:appURL];

	if (appImage != nil)
	{
		NSImage* img = [[NSImage alloc] initWithContentsOfFile:appImage];
		if ([img isValid])
		{
			[progressImage setImage:img];
			[introImage setImage:img];
		}
	}

	NSString* licensePath = [appPath stringByAppendingPathComponent: @LICENSE_FILENAME];
	NSFileManager *fm = [NSFileManager defaultManager];
	if ([fm fileExistsAtPath:licensePath])
	{
		NSString* licenseText = [NSString
			stringWithContentsOfFile: licensePath
			encoding:NSUTF8StringEncoding
			error:nil];
		NSAttributedString* licenseAttrText = [[NSAttributedString alloc] initWithString:licenseText];
		[[introLicenseText textStorage] setAttributedString:licenseAttrText];
		[introLicenseText setEditable:NO];
	}
	else
	{
		[introLicenseLabel setHidden:YES];
		[introLicenseBox setHidden:YES];
		NSRect frame = [introWindow frame];
		frame.size.width = 525;
		frame.size.height = 225;
		[introWindow setFrame:frame display:YES];
		[introWindow setMinSize:frame.size];
		[introWindow setMaxSize:frame.size];
		[introWindow setShowsResizeIndicator:NO];
	}

	std::string tempDir = FileUtils::GetTempDirectory();
	temporaryDirectory = [NSString stringWithUTF8String:tempDir.c_str()];
	[self createDirectory: temporaryDirectory];
	[temporaryDirectory retain];

	installDirectory = runtimeHome;
	[installDirectory retain];

	[NSApp arrangeInFront:introWindow];
	[progressWindow makeKeyAndOrderFront:progressWindow];
	[NSApp activateIgnoringOtherApps:YES];
}

-(void)applicationDidFinishLaunching:(NSNotification *) notif
{
	[progressWindow orderOut:self];
	[introWindow center];
}

-(IBAction)cancelProgress: (id)sender
{
	[progressCancelButton setEnabled:NO];
	[progressBar setDoubleValue:100.0];
	[progressText setStringValue:@"Cancelling..."];
	[NSApp terminate:self];
}
-(IBAction)cancelIntro:(id)sender
{
	[NSApp terminate:self];
}

-(IBAction)continueIntro:(id)sender;
{
	[introWindow orderOut:self];
	[progressText setStringValue:@"Connecting to download site..."];
	[progressBar setUsesThreadedAnimation:NO];
	[progressBar setIndeterminate:NO];
	[progressBar setMinValue:0.0];
	[progressBar setMaxValue:100.0];
	[progressBar setDoubleValue:0.0];

	if ([jobs count] > 0)
		[progressWindow orderFront:self];
	[progressWindow center];
	[NSThread detachNewThreadSelector:@selector(downloadAndInstall:) toTarget:self withObject:self];
}

@end
