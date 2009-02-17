/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _OSX_MENU_ITEM_IMPL_H_
#define _OSX_MENU_ITEM_IMPL_H_

#include <Cocoa/Cocoa.h>
#include "../menu_item.h"

namespace ti
{

	class OSXMenuItem : public MenuItem
	{

	public:
		OSXMenuItem();

		void SetParent(OSXMenuItem* parent);
		OSXMenuItem* GetParent();

		SharedValue AddSeparator();
		SharedValue AddItem(SharedValue label,
		                    SharedValue callback,
		                    SharedValue icon_url);
		SharedValue AddSubMenu(SharedValue label,
		                       SharedValue icon_url);

		SharedValue AppendItem(OSXMenuItem* item);

		NSMenu* GetMenu();
		NSMenu* GetMenuBar();

		/* This is used for transient menus, and the widgets
		 * are not recorded in the instances list */
		void AddChildrenTo(NSMenu* menu);

		void ClearRealization(NSMenu *parent_menu);

		void Enable();
		void Disable();
		void SetLabel(std::string label);
		void SetIcon(std::string icon_path);

		struct MenuPieces
		{
			MenuPieces()
				 : item(NULL),
				   menu(NULL),
				   callback(NULL),
				   parent_menu(NULL) { }
			NSMenuItem* item; // This item's widget
			NSMenu* menu; // This item's submenu
			SharedBoundMethod callback; // This item's callback
			NSMenu* parent_menu; // This item's parent's widget
		};
		
		NSMenuItem* CreateNative();
		void Invoke() {}

	private:
		OSXMenuItem *parent; // NULL parent means this is top-level menu.
		std::vector<OSXMenuItem*> children;
		std::vector<MenuPieces*> instances;
		//std::vector<GtkWidget*> widget_menus;

		MenuPieces* Realize(bool is_menu_bar);
		void MakeMenuPieces(MenuPieces& pieces);


	};

}

#endif
