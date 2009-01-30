/*
 * win32_menu_item_impl.h
 *
 *  Created on: Jan 29, 2009
 *      Author: jorge
 */

#ifndef _TI_WIN32_MENU_ITEM_IMPL_H_
#define _TI_WIN32_MENU_ITEM_IMPL_H_

#include "../menu_item.h"
#include <kroll/base.h>

#define TI_MENU_ITEM_ID_BEGIN 7500

namespace ti
{

	class Win32MenuItemImpl : public MenuItem
	{
	public:
		Win32MenuItemImpl(Win32MenuItemImpl* _parent = NULL);
		virtual ~Win32MenuItemImpl();

		void SetParent(Win32MenuItemImpl* parent);
		Win32MenuItemImpl* GetParent();

		SharedValue AddSeparator();
		SharedValue AddItem(SharedValue label,
		                    SharedValue callback,
		                    SharedValue icon_url);
		SharedValue AddSubMenu(SharedValue label,
		                       SharedValue icon_url);

		void MakeAndAddWidget(Win32MenuItemImpl* item);

		SharedValue GetIconPath(const char *url);

		void Enable();
		void Disable();

		static int nextMenuUID() {
			return currentUID++;
		}

		HMENU GetMenuHandle() { return this->hMenu; }

	private:
		Win32MenuItemImpl *parent;
		HMENU hMenu;
		int menuItemID;

		SharedBoundMethod callback;

		static int currentUID;
	};

}

#endif /* _TI_WIN32_MENU_ITEM_IMPL_H_ */
