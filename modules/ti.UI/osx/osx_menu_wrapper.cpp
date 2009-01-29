/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "osx_menu_wrapper.h"

namespace ti
{
	OSXMenuWrapper::OSXMenuWrapper(SharedBoundList root_item, SharedBoundObject global) : MenuWrapper(root_item,global)
	{
		this->menu = [[NSMenu alloc] init];
		this->AddChildrenToMenu(root_item,this->menu);
	}
	OSXMenuWrapper::~OSXMenuWrapper()
	{
		KR_DUMP_LOCATION
		for (size_t c=0;c<actions.size();c++)
		{
			MenuAction *action = actions.at(c);
			[action release];
		}
		actions.clear();
		for (int c=0;c<[menu numberOfItems];c++)
		{
			[menu removeItemAtIndex:c];
		}
		[menu release];
	}
	NSMenu* OSXMenuWrapper::getNSMenu()
	{
		return menu;
	}
	NSMenuItem* OSXMenuWrapper::ItemFromValue(SharedBoundList menu_item)
	{
		const char* label = ItemGetStringProp(menu_item, "label");
		const char* icon_url = ItemGetStringProp(menu_item, "iconURL");
		SharedValue icon_val = this->GetIconPath(icon_url);
		SharedValue callback_val = menu_item->Get("callback");
	
		NSMenuItem *item;
		
		if (ItemIsSeparator(menu_item))
		{
			item = [NSMenuItem separatorItem];
			[item retain]; // we release, so we need retain since shared
		}
		else
		{
			NSString* empty = [NSString stringWithCString:""];
			item = [[NSMenuItem alloc] initWithTitle:[NSString stringWithCString:label] action:NULL keyEquivalent:empty];
			if (icon_val->IsString())
			{
				NSURL *url = [NSURL URLWithString:[NSString stringWithCString:icon_url]];
				NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
				[item setImage:image];
				[image release];
				
				std::cout << "label = " << label <<", icon = " << icon_url << std::endl;
			}
		}

		if (callback_val->IsMethod())
		{
			SharedBoundMethod meth = callback_val->ToMethod();
			MenuAction* action = [[MenuAction alloc] initWithMethod:meth];
			[item setTarget:action];
			[item setAction:@selector(fire)];
			actions.push_back(action);
		}
		
		return item;
	}
	void OSXMenuWrapper::AddChildrenToMenu(SharedBoundList menu, NSMenu *osxmenu)
	{
		for (int i = 0; i < menu->Size(); i++)
		{
			SharedBoundList menu_item = menu->At(i)->ToList();
			if (menu_item.isNull()) // not a menu item
			{
				continue;
			}
			
			NSMenuItem *item = ItemFromValue(menu_item);
			
			if (ItemIsSubMenu(menu_item))
			{
				NSString *title = [item title];
				NSMenu *submenu = [[[NSMenu alloc] initWithTitle:title] autorelease];
				AddChildrenToMenu(menu_item,submenu);
				[item setSubmenu:submenu];
			}
			
			[osxmenu addItem:item];
			[item release];
		}
	}
}