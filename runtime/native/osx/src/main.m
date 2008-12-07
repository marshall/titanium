/**
 * This file is part of Appcelerator's Titanium project.
 *
 * Copyright 2008 Appcelerator, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */

#import <Cocoa/Cocoa.h>


NSFileHandle *tiLogger;

//
// this method is called by the TRACE macro and shouldn't be (generally)
// called directly
//
void TiLog(NSString *message)
{
	if (tiLogger)
	{
		NSData *data = [message dataUsingEncoding: NSUTF8StringEncoding];
		[tiLogger writeData:data];
	}
}

//
// setup the log stream
//
void TiSetupLog(int argc, const char *argv[], NSString *path)
{
	for (int c=1;c<argc;c++)
	{
		const char *e = argv[c];
		if (strstr(e,"--console"))
		{
			// we want to log to stdout in the case of console
			tiLogger = [NSFileHandle fileHandleWithStandardOutput];
			return;
		}
	}
	// ensure that the file is available 
	[[NSFileManager defaultManager] createFileAtPath:path contents:@"" attributes:nil];
	// open it
	tiLogger = [NSFileHandle fileHandleForWritingAtPath:path];
}

//
// close the log stream
// 
void TiCloseLog()
{
	[tiLogger synchronizeFile];
	[tiLogger closeFile];
	[tiLogger release];
}


int main(int argc, const char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *path = [NSString stringWithFormat:@"%@/Contents/Log",[[NSBundle mainBundle] bundlePath]];
	[[NSFileManager defaultManager] createDirectoryAtPath:path attributes:nil];
	TiSetupLog(argc, argv, [NSString stringWithFormat:@"%@/ti.log",path]);
    int rc = NSApplicationMain(argc, argv);
	TiCloseLog();
	[pool release];
	return rc;
}
