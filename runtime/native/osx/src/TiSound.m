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
#import "TiSound.h"
#import <WebKit/WebKit.h>

@implementation TiSound

- (id)initWithScope:(WebScriptObject*)s url:(NSURL*)u
{
	self = [super initWithContentsOfURL:u byReference:NO];
	if (self!=nil)
	{
		scope = s;
		[scope retain];
		[self setDelegate:self];
	}
	return self;
}

- (void)dealloc
{
	[scope release];
	scope = nil;
	[callback release];
	callback = nil;
	[super dealloc];
}

- (void)onComplete:(WebScriptObject*)fn
{
	[callback release];
	callback = fn;
	if (callback)
	{
		[callback retain];
	}
}

- (NSString*)description
{
	return @"[TiSound native]";
}

- (void)play
{
	[super play];
}

- (void)pause
{
	[super pause];
}

-(void)resume
{
	[super resume];
}

-(void)stop
{
	[super stop];
}

-(BOOL)isPlaying
{
	return [super isPlaying];
}

-(BOOL)isLooping
{
	return [super loops];
}

-(void)setLooping:(BOOL)yn
{
	[super setLoops:yn];
}

-(void)setVolume:(CGFloat)volume
{
	[super setVolume:volume];
}

-(CGFloat)getVolume
{
	return [super volume];
}

-(void)sound:(NSSound*)sound didFinishPlaying:(BOOL)finished
{
	if (callback!=nil)
	{
		NSMutableArray *result = [[NSMutableArray alloc] init];
		NSNumber *n = [NSNumber numberWithBool:finished];
		[result addObject:scope]; // scope
		[result addObject:n]; // argument
		[callback callWebScriptMethod:@"call" withArguments:result];
		[result release];
		[n release];
	}
}

#pragma mark -
#pragma mark WebScripting

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel {
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (NSString *)webScriptNameForSelector:(SEL)sel 
{
	if (sel == @selector(play)) {
		return @"play";
	}
	else if (sel == @selector(pause)) {
		return @"pause";
	}
	else if (sel == @selector(resume)) {
		return @"resume";
	}
	else if (sel == @selector(stop)) {
		return @"stop";
	}
	else if (sel == @selector(setVolume:)) {
		return @"setVolume";
	}
	else if (sel == @selector(getVolume)) {
		return @"getVolume";
	}
	else if (sel == @selector(setLooping:)) {
		return @"setLooping";
	}
	else if (sel == @selector(getLooping)) {
		return @"getLooping";
	}
	else if (sel == @selector(onComplete:)) {
		return @"onComplete";
	}
	return nil;
}


+ (BOOL)isKeyExcludedFromWebScript:(const char*)key {
	return YES;
}


+ (NSString *)webScriptNameForKey:(const char *)name {
	return nil;
}


@end