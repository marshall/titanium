/*
 * win32_menu_item_impl.cpp
 *
 *  Created on: Jan 29, 2009
 *      Author: jorge
 */

#include "win32_menu_item_impl.h"

#include <windows.h>
#include <shellapi.h>

#include "../ui_module.h"

namespace ti
{
	int Win32MenuItemImpl::currentUID = TI_MENU_ITEM_ID_BEGIN + 1;

	Win32MenuItemImpl::Win32MenuItemImpl(Win32MenuItemImpl* _parent) : parent(_parent)
	{
		if(this->parent == NULL)
		{
			// this is the menu bar
			hMenu = CreateMenu();
		}
	}

	Win32MenuItemImpl::~Win32MenuItemImpl() {
		// TODO Auto-generated destructor stub
	}

	void Win32MenuItemImpl::SetParent(Win32MenuItemImpl* parent)
	{
		this->parent = parent;
	}
	Win32MenuItemImpl* Win32MenuItemImpl::GetParent()
	{
		return this->parent;
	}

	SharedValue Win32MenuItemImpl::AddSeparator()
	{
		Win32MenuItemImpl* item = new Win32MenuItemImpl(this);
		item->MakeSeparator();

		this->MakeAndAddWidget(item);

		return MenuItem::AppendItem(item);
	}
	SharedValue Win32MenuItemImpl::AddItem(SharedValue label,
						SharedValue callback,
						SharedValue icon_url)
	{
		Win32MenuItemImpl* item = new Win32MenuItemImpl(this);
		item->MakeItem(label, callback, icon_url);

		this->MakeAndAddWidget(item);

		return MenuItem::AppendItem(item);
	}
	SharedValue Win32MenuItemImpl::AddSubMenu(SharedValue label,
						   SharedValue icon_url)
	{
		Win32MenuItemImpl* item = new Win32MenuItemImpl(this);
		item->MakeSubMenu(label, icon_url);

		this->MakeAndAddWidget(item);

		return MenuItem::AppendItem(item);
	}

	void Win32MenuItemImpl::MakeAndAddWidget(Win32MenuItemImpl* item)
	{
		const char* label = this->GetLabel();
		const char* iconUrl = this->GetIconURL();
		SharedString iconPath = UIModule::GetResourcePath(iconUrl);
		SharedValue callbackVal = this->Get("callback");

		if(this->IsSeparator())
		{
			AppendMenu(this->parent->hMenu, MF_SEPARATOR, 1, "Separator");
		}
		else if(this->IsItem())
		{
			menuItemID = nextMenuUID();
			AppendMenu(this->parent->hMenu, MF_STRING, menuItemID, (LPCTSTR) label);

			if (callbackVal->IsMethod())
			{
				// we need to do our own memory management here because
				// we don't know when GTK will decide to clean up
				this->callback = callbackVal->ToMethod();

				// TODO add code to handle the callbacks
			}
		}
		else if(this->IsSubMenu())
		{
			hMenu = CreatePopupMenu();
			AppendMenu(this->parent->hMenu, MF_STRING | MF_POPUP, (UINT_PTR) hMenu, (LPCTSTR) label);
		}
		else
		{
			std::cerr << "Unknown menu item type requested" << std::endl;
			// TODO throw exception?
		}
	}

	SharedValue Win32MenuItemImpl::GetIconPath(const char *url)
	{
		return NULL;
	}

	void Win32MenuItemImpl::Enable()
	{
	}
	void Win32MenuItemImpl::Disable()
	{
	}

}
