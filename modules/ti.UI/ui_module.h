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
		void SetIcon(SharedString icon_path);
		SharedString GetIcon();

		protected:
		static SharedBoundObject global;
		static SharedPtr<MenuItem> application_menu;
		static SharedString icon_path;

		DISALLOW_EVIL_CONSTRUCTORS(UIModule);

	};

}
#endif
