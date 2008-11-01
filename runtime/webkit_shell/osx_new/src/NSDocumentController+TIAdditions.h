//
//  NSDocumentController+TIAdditions.h
//  Titanium
//
//  Created by Todd Ditchendorf on 10/31/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TIBrowserDocument;

@interface NSDocumentController (TIAdditions)
- (TIBrowserDocument *)newDocumentWithRequest:(NSURLRequest *)request makeKey:(BOOL)makeKey;
@end
