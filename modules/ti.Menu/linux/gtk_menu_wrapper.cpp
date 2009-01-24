
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "gtk_menu_wrapper.h"
#include <cstring>

namespace ti
{

	void menu_callback(gpointer data);

	GtkMenuWrapper::GtkMenuWrapper(SharedBoundList root_item, SharedBoundObject global)
	{
		this->global = global;
		this->gtk_menu_bar = gtk_menu_bar_new ();
		this->AddChildrenToMenu(root_item, gtk_menu_bar);
	}

	GtkMenuWrapper::~GtkMenuWrapper()
	{
		this->callbacks.clear();
	}

	GtkWidget* GtkMenuWrapper::GetMenu()
	{
		return this->gtk_menu;
	}

	GtkWidget* GtkMenuWrapper::GetMenuBar()
	{
		return this->gtk_menu_bar;
	}

	void GtkMenuWrapper::AddChildrenToMenu(SharedBoundList menu,
	                                       GtkWidget *gtk_menu)
	{
		for (int i = 0; i < menu->Size(); i++)
		{
			SharedBoundList menu_item = menu->At(i)->ToList();
			if (menu_item.isNull()) // not a menu item
				continue;

			GtkWidget* gtk_item = this->ItemFromValue(menu_item);
			if (GtkMenuWrapper::ItemIsSubMenu(menu_item))
			{
				GtkWidget* gtk_submenu = gtk_menu_new();
				this->AddChildrenToMenu(menu_item, gtk_submenu);
				gtk_menu_item_set_submenu(GTK_MENU_ITEM (gtk_item), gtk_submenu);
			}

			gtk_menu_shell_append(GTK_MENU_SHELL(gtk_menu), gtk_item);
			gtk_widget_show(gtk_item);

		}
	}

	GtkWidget* GtkMenuWrapper::ItemFromValue(SharedBoundList menu_item)
	{
		const char* label = ItemGetStringProp(menu_item, "label");
		const char* icon_url = ItemGetStringProp(menu_item, "iconURL");
		SharedValue icon_val = this->GetIconPath(icon_url);
		SharedValue callback_val = menu_item->Get("callback");

		GtkWidget* gtk_item;

		if (!icon_val->IsString())
		{
			gtk_item = gtk_menu_item_new_with_label(label);
		}
		else
		{
			gtk_item = gtk_image_menu_item_new_with_label(label);
			GtkWidget* image = gtk_image_new_from_file(icon_val->ToString());
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gtk_item), image);
		}

		if (callback_val->IsMethod())
		{
			// we need to do our own memory management here because
			// we don't know when GTK will decide to clean up
			SharedBoundMethod meth = callback_val->ToMethod();
			this->callbacks.push_back(meth);

			g_signal_connect_swapped(
			    G_OBJECT (gtk_item), "activate",
			    G_CALLBACK(menu_callback), 
			    (gpointer) &meth);
		}

		return gtk_item;
	}

	void menu_callback(gpointer data)
	{
		printf("menu callback\n");
		//SharedBoundMethod* shared_meth = (SharedBoundMethod*) data;
		//BoundMethod* meth = shared_meth->get();

		//// TODO: Handle exceptions in some way
		//try
		//{
		//	ValueList args;
		//	meth->Call(args);
		//}
		//catch(...)
		//{
		//	std::cout << "Menu callback failed" << std::endl;
		//}

	}

	bool GtkMenuWrapper::ItemIsSubMenu(SharedBoundList item)
	{
		SharedBoundMethod submenu_test = item->Get("isSubMenu")->ToMethod();
		if (submenu_test.isNull())
			return false;

		ValueList args;
		SharedValue result = submenu_test->Call(args);
		return result->IsBool() && result->ToBool();
	}

	const char* GtkMenuWrapper::ItemGetStringProp(SharedBoundList item, const char* prop_name)
	{
		SharedValue prop = item->Get(prop_name);
		if (prop->IsString())
			return prop->ToString();
		else
			return "";
	}

	SharedValue GtkMenuWrapper::GetIconPath(const char *url)
	{
		if (!strcmp(url, ""))
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

}
