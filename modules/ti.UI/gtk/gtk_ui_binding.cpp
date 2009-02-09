/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"

namespace ti
{
	GtkUIBinding::GtkUIBinding() : UIBinding()
	{
		/* Prepare the custom curl URL handler */
		curl_register_local_handler(&Titanium_app_url_handler);

		/* Register the script evaluator */
		evaluator = new ScriptEvaluator();
		addScriptEvaluator(evaluator);
	}

	SharedPtr<MenuItem> GtkUIBinding::CreateMenu()
	{
		SharedPtr<MenuItem> menu = new GtkMenuItemImpl();
		return menu;
	}

	void GtkUIBinding::SetMenu(SharedPtr<MenuItem> new_menu)
	{
		// Notify all windows that the app menu has changed.
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			GtkUserWindow* guw = dynamic_cast<GtkUserWindow*>(*i);
			if (guw != NULL)
				guw->AppMenuChanged();

			i++;
		}
	}

	void GtkUIBinding::SetContextMenu(SharedPtr<MenuItem> new_menu)
	{
	}

	void GtkUIBinding::SetIcon(SharedString icon_path)
	{

		// Notify all windows that the app icon has changed.
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			GtkUserWindow* guw = dynamic_cast<GtkUserWindow*>(*i);
			if (guw != NULL)
				guw->AppIconChanged();

			i++;
		}
	}

	SharedPtr<TrayItem> GtkUIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		SharedPtr<TrayItem> item = new GtkTrayItem(icon_path, cb);
		return item;
	}

	std::vector<std::string> GtkUIBinding::OpenFiles(
		bool multiple,
		bool files,
		bool directories,
		std::vector<std::string> types)
	{

		std::string text = "Choose File";
		GtkFileChooserAction a = GTK_FILE_CHOOSER_ACTION_OPEN;
		if (directories)
		{
			text = "Choose Directory";
			a = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
		}

		GtkWidget* chooser = gtk_file_chooser_dialog_new(
			text.c_str(),
			NULL,
			a,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
		

		//gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), multiple);

		//GtkFileFilter* filter = gtk_file_filter_new();
		//for (size_t i = 0; i < types.size(); i++)
		//{
		//	std::string pat = std::string("*.") + types.at(i);
		//	gtk_file_filter_add_pattern(filter, pat.c_str());
		//}
		//gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(chooser), filter);

		std::vector<std::string> to_ret;
		if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
		{
			if (multiple)
			{
				GSList* files = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(chooser));
				for (size_t i = 0; i < g_slist_length(files); i++)
				{
					char* f = (char*) g_slist_nth_data(files, i);
					to_ret.push_back(std::string(f));
					g_free(f);
				}
				g_slist_free(files);
			}
			else
			{
				char *f = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
				to_ret.push_back(std::string(f));
				g_free(f);
			}
		}
		gtk_widget_destroy(chooser);

		return to_ret;
	}
}

// GtkDialogFlags f = (GtkDialogFlags) GTK_DIALOG_MODAL;
// GtkWidget *dialog = gtk_dialog_new_with_buttons ("My dialog",
//                                                  NULL,
//                                                  f,
//                                                  GTK_STOCK_OK,
//                                                  GTK_RESPONSE_ACCEPT,
//                                                  GTK_STOCK_CANCEL,
//                                                  GTK_RESPONSE_REJECT,
//                                                  NULL);
//  gint result = gtk_dialog_run (GTK_DIALOG (dialog));
//  switch (result)
//    {
//      case GTK_RESPONSE_ACCEPT:
//         printf("yes\n");
//         break;
//      default:
//         printf("no\n");
//         break;
//    }
//  gtk_widget_destroy (dialog);
