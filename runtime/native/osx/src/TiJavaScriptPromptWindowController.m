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


#import "TiJavaScriptPromptWindowController.h"

@implementation TiJavaScriptPromptWindowController

- (id)initWithWindowNibName:(NSString *)nibName {
	self = [super initWithWindowNibName:nibName];
	if (self != nil) {
		
	}
	return self;
}


- (void)dealloc {
	[self setLabelText:nil];
	[self setUserText:nil];
	[super dealloc];
}


- (IBAction)close:(id)sender {
	[NSApp stopModalWithCode:[sender tag]];
}


- (IBAction)showWindow:(id)sender {
	[super showWindow:sender];
	[[self window] center];
	[textView selectAll:self];
}


- (NSString *)labelText {
	return [[labelText copy] autorelease];
}


- (void)setLabelText:(NSString *)s {
	if (labelText != s) {
		[labelText autorelease];
		labelText = [s copy];
	}
}

- (NSString *)userText {
	return [[userText copy] autorelease];
}


- (void)setUserText:(NSString *)s {
	if (userText != s) {
		[userText autorelease];
		userText = [s copy];
	}
}

@end
