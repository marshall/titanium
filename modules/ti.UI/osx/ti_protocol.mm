/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "ti_protocol.h"
#import "app_protocol.h"

@implementation TiProtocol

+ (NSString*) specialProtocolScheme {
	return @"ti";
}

+ (void) registerSpecialProtocol {
	static BOOL inited = NO;
	if ( ! inited ) {
		[NSURLProtocol registerClass:[TiProtocol class]];
		// SECURITY FLAG: this will allow apps to have the same security
		// as local files (like cross-domain XHR requests).  we should 
		// make sure this is part of the upcoming security work
		[WebView registerURLSchemeAsLocal:[self specialProtocolScheme]];
		inited = YES;
	}
}

+ (BOOL)canInitWithRequest:(NSURLRequest *)theRequest 
{
	NSString *theScheme = [[theRequest URL] scheme];
	return [theScheme isEqual:@"ti"];
}

+(NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request {
    return request;
}

- (void)startLoading
{
    id<NSURLProtocolClient> client = [self client];
    NSURLRequest *request = [self request];
	
	NSURL *url = [request URL];
	NSString *ts = [url absoluteString];
	NSString *s = nil;
	if ([ts hasPrefix:@"ti://"])
	{
		s=[ts substringFromIndex:[[TiProtocol specialProtocolScheme] length]+3];	
	}
	else if ([ts hasPrefix:@"ti:/"])
	{
		s=[ts substringFromIndex:[[TiProtocol specialProtocolScheme] length]+2];	
	}
	else if ([ts hasPrefix:@"ti:"])
	{
		s=[ts substringFromIndex:[[TiProtocol specialProtocolScheme] length]+1];	
	}
	NSString *basePath = [NSString stringWithFormat:@"%s/Resources",getenv("KR_HOME")];
	basePath = [basePath stringByAppendingPathComponent:@"titanium"];
	NSString *resourcePath = nil;
	
	//TODO: this class still needs implementation
	KR_UNUSED(basePath);
	
	NSData *data = nil;
	NSString *mime = nil;
	BOOL needToReleaseData = NO;
	
	// support for loading app://notification which is the notification template
	if ([ts hasPrefix:@"ti://notification/"])
	{
		resourcePath = [basePath stringByAppendingPathComponent:@"notification.html"];
		mime = @"text/html";
	}
	else
	{
		
		// TiAppArguments *args = (TiAppArguments *)[[TiController instance] arguments];
		// 
		// if ([args devLaunch]) {
		// 	if ([s rangeOfString:@"plugin/"].location == 0) {
		// 		resourcePath = [[args pluginPath:pluginInfo.pluginName] stringByAppendingPathComponent:pluginInfo.pluginResource];
		// 	}
		// 	else if([s rangeOfString:@"plugins.js"].location == 0) {
		// 		// plugins.js is a virtual resource in dev launch mode , so we simulate it
		// 		NSString *content = @"ti.plugins=[];\n";
		// 		
		// 		NSEnumerator *pluginEnum = [args plugins];
		// 		id object;
		// 		while ((object = [pluginEnum nextObject])) {
		// 			content = [content stringByAppendingString:@"ti.App.include(\"ti://plugin/"];
		// 			content = [content stringByAppendingString:(NSString *)object];
		// 			content = [content stringByAppendingString:@"/plugin.js\");\n"];
		// 		}
		// 		
		// 		data = [content dataUsingEncoding:NSUTF8StringEncoding];
		// 		mime = @"text/javascript";
		// 	} else {
		// 		resourcePath = [[args runtimePath] stringByAppendingPathComponent:s];
		// 	}
		// } else if (resourcePath == nil) {
		// 	resourcePath = [basePath stringByAppendingPathComponent:s];
		// }
	}
	
	
	if (data == nil) {
		data = [[NSData alloc] initWithContentsOfFile:resourcePath];
		needToReleaseData = YES;
		NSString *ext = [resourcePath pathExtension];
		mime = [AppProtocol mimeTypeFromExtension:ext];
	}
	
	NSURLResponse *response = [[NSURLResponse alloc] initWithURL:url MIMEType:mime expectedContentLength:-1 textEncodingName:@"utf-8" ];
	[client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:NSURLCacheStorageAllowed];
	
	if (data != nil && [data length] > 0) {
		[client URLProtocol:self didLoadData:data];
	}
	
	[client URLProtocolDidFinishLoading:self];
	[response release];
	
	if (needToReleaseData)
		[data release];
}

- (void)stopLoading {
}

@end