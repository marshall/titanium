//
//  TIPreferencesWindowController.m
//  Titanium
//
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "TIPreferencesWindowController.h"

@implementation TIPreferencesWindowController

+ (id)instance {
	static TIPreferencesWindowController *instance = nil;

	@synchronized (self) {
		if (!instance) {
			instance = [[TIPreferencesWindowController alloc] initWithWindowNibName:@"PreferencesWindow"];
		}
	}
	
	return instance;
}


- (void)awakeFromNib {
	[[self window] center];
}

@end
