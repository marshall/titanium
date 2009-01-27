
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _GTK_MENU_CREATOR_H_
#define _GTK_MENU_CREATOR_H_

namespace ti
{
	class GtkMenuWrapper
	{
	public:
		GtkMenuWrapper(SharedBoundList root_item, SharedBoundObject global);
		~GtkMenuWrapper();
		GtkWidget* GetMenu();
		GtkWidget* GetMenuBar();

	private:
		GtkWidget* ItemFromValue(SharedBoundList value);
		void AddChildrenToMenu(SharedBoundList menu, GtkWidget *gtk_menu);
		SharedValue GetIconPath(const char *url);
		static const char* ItemGetStringProp(SharedBoundList item, const char* prop_name);
		static bool ItemIsSubMenu(SharedBoundList item);
		static bool ItemIsSeparator(SharedBoundList item);

		GtkWidget* gtk_menu;
		GtkWidget* gtk_menu_bar;
		std::vector<SharedBoundMethod> callbacks;;
		SharedBoundObject global;
	};
}

#endif

