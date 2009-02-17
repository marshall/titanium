/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"
#include "osx_menu_item.h"
#include "osx_menu_delegate.h"

namespace ti {

	OSXMenuItem::OSXMenuItem(OSXMenuItemType type)
		 : parent(NULL)
	{
		if (type == Separator)
		{
			this->native = [NSMenuItem separatorItem];
			[native retain];
		}
		else if (type == Tray)
		{
			//TODO:
		}
		else if (type == Item)
		{
			this->native = [[OSXMenuDelegate alloc] initWithMenu:this];
		}
	}
	
	OSXMenuItem::~OSXMenuItem()
	{
		[native release];
	}
	
	void OSXMenuItem::Invoke()
	{
		//invoke callback
		SharedValue callback_val = this->RawGet("callback");
		if (callback_val->IsMethod())
		{
			SharedBoundMethod method = callback_val->ToMethod();
			try
			{
				ValueList args;
				method->Call(args);
			}
			catch(...)
			{
				std::cerr << "Menu callback failed" << std::endl;
			}
		}
	}

	void OSXMenuItem::SetParent(OSXMenuItem* parent)
	{
		//TODO: review this ...
		this->parent = parent;
	}

	OSXMenuItem* OSXMenuItem::GetParent()
	{
		return this->parent;
	}

	SharedValue OSXMenuItem::AddSeparator()
	{
		OSXMenuItem* item = new OSXMenuItem(Separator);
		item->MakeSeparator();
		return this->AppendItem(item);
	}
	
	SharedValue OSXMenuItem::AddItem(SharedValue label,
	                              SharedValue callback,
	                              SharedValue icon_url)
	{
		OSXMenuItem* item = new OSXMenuItem(Item);
		item->MakeItem(label, callback, icon_url);
		return this->AppendItem(item);
	}

	SharedValue OSXMenuItem::AddSubMenu(SharedValue label,
	                                        SharedValue icon_url)
	{
		OSXMenuItem* item = new OSXMenuItem(Item);
		item->MakeSubMenu(label, icon_url);
		return this->AppendItem(item);
	}

	SharedValue OSXMenuItem::AppendItem(OSXMenuItem* item)
	{
		item->SetParent(this);
		this->children.push_back(item);


		// /* Realize the new item and add it to all existing instances */
		// std::vector<MenuPieces*>::iterator i = this->instances.begin();
		// while (i != this->instances.end())
		// {
		// 	MenuPieces *pieces = item->Realize(false);
		// 	gtk_menu_shell_append(GTK_MENU_SHELL((*i)->menu), pieces->item);
		// 	gtk_widget_show(pieces->item);
		// 	i++;
		// }

		return MenuItem::AddToListModel(item);
	}


	/* Crazy mutations below */
	void OSXMenuItem::Enable()
	{
		[native setEnabled:YES];
	}

	void OSXMenuItem::Disable()
	{
		[native setEnabled:NO];
	}

	void OSXMenuItem::SetLabel(std::string label)
	{
		NSString *title = [NSString stringWithCString:label.c_str()];
		[native setTitle:title];
	}

	void OSXMenuItem::SetIcon(std::string icon_url)
	{
		if (UIModule::IsResourceLocalFile(icon_url))
		{
			SharedString file = UIModule::GetResourcePath(icon_url.c_str());
			NSString *path = [NSString stringWithCString:((*file).c_str())];
			NSImage *image = [[NSImage alloc] initWithContentsOfFile:path];
			[native setImage:image];
			[image release];
		}
		else
		{
			NSURL *url = [NSURL URLWithString:[NSString stringWithCString:icon_url.c_str()]];
			NSImage *image = [[NSImage alloc] initWithContentsOfURL:url];
			[native setImage:image];
			[image release];
		}
	}
}

