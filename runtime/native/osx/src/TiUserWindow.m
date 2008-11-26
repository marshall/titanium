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
#import "TiUserWindow.h"
#import "TiDocument.h"
#import "TiController.h"
 
@implementation TiUserWindow

- (id) initWithWindow:(TiWindow *)win
{
	self = [super init];
	if (self != nil)
	{
		window = win;
		[window retain];
	}
	return self;
}

- (void)dealloc 
{
	[window release];
	window = nil;
	[super dealloc];
}

- (NSString *)description 
{
	return @"[TiUserWindow native]";
}

- (NSString*)getTitle
{
	return [window title];
}

- (void)setTitle:(NSString*)title
{
	[window setTitle:title];
}

- (CGFloat)getTransparency
{
	return [window alphaValue];
}

- (void)setTransparency:(CGFloat)alphaValue
{
	[window setAlphaValue:alphaValue];
}

-(BOOL)isUsingChrome
{
	return [[window config] isChrome];
}


- (void)close 
{
	[[TiController getDocument:window] close];
}

- (void)hide {
	[window orderOut:nil]; // to hide it
}

- (void)show {
	[window makeKeyAndOrderFront:nil]; // to show it
}

+ (NSString *) webScriptNameForSelector:(SEL)sel{
	if (sel == @selector(show))
	{
		return @"show";
	}
	else if (sel == @selector(close))
	{
		return @"close";
	}
	else if (sel == @selector(hide))
	{
		return @"hide";
	}
	else if (sel == @selector(getTitle))
	{
		return @"getTitle";
	}
	else if (sel == @selector(setTitle:))
	{
		return @"setTitle";
	}
	else if (sel == @selector(setTransparency:))
	{
		return @"setTransparency";
	}
	else if (sel == @selector(getTransparency))
	{
		return @"getTransparency";
	}
	else if (sel == @selector(isUsingChrome))
	{
		return @"isUsingChrome";
	}
	return nil;
}

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)sel{
	return (nil == [self webScriptNameForSelector:sel]);
}

+ (BOOL)isKeyExcludedFromWebScript:(const char *)name{
	return YES;
}

@end
