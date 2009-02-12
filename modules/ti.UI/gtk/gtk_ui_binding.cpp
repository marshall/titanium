/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>
#include <gdk/gdkx.h>

namespace ti
{
	GtkUIBinding::GtkUIBinding(Host *host) : UIBinding(host)
	{
		/* Prepare the custom curl URL handler */
		curl_register_local_handler(&Titanium_app_url_handler);

		/* Register the script evaluator */
		evaluator = new ScriptEvaluator();
		addScriptEvaluator(evaluator);
	}

	SharedPtr<MenuItem> GtkUIBinding::CreateMenu(bool trayMenu)
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


	struct OpenFilesJob
	{
		Host *host;
		SharedBoundMethod callback;
		bool multiple;
		bool files;
		bool directories;
		std::string path;
		std::string file;
		std::vector<std::string> types;
	};

	void* open_files_thread(gpointer data)
	{
		OpenFilesJob* job = reinterpret_cast<OpenFilesJob*>(data);
		SharedBoundList results = new StaticBoundList();

		/* Must do this because we are running on a separate thread
		 * from the GTK main loop */
		gdk_threads_enter();

		std::string text = "Select File";
		GtkFileChooserAction a = GTK_FILE_CHOOSER_ACTION_OPEN;
		if (job->directories)
		{
			text = "Select Directory";
			a = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
		}

		GtkWidget* chooser = gtk_file_chooser_dialog_new(
			text.c_str(),
			NULL,
			a,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), job->multiple);

		if (job->types.size() > 0)
		{
			GtkFileFilter* f = gtk_file_filter_new();
			for (size_t fi = 0; fi < job->types.size(); fi++)
			{
				std::string filter = std::string("*.") + job->types.at(fi);
				gtk_file_filter_add_pattern(f, filter.c_str());
			}
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), f);
		}

		int result = gtk_dialog_run(GTK_DIALOG(chooser));
		if (result == GTK_RESPONSE_ACCEPT && job->multiple)
		{
			GSList* files = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(chooser));
			for (size_t i = 0; i < g_slist_length(files); i++)
			{
				char* f = (char*) g_slist_nth_data(files, i);
				results->Append(Value::NewString(f));
				g_free(f);
			}
			g_slist_free(files);
		}
		else if (result == GTK_RESPONSE_ACCEPT)
		{
			char *f = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
			results->Append(Value::NewString(f));
			g_free(f);
		}
		gtk_widget_destroy(chooser);

		/* Let gtk_main continue */
		gdk_threads_leave();

		ValueList args;
		args.push_back(Value::NewList(results));

		try
		{
			job->host->InvokeMethodOnMainThread(job->callback, args);
		}
		catch (ValueException &e)
		{
			std::cerr << "openFiles callback failed because of an exception" << std::endl;
		}

		return NULL;
	}

	void GtkUIBinding::OpenFiles(
		SharedBoundMethod callback,
		bool multiple,
		bool files,
		bool directories,
		std::string& path,
		std::string& file,
		std::vector<std::string>& types)
	{
		OpenFilesJob* job = new OpenFilesJob;
		job->callback = callback;
		job->host = host;
		job->multiple = multiple;
		job->files = files;
		job->directories = directories;
		job->path = path;
		job->file = file;
		job->types = types;

		g_thread_create(&open_files_thread, job, false, NULL);
	}

	long GtkUIBinding::GetSystemIdleTime()
	{
		Display *display = gdk_x11_get_default_xdisplay();
		if (display == NULL)
			return -1;
		int screen = gdk_x11_get_default_screen();

		XScreenSaverInfo *mit_info = XScreenSaverAllocInfo();
		XScreenSaverQueryInfo(display, RootWindow(display, screen), mit_info);
		long idle_time = mit_info->idle;
		XFree(mit_info);

		return idle_time;
	}

}
