/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#import "osx_menu_delegate.h"
#import "osx_menu_item.h"
#import "../ui_module.h"

@implementation OSXMenuDelegate
-(id)initWithMenu:(ti::OSXMenuItem*)item
{
	NSString *title = [NSString stringWithCString:item->GetLabel()];
	self = [super initWithTitle:title action:@selector(invoke:) keyEquivalent:@""];
	if (self!=nil)
	{
		delegate = item;
		[self setTarget:self];
		const char *icon_url = item->GetIconURL();
		if (icon_url)
		{
			if (ti::UIModule::IsResourceLocalFile(icon_url))
			{
				SharedString file = ti::UIModule::GetResourcePath(icon_url);
				NSString *path = [NSString stringWithCString:((*file).c_str())];
				NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];
				[self setImage:image];
				[image release];
			}
			else
			{
				NSURL *url = [NSURL URLWithString:[NSString stringWithCString:icon_url]];
				NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
				[self setImage:image];
				[image release];
			}
		}
		int count = item->GetChildCount();
		if (count > 0)
		{
			NSMenu *submenu = [[NSMenu alloc] initWithTitle:title];
			for (int c=0;c<count;c++)
			{
				OSXMenuItem *i = item->GetChild(c);
				NSMenuItem *mi = i->CreateNative();
				[submenu addItem:mi];
			}
			[self setSubmenu:submenu];
			// don't release this or items 
		}
	}
	return self;
}
-(void)invoke:(id)sender
{
	delegate->Invoke();
}
@end


