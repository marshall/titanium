/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"

namespace ti {

	void menu_callback(gpointer data);

	GtkMenuItemImpl::GtkMenuItemImpl(SharedBoundObject global, bool top)
		 : top_level(top), global(global)
	{
		if (top_level)
		{
			this->gtk_menu_bar = gtk_menu_bar_new();
			//this->gtk_menu = gtk_menu_new();
			this->MakeSubMenu(Value::Undefined, Value::Undefined);
		}
		else
		{
			this->gtk_menu_bar = NULL;
			this->gtk_menu = NULL;
		}
		this->widget = NULL;
	}

	GtkWidget* GtkMenuItemImpl::GetWidget()
	{
		return this->widget;
	}
	GtkWidget* GtkMenuItemImpl::GetMenu()
	{
		return this->gtk_menu;
	}
	GtkWidget* GtkMenuItemImpl::GetMenuBar()
	{
		return this->gtk_menu_bar;
	}
	void GtkMenuItemImpl::SetParent(GtkMenuItemImpl* parent)
	{
		this->parent = parent;
	}
	GtkMenuItemImpl* GtkMenuItemImpl::GetParent()
	{
		return this->parent;
	}

	SharedValue GtkMenuItemImpl::AddSeparator()
	{
		GtkMenuItemImpl* item = new GtkMenuItemImpl(this->global, false);
		item->MakeSeparator();
		item->MakeWidget();

		return this->AppendItem(item);
	}

	SharedValue GtkMenuItemImpl::AddItem(SharedValue label,
	                              SharedValue callback,
	                              SharedValue icon_url)
	{
		GtkMenuItemImpl* item = new GtkMenuItemImpl(this->global, false);
		item->MakeItem(label, callback, icon_url);
		item->MakeWidget();

		return this->AppendItem(item);
	}

	SharedValue GtkMenuItemImpl::AddSubMenu(SharedValue label,
	                                 SharedValue icon_url)
	{
		GtkMenuItemImpl* item = new GtkMenuItemImpl(this->global, false);
		item->MakeSubMenu(label, icon_url);
		item->MakeWidget();

		return this->AppendItem(item);
	}

	SharedValue GtkMenuItemImpl::AppendItem(GtkMenuItemImpl* item)
	{
		GtkWidget* w = item->GetWidget();
		item->SetParent(this);

		/* If this is a top-level menu also maintain a menu bar, so
		 * that the interface can also assign this menu to windows */
		if (top_level)
		{
			gtk_menu_shell_append(GTK_MENU_SHELL(gtk_menu_bar), w);
		}
		else
		{
			gtk_menu_shell_append(GTK_MENU_SHELL(gtk_menu), w);
		}

		gtk_widget_show(w);
		return MenuItem::AppendItem(item);
	}

	void GtkMenuItemImpl::MakeWidget()
	{
		const char* label = this->GetLabel();
		const char* icon_url = this->GetIconURL();
		SharedValue icon_val = this->GetIconPath(icon_url);
		SharedValue callback_val = this->Get("callback");

		GtkWidget* widget;
		if (this->IsSeparator())
		{
			widget = gtk_separator_menu_item_new();
		}
		else if (!icon_val->IsString())
		{
			widget = gtk_menu_item_new_with_label(label);
		}
		else
		{
			widget = gtk_image_menu_item_new_with_label(label);
			GtkWidget* image = gtk_image_new_from_file(icon_val->ToString());
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(widget), image);
		}

		if (callback_val->IsMethod())
		{
			// we need to do our own memory management here because
			// we don't know when GTK will decide to clean up
			this->callback = callback_val->ToMethod();

			g_signal_connect_swapped(
			    G_OBJECT (widget), "activate",
			    G_CALLBACK(menu_callback), 
			    (gpointer) new SharedBoundMethod(this->callback));
		}

		if (this->IsSubMenu())
		{
			this->gtk_menu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget), this->gtk_menu);
		}

		this->widget = widget;
	}

	SharedValue GtkMenuItemImpl::GetIconPath(const char *url)
	{
		if (url == NULL || !strcmp(url, ""))
			return Value::Undefined;

		SharedValue app_obj = this->global->Get("App");
		if (!app_obj->IsObject())
			return Value::Undefined;

		SharedValue meth_val = app_obj->ToObject()->Get("appURLToPath");
		if (!meth_val->IsMethod())
			return Value::Undefined;

		SharedBoundMethod meth = meth_val->ToMethod();
		ValueList args;
		args.push_back(Value::NewString(url));
		return meth->Call(args);
	}

	void GtkMenuItemImpl::Enable()
	{
		if (this->widget != NULL)
			gtk_widget_set_sensitive(this->widget, TRUE);
	}

	void GtkMenuItemImpl::Disable()
	{
		if (this->widget != NULL)
			gtk_widget_set_sensitive(this->widget, FALSE);
	}

	void menu_callback(gpointer data)
	{
		SharedBoundMethod* shared_meth = (SharedBoundMethod*) data;

		// TODO: Handle exceptions in some way
		try
		{
			ValueList args;
			shared_meth->get()->Call(args);
		}
		catch(...)
		{
			std::cout << "Menu callback failed" << std::endl;
		}

	}

}

