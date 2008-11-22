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

#import "TiProtocol.h"
#import "AppProtocol.h"
#import "TiAppArguments.h"
#import "TiAppDelegate.h"
#import "TiApp.h"

@implementation TiProtocol

+ (NSString*) specialProtocolScheme {
	return @"ti";
}
-(NSString *)getBasePath {
	return [[NSBundle mainBundle] resourcePath];
}

+ (void) registerSpecialProtocol {
	static BOOL inited = NO;
	if ( ! inited ) {
		[NSURLProtocol registerClass:[TiProtocol class]];
		inited = YES;
	}
}

+ (BOOL)canInitWithRequest:(NSURLRequest *)theRequest {
	
    /* get the scheme from the URL */
	NSString *theScheme = [[theRequest URL] scheme];
	
    /* return true if it matches the scheme we're using for our protocol. */
	return ([theScheme caseInsensitiveCompare: [TiProtocol specialProtocolScheme]] == NSOrderedSame );
}


+(NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request {
    return request;
}

typedef struct {
	NSString *pluginName;
	NSString *pluginResource;
} PluginResourceInfo;

- (PluginResourceInfo) getPluginResourceInfo:(NSString *)uri {
	
	PluginResourceInfo info;
	if ([uri rangeOfString:@"plugin/"].location == 0) {
		uri = [uri substringFromIndex:7];
		
		NSUInteger firstSlash = [uri rangeOfString:@"/"].location;
		info.pluginName = [uri substringToIndex:firstSlash];
		info.pluginResource = [uri substringFromIndex:firstSlash+1];
	}
	
	return info;
}

- (void)startLoading
{
    id<NSURLProtocolClient> client = [self client];
    NSURLRequest *request = [self request];
	
	NSURL *url = [request URL];
	NSString *s = [[url absoluteString] substringFromIndex:[[TiProtocol specialProtocolScheme] length]+3];
	NSString *basePath = [[NSBundle mainBundle] resourcePath];
	basePath = [basePath stringByAppendingPathComponent:@"titanium"];
	NSString *resourcePath = nil;
	PluginResourceInfo pluginInfo;
	
	// ti://plugin/plugin_id/path/to/resource
	if ([s rangeOfString:@"plugin/"].location == 0) {
		pluginInfo = [self getPluginResourceInfo:s];
		resourcePath = [[basePath stringByAppendingPathComponent:pluginInfo.pluginName] stringByAppendingPathComponent:pluginInfo.pluginResource];
	}
	
	TiAppArguments *args = (TiAppArguments *)[[TiAppDelegate instance] arguments];
	
	NSData *data = nil;
	NSString *mime = nil;
	BOOL needToReleaseData = NO;
	
	if ([args devLaunch]) {
		if ([s rangeOfString:@"plugin/"].location == 0) {
			resourcePath = [[args pluginPath:pluginInfo.pluginName] stringByAppendingPathComponent:pluginInfo.pluginResource];
		}
		else if([s rangeOfString:@"plugins.js"].location == 0) {
			// plugins.js is a virtual resource in dev launch mode , so we simulate it
			NSString *content = @"ti.plugins=[];\n";
			
			NSEnumerator *pluginEnum = [args plugins];
			id object;
			while ((object = [pluginEnum nextObject])) {
				content = [content stringByAppendingString:@"ti.App.include(\"ti://plugin/"];
				content = [content stringByAppendingString:(NSString *)object];
				content = [content stringByAppendingString:@"/plugin.js\");\n"];
			}
	
			data = [content dataUsingEncoding:NSUTF8StringEncoding];
			mime = @"text/javascript";
		} else {
			resourcePath = [[args runtimePath] stringByAppendingPathComponent:s];
		}
	} else if (resourcePath == nil) {
		resourcePath = [basePath stringByAppendingPathComponent:s];
	}
	
	if (data == nil) {
		NSLog(@"resolving url %@ to path: %@\n", [url absoluteString], resourcePath);
		
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