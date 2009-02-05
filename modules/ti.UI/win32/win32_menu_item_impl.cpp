/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "win32_menu_item_impl.h"

#include <windows.h>
#include <shellapi.h>

#include "../ui_module.h"

namespace ti
{
	int Win32MenuItemImpl::currentUID = TI_MENU_ITEM_ID_BEGIN + 1;

	std::vector<Win32MenuItemImpl *> menuItemsWithCallbacks;

	Win32MenuItemImpl::Win32MenuItemImpl(Win32MenuItemImpl* _parent) : parent(_parent)
	{
		if(this->parent == NULL)
		{
			// this is the menu bar
			hMenu = CreateMenu();
		}
	}

	Win32MenuItemImpl::~Win32MenuItemImpl()
	{
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

	/* static */
	void Win32MenuItemImpl::MakeAndAddWidget(Win32MenuItemImpl* item)
	{
		const char* label = item->GetLabel();
		const char* iconUrl = item->GetIconURL();
		SharedString iconPath = UIModule::GetResourcePath(iconUrl);
		SharedValue callbackVal = item->Get("callback");

		if(item->IsSeparator())
		{
			AppendMenu(item->parent->hMenu, MF_SEPARATOR, 1, "Separator");
		}
		else if(item->IsItem())
		{
			item->menuItemID = nextMenuUID();
			AppendMenu(item->parent->hMenu, MF_STRING, item->menuItemID, (LPCTSTR) label);

			if (callbackVal->IsMethod())
			{
				item->callback = callbackVal->ToMethod();

				menuItemsWithCallbacks.push_back(item);
			}
		}
		else if(item->IsSubMenu())
		{
			item->hMenu = CreatePopupMenu();
			AppendMenu(item->parent->hMenu, MF_STRING | MF_POPUP, (UINT_PTR) item->hMenu, (LPCTSTR) label);
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


	/*static*/
	bool Win32MenuItemImpl::invokeCallback(int menuItemUID)
	{
		for(size_t i = 0; i < menuItemsWithCallbacks.size(); i++)
		{
			Win32MenuItemImpl* item = menuItemsWithCallbacks[i];

			if(item->menuItemID == menuItemUID)
			{
				BoundMethod* cb = (BoundMethod*) item->callback;

				// TODO: Handle exceptions in some way
				try
				{
					ValueList args;
					cb->Call(args);
				}
				catch(...)
				{
					std::cout << "Menu callback failed" << std::endl;
				}

				return true;
			}
		}

		return false;
	}

	/*static*/
	LRESULT CALLBACK Win32MenuItemImpl::handleMenuClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == WM_COMMAND)
		{
			int wmId = LOWORD(wParam);

			return invokeCallback(wmId);
		}

		return FALSE;
	}

}
