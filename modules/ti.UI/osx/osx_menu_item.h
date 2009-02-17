/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _OSX_MENU_ITEM_H_
#define _OSX_MENU_ITEM_H_

#include "../menu_item.h"

namespace ti
{
	enum OSXMenuItemType
	{
		Separator,
		Tray,
		Item
	};
	
	//FIXME Type
	
	class OSXMenuItem : public MenuItem
	{

	public:
		OSXMenuItem(OSXMenuItemType type = Item);
		virtual ~OSXMenuItem();

		void SetParent(OSXMenuItem* parent);
		OSXMenuItem* GetParent();

		SharedValue AddSeparator();
		SharedValue AddItem(SharedValue label,
		                    SharedValue callback,
		                    SharedValue icon_url);
		SharedValue AddSubMenu(SharedValue label,
		                       SharedValue icon_url);

		SharedValue AppendItem(OSXMenuItem* item);

		void Enable();
		void Disable();
		void SetLabel(std::string label);
		void SetIcon(std::string icon_path);
		
		void Invoke();
		
		NSMenuItem* GetNative() { return native; }
		
	private:
		OSXMenuItem *parent; // NULL parent means this is top-level menu.
		std::vector<OSXMenuItem*> children;
		NSMenuItem *native;
		
	};

}

#endif
