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

static FILE* fd;
static bool console = false;

void TiLog(const char *log)
{
	if (console)
	{
		printf(log);
		fflush(stdout);
	}
	else if (fd)
	{
		fprintf(fd, log);
		fflush(fd);
	}
}
	
void TiSetupLog(int argc, const char *argv[], const char *dir)
{
	for (int c=1;c<argc;c++)
	{
		const char *e = argv[c];
		if (strstr(e,"--debug"))
		{
			console = true;
			return;
		}
	}
	printf("Application logging will go to file in %s\n",dir);
	char path[512];
	sprintf(path,"%s/ti.log",dir);
	fd = fopen(path, "w+");
}

void TiCloseLog()
{
	if (fd)
	{
		fflush(fd);
		fclose(fd);
	}
}


int main(int argc, const char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *path = [NSString stringWithFormat:@"%@/Contents/Log",[[NSBundle mainBundle] bundlePath]];
	[[NSFileManager defaultManager] createDirectoryAtPath:path attributes:nil];
	TiSetupLog(argc, argv, [path UTF8String]);
    int rc = NSApplicationMain(argc, argv);
	TiCloseLog();
	[pool release];
	return rc;
}
