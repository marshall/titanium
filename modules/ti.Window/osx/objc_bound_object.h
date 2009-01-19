/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "preinclude.h"
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "WebViewPrivate.h"
#import "WebInspector.h"
#import "WebScriptDebugDelegate.h"
#import "WebScriptObject.h"

@interface ObjcBoundObject : NSObject {
	SharedPtr<BoundObject>* object;
	JSContextRef context;
	NSString *key; //TODO: debug only
}
-(id)initWithObject:(SharedPtr<BoundObject>)object key:(NSString*)key context:(JSContextRef)ctx;
-(BOOL)isWrappedBoundObject;
-(SharedPtr<BoundObject>)boundObject;
+(id)ValueToID:(Value*)value key:(NSString*)key context:(JSContextRef)ctx;
+(SharedValue)IDToValue:(id)value context:(JSContextRef)ctx;
@end
