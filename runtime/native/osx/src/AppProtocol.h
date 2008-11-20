//
//  AppProtocol.h
//  Titanium
//
//  Created by Marshall on 11/18/08.
//  Copyright 2008 Appcelerator, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSURLProtocol.h>


@interface AppProtocol : NSURLProtocol {

}

+ (NSString *)mimeTypeFromExtension:(NSString *)ext;
+ (NSString*)specialProtocolScheme;
+ (void) registerSpecialProtocol;
@end
