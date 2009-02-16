/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _UI_MODULE_H_
#define _UI_MODULE_H_

#include <kroll/kroll.h>

namespace ti {
	class UIBinding;
	class WindowBinding;
	class MenuItem;
	class TrayItem;
}

#ifdef OS_WIN32
// A little disorganization; header include order is very sensitive in win32,
// and the build breaks if this below the other OS_ defines
#include "win32/win32_user_window.h"
#endif

#include <iostream>
#include "window_config.h"
#include "user_window.h"
#include "window.h"
#include "menu_item.h"
#include "tray_item.h"
#include "ui_binding.h"

#ifdef OS_LINUX
#include "gtk/ui_module_gtk.h"
#include "gtk/script_evaluator.h"

#elif defined(OS_OSX)
#include "osx/ui_module_osx.h"

#elif defined(OS_WIN32)
#include "win32/win32_ui_binding.h"
#endif


namespace ti {

	class UIModule : public kroll::Module
	{
		KROLL_MODULE_CLASS(UIModule)

		public:

		/*
		 * Function: GetResourcePath
		 * Get the real path to a resource given an app:// URL. This
		 * will access the appropriate function in the global object.
		 * Arguments:
		 *  url - the app:// URL to resolve
		 * Returns: The path to the resource on this system or a
		 *          NULL SharedString on failure.
		 */
		static SharedString GetResourcePath(const char *URL);

		static void SetMenu(SharedPtr<MenuItem> menu);
		static SharedPtr<MenuItem> GetMenu();
		static void SetContextMenu(SharedPtr<MenuItem> menu);
		static SharedPtr<MenuItem> GetContextMenu();
		static void SetIcon(SharedString icon_path);
		static SharedString GetIcon();
		static void AddTrayItem(SharedPtr<TrayItem>);
		static void ClearTrayItems();
		static void UnregisterTrayItem(TrayItem* item);

		protected:
		static SharedBoundObject global;
		static SharedPtr<MenuItem> app_menu;
		static SharedPtr<MenuItem> app_context_menu;
		static SharedString icon_path;
		static std::vector<SharedPtr<TrayItem> > tray_items;

		DISALLOW_EVIL_CONSTRUCTORS(UIModule);

	};

}
#endif
