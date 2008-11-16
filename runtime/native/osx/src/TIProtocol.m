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

#import "TIProtocol.h"

@implementation TIProtocol

+ (NSString*) specialProtocolScheme {
	return @"ti";
}

+ (void) registerSpecialProtocol {
	static BOOL inited = NO;
	if ( ! inited ) {
		[NSURLProtocol registerClass:[TIProtocol class]];
		inited = YES;
	}
}


+ (BOOL)canInitWithRequest:(NSURLRequest *)theRequest {
	
    /* get the scheme from the URL */
	NSString *theScheme = [[theRequest URL] scheme];
	
    /* return true if it matches the scheme we're using for our protocol. */
	return ([theScheme caseInsensitiveCompare: [TIProtocol specialProtocolScheme]] == NSOrderedSame );
}


+(NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request {
    return request;
}

- (void)startLoading
{
    id<NSURLProtocolClient> client = [self client];
    NSURLRequest *request = [self request];

	NSURL *url = [request URL];
	NSString *s = [[url absoluteString] substringFromIndex:5];	// ti://
	NSString *resPath = [[NSBundle mainBundle] resourcePath];
	NSString *scriptPath = [resPath stringByAppendingPathComponent:s];
	NSData *data = [[NSData alloc] initWithContentsOfFile:scriptPath];

	NSString *ext = [scriptPath pathExtension];
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
	
	NSURLResponse *response = [[NSURLResponse alloc] initWithURL:url MIMEType:mime expectedContentLength:-1 textEncodingName:@"utf-8"];
	[client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:NSURLCacheStorageNotAllowed];
	[client URLProtocol:self didLoadData:data];
	[client URLProtocolDidFinishLoading:self];
	[response release];
	[data release];
}		

- (void)stopLoading
{
}


@end