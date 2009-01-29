
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _GTK_MENU_ITEM_IMPL_H_
#define _GTK_MENU_ITEM_IMPL_H_

namespace ti
{

	class GtkMenuItemImpl : public MenuItem
	{

	public:
		GtkMenuItemImpl(bool top_level=false);

		GtkWidget* GetWidget();
		GtkWidget* GetMenu();
		GtkWidget* GetMenuBar();
		void SetParent(GtkMenuItemImpl* parent);
		GtkMenuItemImpl* GetParent();

		SharedValue AddSeparator();
		SharedValue AddItem(SharedValue label,
		                    SharedValue callback,
		                    SharedValue icon_url);
		SharedValue AddSubMenu(SharedValue label,
		                       SharedValue icon_url);

		void MakeWidget();
		SharedValue GetIconPath(const char *url);
		SharedValue AppendItem(GtkMenuItemImpl* item);

		void Enable();
		void Disable();

	private:
		bool top_level;

		GtkWidget *gtk_menu_bar;
		GtkWidget *gtk_menu;
		GtkWidget *widget;
		GtkMenuItemImpl *parent;
		SharedBoundMethod callback;

	};

}

#endif
