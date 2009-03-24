/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#import <Cocoa/Cocoa.h>
#define USEURLREQUEST 1


#if USEURLREQUEST
@class CURLHandle;

@interface Downloader :  NSObject {
	CURLHandle *handle;
#else
@interface Downloader :  NSObject<NSURLHandleClient> {
#endif
	NSString * suggestedFileName;
	
	NSURLConnection * downloadConnection;
	NSProgressIndicator *progress;
	int bytesRetrievedSoFar;
	long long expectedBytes;
	BOOL completed;
	NSMutableData *data;
}
-(id)initWithURL:(NSURL*)url progress:(NSProgressIndicator*)p;
-(BOOL)isDownloadComplete;

- (BOOL)completed;
- (void)setCompleted:(BOOL)value;
- (NSString *)suggestedFileName;
- (void)setSuggestedFileName:(NSString *)value;
		
-(NSData*)data;
@end
