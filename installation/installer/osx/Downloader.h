/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#import <Cocoa/Cocoa.h>
#import "CURLHandle.h"


@interface Downloader :  NSObject<NSURLHandleClient> {
	CURLHandle *handle;
	NSProgressIndicator *progress;
	int bytesRetrievedSoFar;
	BOOL completed;
	NSData *data;
}
-(id)initWithURL:(NSURL*)url progress:(NSProgressIndicator*)p;
-(BOOL)isDownloadComplete;
-(NSData*)data;
-(NSString*)contentType;
@end
