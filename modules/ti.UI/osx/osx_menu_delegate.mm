/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "osx_menu_delegate.h"
#import "osx_menu_item.h"

@implementation OSXMenuDelegate
-(id)initWithMenu:(ti::OSXMenuItem*)item
{
	NSString *title = [NSString stringWithCString:item->GetLabel()];
	self = [super initWithTitle:title action:@selector(invoke:) keyEquivalent:@""];
	if (self)
	{
		[self setTarget:self];
		delegate = item;
	}
	return self;
}
-(void)invoke:(id)data
{
	delegate->Invoke();
}
@end


