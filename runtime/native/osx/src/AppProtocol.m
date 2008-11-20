//
//  AppProtocol.m
//  Titanium
//
//  Created by Marshall on 11/18/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import "AppProtocol.h"
#import "TiAppArguments.h"
#import "TiAppDelegate.h"
#include <unistd.h>

@implementation AppProtocol

+ (NSString*) specialProtocolScheme {
	return @"app";
}


+ (void) registerSpecialProtocol {
	static BOOL inited = NO;
	if ( ! inited ) {
		[NSURLProtocol registerClass:[AppProtocol class]];
		inited = YES;
	}
}

+ (BOOL)canInitWithRequest:(NSURLRequest *)theRequest {
	
    /* get the scheme from the URL */
	NSString *theScheme = [[theRequest URL] scheme];
	
    /* return true if it matches the scheme we're using for our protocol. */
	return ([theScheme caseInsensitiveCompare: [AppProtocol specialProtocolScheme]] == NSOrderedSame );
}


+(NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request {
    return request;
}

+ (NSString *)mimeTypeFromExtension:(NSString *)ext
{
	NSString *mime = @"application/octet-stream";
	
	if ([ext isEqualToString:@"png"])
	{
		mime = @"image/png";
	}
	else if ([ext isEqualToString:@"gif"])
	{
		mime = @"image/gif";
	}
	else if ([ext isEqualToString:@"jpg"])
	{
		mime = @"image/jpeg";
	}
	else if ([ext isEqualToString:@"jpeg"])
	{
		mime = @"image/jpeg";
	}
	else if ([ext isEqualToString:@"ico"])
	{
		mime = @"image/x-icon";
	}
	else if ([ext isEqualToString:@"html"])
	{
		mime = @"text/html";
	}
	else if ([ext isEqualToString:@"htm"])
	{
		mime = @"text/html";
	}
	else if ([ext isEqualToString:@"text"])
	{
		mime = @"text/plain";
	}
	else if ([ext isEqualToString:@"js"])
	{
		mime = @"text/javascript";
	}
	return mime;
}

- (void)startLoading
{
    id<NSURLProtocolClient> client = [self client];
    NSURLRequest *request = [self request];
	
	NSURL *url = [request URL];
	NSString *s = [[url absoluteString] substringFromIndex:[[AppProtocol specialProtocolScheme] length]+3];	// ti://
	NSString *basePath = [[NSBundle mainBundle] resourcePath];
	NSString *resourcePath = [basePath stringByAppendingPathComponent:s];
	
	TiAppArguments *args = (TiAppArguments *)[[TiAppDelegate instance] arguments];
	
	if ([args devLaunch]) {
		char *cwd = getcwd(NULL, 0);
		
		NSString *currentPath = [NSString stringWithCString:cwd length:strlen(cwd)];
		resourcePath = [currentPath stringByAppendingPathComponent:s];
	}
	
	NSData *data = [[NSData alloc] initWithContentsOfFile:resourcePath];
	
	NSString *ext = [resourcePath pathExtension];
	NSString *mime = [AppProtocol mimeTypeFromExtension:ext];
	
	NSURLResponse *response = [[NSURLResponse alloc] initWithURL:url MIMEType:mime expectedContentLength:-1 textEncodingName:@"utf-8"];
	[client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:NSURLCacheStorageAllowed];
	[client URLProtocol:self didLoadData:data];
	[client URLProtocolDidFinishLoading:self];
	[response release];
	[data release];
}

- (void)stopLoading {
	
}

@end
