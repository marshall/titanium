/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"
#include "win32_menu_item_impl.h"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include "win32_tray_item.h"

namespace ti
{
	HMENU Win32UIBinding::contextMenuInUseHandle = NULL;

	Win32UIBinding::Win32UIBinding(Host *host) : UIBinding(host)
	{
	}

	Win32UIBinding::~Win32UIBinding()
	{
	}

	SharedPtr<MenuItem> Win32UIBinding::CreateMenu(bool trayMenu)
	{
		SharedPtr<MenuItem> menu = new Win32MenuItemImpl(NULL);
		return menu;
	}

	void Win32UIBinding::SetMenu(SharedPtr<MenuItem>)
	{
		// Notify all windows that the app menu has changed.
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			Win32UserWindow* wuw = dynamic_cast<Win32UserWindow*>(*i);
			if (wuw != NULL)
				wuw->AppMenuChanged();

			i++;
		}
	}

	void Win32UIBinding::SetContextMenu(SharedPtr<MenuItem> menu)
	{
		SharedPtr<Win32MenuItemImpl> menu_new = menu.cast<Win32MenuItemImpl>();

		// if same menu, just return
		if ((menu.isNull() && contextMenuInUse.isNull()) || (menu_new == contextMenuInUse))
		{
			return;
		}

		// delete old menu if available
		if(! contextMenuInUse.isNull())
		{
			contextMenuInUse->ClearRealization(contextMenuInUseHandle);
			contextMenuInUseHandle = NULL;
		}

		contextMenuInUse = menu_new;

		// create new menu if needed
		if(! contextMenuInUse.isNull())
		{
			contextMenuInUseHandle = contextMenuInUse->GetMenu();
		}
	}

	void Win32UIBinding::SetIcon(SharedString icon_path)
	{
		// Notify all windows that the app icon has changed
		// TODO this kind of notification should really be placed in UIBinding..
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			Win32UserWindow* wuw = dynamic_cast<Win32UserWindow*>(*i);
			if (wuw != NULL)
				wuw->AppIconChanged();

			i++;
		}
	}

	SharedPtr<TrayItem> Win32UIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		SharedPtr<TrayItem> trayItem = new Win32TrayItem(icon_path, cb);
		return trayItem;
	}

	long Win32UIBinding::GetIdleTime()
	{
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		return (int)idleTicks;
	}
}
