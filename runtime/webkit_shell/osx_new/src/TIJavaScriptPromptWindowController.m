//
//  TIJavaScriptPromptWindowController.m
//  Titanium
//
//  Created by Todd Ditchendorf on 11/1/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "TIJavaScriptPromptWindowController.h"

@implementation TIJavaScriptPromptWindowController

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
