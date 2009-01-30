/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#import "Downloader.h"


@implementation Downloader

-(id)initWithURL:(NSURL*)url progress:(NSProgressIndicator*)p
{
	self = [super init];
	if (self)
	{
		progress = p;
		completed = NO;
		bytesRetrievedSoFar = 0;
		handle = (CURLHandle *)[url URLHandleUsingCache:NO];
		[handle retain];
		[handle setFailsOnError:YES];	
		[handle setFollowsRedirects:YES];
		[handle setConnectionTimeout:4];
		[handle setUserAgent:@"Mozilla/5.0 (compatible; Titanium_Downloader/0.2; Mac)"];
		[handle addClient:self];
		[handle setProgressIndicator:progress];
		[progress startAnimation:self];
		// directly call up the results
		[self URLHandleResourceDidFinishLoading:handle];
//		if (NSURLHandleLoadFailed == [handle status])
//		{
//			[oResultCode setStringValue:[mURLHandle failureReason]];
//		}
		
	}
	return self;
}
-(void)dealloc
{
	[handle release];
	[data release];
	[super dealloc];
}

-(NSData*)data
{
	return data;
}

-(BOOL)isDownloadComplete
{
	return completed;
}

- (void)URLHandle:(NSURLHandle *)sender resourceDataDidBecomeAvailable:(NSData *)newBytes
{
	id contentLength = [sender propertyForKeyIfAvailable:@"content-length"];
	bytesRetrievedSoFar += [newBytes length];
	
	if (nil != contentLength)
	{
		double total = [contentLength doubleValue];
		[progress setIndeterminate:NO];
		[progress setMaxValue:total];
		[progress setDoubleValue:bytesRetrievedSoFar];
	}
}

- (void)URLHandleResourceDidBeginLoading:(NSURLHandle *)sender
{
	completed = NO;
	[progress startAnimation:self];
}

- (void)URLHandleResourceDidFinishLoading:(NSURLHandle *)sender
{
	data = [handle resourceData];
	[data retain];
	[progress stopAnimation:self];
	[handle removeClient:self];
	completed = YES;
}

- (void)URLHandleResourceDidCancelLoading:(NSURLHandle *)sender
{
	[progress setIndeterminate:YES];
	[handle removeClient:self];	
	[progress stopAnimation:self];
	completed = YES;
}

- (void)URLHandle:(NSURLHandle *)sender resourceDidFailLoadingWithReason:(NSString *)reason
{
	[progress setIndeterminate:YES];
	[handle removeClient:self];
	[progress stopAnimation:self];
	completed = YES;
}

@end
