/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _WIN32_UI_BINDING_H_
#define _WIN32_UI_BINDING_H_

#include "../ui_module.h"
#include "win32_menu_item_impl.h"

namespace ti
{
	class Win32UIBinding : public UIBinding
	{

	public:
		Win32UIBinding(Host *host);
		~Win32UIBinding();

		SharedUserWindow CreateWindow(WindowConfig*, SharedUserWindow& parent);
		void ErrorDialog(std::string);

		SharedPtr<MenuItem> CreateMenu(bool trayMenu);
		void SetMenu(SharedPtr<MenuItem>);
		void SetContextMenu(SharedPtr<MenuItem>);
		void SetIcon(SharedString icon_path);
		SharedPtr<TrayItem> AddTray(SharedString icon_path,
		                            SharedBoundMethod cb);

		long GetIdleTime();

		static HMENU getContextMenuInUseHandle() { return contextMenuInUseHandle; }

	private:
		SharedPtr<Win32MenuItemImpl> contextMenuInUse;
		static HMENU contextMenuInUseHandle;
	};
}

#endif
