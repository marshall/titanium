//
//  TIJavaScriptPromptWindowController.h
//  Titanium
//
//  Created by Todd Ditchendorf on 11/1/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TIJavaScriptPromptWindowController : NSWindowController {
	IBOutlet NSTextView *textView;
	NSString *labelText;
	NSString *userText;
}
- (IBAction)close:(id)sender;

- (NSString *)labelText;
- (void)setLabelText:(NSString *)s;
- (NSString *)userText;
- (void)setUserText:(NSString *)s;
@end
