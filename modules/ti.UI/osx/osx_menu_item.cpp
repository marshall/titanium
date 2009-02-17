/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"
#include "osx_menu_item.h"
#include "osx_menu_delegate.h"

namespace ti {

	OSXMenuItem::OSXMenuItem()
		 : parent(NULL)
	{
	}

	void OSXMenuItem::SetParent(OSXMenuItem* parent)
	{
		this->parent = parent;
	}

	OSXMenuItem* OSXMenuItem::GetParent()
	{
		return this->parent;
	}

	SharedValue OSXMenuItem::AddSeparator()
	{
		OSXMenuItem* item = new OSXMenuItem();
		item->MakeSeparator();
		return this->AppendItem(item);
	}

	SharedValue OSXMenuItem::AddItem(SharedValue label,
	                              SharedValue callback,
	                              SharedValue icon_url)
	{
		OSXMenuItem* item = new OSXMenuItem();
		item->MakeItem(label, callback, icon_url);
		return this->AppendItem(item);
	}

	SharedValue OSXMenuItem::AddSubMenu(SharedValue label,
	                                        SharedValue icon_url)
	{
		OSXMenuItem* item = new OSXMenuItem();
		item->MakeSubMenu(label, icon_url);
		return this->AppendItem(item);
	}

	SharedValue OSXMenuItem::AppendItem(OSXMenuItem* item)
	{
		item->SetParent(this);
		this->children.push_back(item);

		/* Realize the new item and add it to all existing instances */
		std::vector<MenuPieces*>::iterator i = this->instances.begin();
		while (i != this->instances.end())
		{
			// MenuPieces *pieces = item->Realize(false);
			// gtk_menu_shell_append(GTK_MENU_SHELL((*i)->menu), pieces->item);
			// gtk_widget_show(pieces->item);
			i++;
		}

		return MenuItem::AddToListModel(item);
	}

	NSMenu* OSXMenuItem::GetMenu()
	{
		if (this->parent == NULL) // top-level
		{
			MenuPieces* pieces = this->Realize(false);
			return pieces->menu;
		}
		else
		{
			// For now we do not support using a submenu as a menu,
			// as that makes determining parent-child relationships
			// really hard, so just return NULL and check above.
			return NULL;
		}
	}

	NSMenu* OSXMenuItem::GetMenuBar()
	{
		if (this->parent == NULL) // top level
		{
			MenuPieces* pieces = this->Realize(true);
			return pieces->menu;
		}
		else
		{
			// For now we do not support using a submenu as a menu,
			// as that makes determining parent-child relationships
			// really hard, so just return NULL and check above.
			return NULL;
		}
	}

	void OSXMenuItem::AddChildrenTo(NSMenu* menu)
	{
		// std::vector<OSXMenuItem*>::iterator c;
		// for (c = this->children.begin(); c != this->children.end(); c++)
		// {
		// 	MenuPieces* pieces = new MenuPieces();
		// 	(*c)->MakeMenuPieces(*pieces);
		// 	gtk_menu_shell_append(GTK_MENU_SHELL(menu), pieces->item);
		// 	gtk_widget_show(pieces->item);
		// 
		// 	if (this->IsSubMenu() || this->parent == NULL)
		// 	{
		// 		(*c)->AddChildrenTo(pieces->menu);
		// 	}
		// 
		// 	delete pieces;
		// }
	}

	void OSXMenuItem::ClearRealization(NSMenu *parent_menu)
	{
		std::vector<MenuPieces*>::iterator i;
		std::vector<OSXMenuItem*>::iterator c;

		// Find the instance which is contained in parent_menu or,
		// if we are the root, find the instance which uses this
		// menu to contain it's children.
		for (i = this->instances.begin(); i != this->instances.end(); i++)
		{
			if ((*i)->parent_menu == parent_menu
				|| (this->parent == NULL && (*i)->menu == parent_menu))
				break;
		}

		// Could not find an instance which uses the menu.
		if (i == this->instances.end()) return;

		// Erase all children which use
		// the sub-menu as their parent.
		for (c = this->children.begin(); c != this->children.end(); c++)
		{
			(*c)->ClearRealization((*i)->menu);
		}

		this->instances.erase(i); // Erase the instance
	}

