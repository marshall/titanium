//
//  TiAppArguments.h
//  Titanium
//
//  Created by Marshall on 11/18/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface TiAppArguments : NSObject {
	NSArray *arguments;
	
	NSString *launchURL;
	NSString *tiAppXml;
	BOOL devLaunch;
	NSString *runtimePath;
	NSMutableDictionary *pluginPaths;
}

@property (copy,readwrite) NSString* launchURL;
@property (copy,readwrite) NSString* tiAppXml;
@property (readwrite) BOOL devLaunch;
@property (copy,readwrite) NSString* runtimePath;

-(id)init;
-(NSEnumerator *) plugins;
-(NSString*) pluginPath:(NSString*)pluginName;

@end
