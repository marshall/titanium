
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"

namespace ti
{

	void tray_click_callback(GtkStatusIcon*, gpointer);
	void tray_menu_callback(GtkStatusIcon*, guint, guint, gpointer);

	GtkTrayItem::GtkTrayItem(SharedString icon_path, SharedBoundMethod cb)
		 : TrayItem()
	{
		this->widget = gtk_status_icon_new();
		this->menu = NULL;
		this->menu_widget = NULL;

		if (!cb.isNull())
		{
			this->callback = cb;
			g_signal_connect(
				G_OBJECT(this->widget),
				"activate", 
				G_CALLBACK(tray_click_callback),
				cb.get());
		}

		g_signal_connect(
			G_OBJECT(this->widget), 
			"popup-menu",
			G_CALLBACK(tray_menu_callback),
			this);

		this->SetIcon(icon_path);

		gtk_status_icon_set_visible(this->widget, TRUE);
	}

	GtkTrayItem::~GtkTrayItem()
	{
	}

	void GtkTrayItem::SetIcon(SharedString icon_path)
	{
		if (icon_path.isNull())
		{
			gtk_status_icon_set_from_file(this->widget, NULL);
		}
		else
		{
			gtk_status_icon_set_from_file(
				this->widget,
				icon_path->c_str());
		}
	}

	void GtkTrayItem::SetMenu(SharedBoundList menu)
	{
		SharedPtr<GtkMenuItemImpl> gtk_menu = menu.cast<GtkMenuItemImpl>();
		if (gtk_menu == this->menu)
			return;

		if (!this->menu.isNull() && this->menu_widget != NULL)
		{
			this->menu->DeleteWidget(this->menu_widget);
		}

		this->menu = gtk_menu;
		this->menu_widget = gtk_menu->GetMenu();
	}

	void GtkTrayItem::SetHint(SharedString hint)
	{
		gtk_status_icon_set_tooltip(this->widget, hint->c_str());
	}

	void GtkTrayItem::Remove()
	{
		if (!this->menu.isNull() && this->menu_widget != NULL)
		{
			this->menu->DeleteWidget(this->menu_widget);
		}

		gtk_widget_destroy(GTK_WIDGET(this->widget));
	}

	GtkStatusIcon* GtkTrayItem::GetWidget()
	{
		return this->widget;
	}

	GtkWidget* GtkTrayItem::GetMenuWidget()
	{
		return this->menu_widget;
	}

	SharedPtr<GtkMenuItemImpl> GtkTrayItem::GetMenu()
	{
		return this->menu;
	}

	void tray_click_callback(GtkStatusIcon *status_icon, gpointer data)
	{
		BoundMethod* cb = (BoundMethod*) data;
		// TODO: Handle exceptions in some way
		try
		{
			ValueList args;
			cb->Call(args);
		}
		catch(...)
		{
			std::cout << "Tray icon callback failed" << std::endl;
		}
	}

	void tray_menu_callback(
		GtkStatusIcon *status_icon,
		guint button, 
		guint activate_time,
		gpointer data)
	{
		GtkTrayItem* item = (GtkTrayItem*) data;

		GtkStatusIcon* tray_widget = item->GetWidget();
		GtkWidget* menu_widget = item->GetMenuWidget();
		SharedPtr<GtkMenuItemImpl> menu = item->GetMenu();

		if (menu.isNull() || menu_widget == NULL)
			return;

		gtk_menu_popup(
			GTK_MENU(menu_widget),
			NULL,
			NULL,
			gtk_status_icon_position_menu,
			tray_widget,
			button,
			activate_time);
	}


}
