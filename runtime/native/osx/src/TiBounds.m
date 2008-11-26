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
#import "TiBounds.h"
#import <WebKit/WebKit.h>

@implementation TiBounds

@synthesize x;
@synthesize y;
@synthesize width;
@synthesize height;

- (NSString*)description
{
	return [NSString stringWithFormat:@"[Bounds {%f,%f,%f,%f}]",x,y,width,height];
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	return nil;
}


+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return YES;
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	if (strcmp(name, "width") == 0) {
		return @"width";
	}
	else if (strcmp(name, "height") == 0) {
		return @"height";
	}
	else if (strcmp(name, "x") == 0) {
		return @"x";
	}
	else if (strcmp(name, "y") == 0) {
		return @"y";
	}
	return nil;
}

@end