	OSXMenuItem::MenuPieces* OSXMenuItem::Realize(bool is_menu_bar)
	{
		MenuPieces* pieces = new MenuPieces();
		// if (this->parent == NULL) // top-level
		// {
		// 	if (is_menu_bar)
		// 		pieces->menu = gtk_menu_bar_new();
		// 	else
		// 		pieces->menu = gtk_menu_new();
		// }
		// else
		// {
		// 	this->MakeMenuPieces(*pieces);
		// }
		// 
		// /* Realize this widget's children */
		// if (this->IsSubMenu() || this->parent == NULL)
		// {
		// 	std::vector<OSXMenuItem*>::iterator i = this->children.begin();
		// 	while (i != this->children.end())
		// 	{
		// 		MenuPieces* child_pieces = (*i)->Realize(false);
		// 		child_pieces->parent_menu = pieces->menu;
		// 
		// 		gtk_menu_shell_append(
		// 			GTK_MENU_SHELL(pieces->menu),
		// 			child_pieces->item);
		// 		gtk_widget_show(child_pieces->item);
		// 		i++;
		// 	}
		// }
		// 
		this->instances.push_back(pieces);
		return pieces;
	}

	void OSXMenuItem::MakeMenuPieces(MenuPieces& pieces)
	{
		// const char* label = this->GetLabel();
		const char* icon_url = this->GetIconURL();
		SharedString icon_path = UIModule::GetResourcePath(icon_url);
		SharedValue callback_val = this->RawGet("callback");

		if (this->IsSeparator())
		{
//			pieces.item = gtk_separator_menu_item_new();
		}
		else if (icon_path.isNull())
		{
//			pieces.item = gtk_menu_item_new_with_label(label);
		}
		else
		{
			// pieces.item = gtk_image_menu_item_new_with_label(label);
			// GtkWidget* image = gtk_image_new_from_file(icon_path->c_str());
			// gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pieces.item), image);
		}

		if (callback_val->IsMethod())
		{
			// The callback is stored as a property of this MenuItem
			// so we do not need to worry about the pointer being freed
			// out from under us. At some point, in threaded code we will
			// have to protect it with a mutex though, in the case that
			// the callback is fired after it has been reassigned.
			// BoundMethod* cb = callback_val->ToMethod().get();

			// g_signal_connect_swapped(
			// 	G_OBJECT (pieces.item), "activate",
			// 	G_CALLBACK(menu_callback),
			// 	(gpointer) cb);
		}

		if (this->IsSubMenu())
		{
			// pieces.menu = gtk_menu_new();
			// gtk_menu_item_set_submenu(GTK_MENU_ITEM(pieces.item), pieces.menu);
		}

	}



	/* Crazy mutations below */
	void OSXMenuItem::Enable()
	{
		std::vector<MenuPieces*>::iterator i = this->instances.begin();
		while (i != this->instances.end())
		{
			// GtkWidget *w = (*i)->item;
			// if (w != NULL)
			// 	gtk_widget_set_sensitive(w, TRUE);
			i++;
		}
	}

	void OSXMenuItem::Disable()
	{
		std::vector<MenuPieces*>::iterator i = this->instances.begin();
		while (i != this->instances.end())
		{
			// GtkWidget *w = (*i)->item;
			// if (w != NULL)
			// 	gtk_widget_set_sensitive(w, FALSE);
			i++;
		}
	}

	void OSXMenuItem::SetLabel(std::string label)
	{
		std::vector<MenuPieces*>::iterator i = this->instances.begin();
		while (i != this->instances.end())
		{
			// GtkWidget *w = (*i)->item;
			// if (w != NULL)
			// {
			// 	GtkWidget *menu_label = gtk_bin_get_child(GTK_BIN(w));
			// 	gtk_label_set_text(GTK_LABEL(menu_label), label.c_str());
			// }
			i++;
		}
	}

	void OSXMenuItem::SetIcon(std::string icon_url)
	{
		std::vector<MenuPieces*>::iterator i = this->instances.begin();
		SharedString icon_path = UIModule::GetResourcePath(icon_url.c_str());

		while (i != this->instances.end())
		{
			/*GtkWidget *w = (*i)->item;
			if (w != NULL && G_TYPE_FROM_INSTANCE(w) == GTK_TYPE_IMAGE_MENU_ITEM)
			{
				GtkWidget* image = gtk_image_new_from_file(icon_path->c_str());
				gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(w), image);
			}
			*/
			i++;
		}
	}
	
	NSMenuItem* OSXMenuItem::CreateNative()
	{
		return [[OSXMenuDelegate alloc] initWithMenu:this]; 
	}	

}

