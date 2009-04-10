/** * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "installer.h"
#include "titanium_icon.h"

#include <iostream>
#include <sstream>

void *download_thread_f(gpointer data);
void *install_thread_f(gpointer data);
static gboolean watcher(gpointer data);
static void cancel_cb(GtkWidget *widget, gpointer data);
static void destroy_cb(GtkWidget *widget, gpointer data);

#define WINDOW_WIDTH 350 
#define WINDOW_HEIGHT 130

#define CANCEL 0
#define HOMEDIR_INSTALL 1
#define SYSTEM_INSTALL 2

Installer::Installer(std::string app_name) :
		app_name(app_name),
		current_job(NULL),
		cancel(false),
		error(""),
		install_type(HOMEDIR_INSTALL)
{

	this->download_finished = false;
	this->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(this->window), 10);
	gtk_window_set_default_size(GTK_WINDOW(this->window), WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_resize(GTK_WINDOW(this->window), WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_set_gravity(GTK_WINDOW(this->window), GDK_GRAVITY_CENTER);

	std::string title = this->app_name + " - Installer";
	gtk_window_set_title(GTK_WINDOW(this->window), title.c_str());

	g_signal_connect (
		G_OBJECT(this->window),
		"destroy",
		G_CALLBACK(destroy_cb),
		(gpointer) this);

	gtk_window_move(
		GTK_WINDOW(this->window),
		gdk_screen_width()/2 - WINDOW_WIDTH/2,
		gdk_screen_height()/2 - WINDOW_HEIGHT/2);

	GdkColormap* colormap = gtk_widget_get_colormap(this->window);
	GdkBitmap *mask = NULL;
	GdkPixmap* icon = gdk_pixmap_colormap_create_from_xpm_d(
		NULL,
		colormap,
		&mask,
		NULL,
		(gchar**) titanium_xpm);
	GtkWidget* image = gtk_image_new_from_pixmap(icon, mask);
	this->label = gtk_label_new("Downloading packages..");

	GtkWidget* hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), this->label, FALSE, FALSE, 0);

	this->bar = gtk_progress_bar_new();

	GtkWidget* hbox2 = gtk_hbox_new(FALSE, 0);
	GtkWidget* cancel_but = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_start(GTK_BOX(hbox2), cancel_but, TRUE, FALSE, 0);

	g_signal_connect (
		G_OBJECT(cancel_but),
		"clicked",
		G_CALLBACK(cancel_cb),
		(gpointer) this);

	GtkWidget* vbox = gtk_vbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), this->bar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 10);

	gtk_container_add(GTK_CONTAINER(this->window), vbox);

}

void Installer::AddJob(std::string url)
{
	this->jobs.push_back(new Job(url, this));
}

void Installer::ClearJobs()
{
	std::vector<Job*>::iterator i = jobs.begin();
	while (i != jobs.end())
	{
		Job* j = *i;
		i = jobs.erase(i);
		delete j;
	}
}

void Installer::Run()
{

	this->SetRunning(true);

	this->StartDownloading();
	int timer = g_timeout_add(100, watcher, this);
	gtk_widget_show_all(this->window);

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();

	g_source_remove(timer);


}

void Installer::StartDownloading()
{
	g_thread_init(NULL);
	this->download_thread =
		g_thread_create(&download_thread_f, this, TRUE, NULL);

	if (this->download_thread == NULL)
		g_warning("Can't create download thread!\n");
}

void Installer::StartInstalling()
{
	if (this->download_thread != NULL)
	{
		g_thread_join(this->download_thread);

		// This has to happen when no other threads are
		// running, since CURL's global shutdown is not
		// thread safe and we need access to downloaded
		// files now.
		Job::ShutdownDownloader();

		if(!g_thread_create(&install_thread_f, this, FALSE, NULL))
			g_warning("Can't create install thread!\n");

		this->download_thread = NULL;
	}
}


void Installer::UpdateProgress()
{
	Job *j = this->CurrentJob();

	if (this->window == NULL)
		return;

	if (this->IsRunning() && j != NULL)
	{
		double progress = j->GetProgress();
		gtk_progress_bar_set_fraction(
			GTK_PROGRESS_BAR(this->bar),
			progress);

		std::ostringstream text;
		if (this->DownloadFinished())
			text << "Installing ";
		else
			text << "Downloading ";
		text << "package " << j->Index() << " of " << Job::total;

		gtk_label_set_text(GTK_LABEL(this->label), text.str().c_str());

	}
	else if (!this->IsRunning())
	{
		gtk_progress_bar_set_fraction(
			GTK_PROGRESS_BAR(this->bar),
			1.0);
	}
}

static gboolean watcher(gpointer data)
{
	Installer *i = (Installer*) data;

	if (i->DownloadFinished() && i->IsRunning())
	{
		i->StartInstalling();
	}

	if (!i->IsRunning())
	{
		i->ShowError();
		gtk_main_quit();
	}
	else if (i->IsCancelled())
	{
		i->SetError(std::string("Package installation cancelled"));
		i->ShowError();
		gtk_main_quit();
	}
	else
	{
		i->UpdateProgress();
	}

	return i->IsRunning();
}

void *download_thread_f(gpointer data)
{
	Installer* inst = (Installer*) data;
	std::vector<Job*>& jobs = inst->GetJobs();
	try
	{
		for (size_t i = 0; i < jobs.size(); i++)
		{
			Job *j = jobs.at(i);
			inst->SetCurrentJob(j);
			j->Fetch();
		}
	}
	catch (std::exception& e)
	{
		std::string message = e.what();
		inst->SetError(message);
		inst->SetRunning(false);
	}
	catch (std::string& e)
	{
		inst->SetError(e);
		inst->SetRunning(false);
	}
	catch (...)
	{
		std::string message = "Unknown error";
		inst->SetError(message);
		inst->SetRunning(false);
	}

	inst->DownloadDone();
	return NULL;
}

void *install_thread_f(gpointer data)
{
	Installer* inst = (Installer*) data;
	std::vector<Job*>& jobs = inst->GetJobs();
	try
	{
		for (size_t i = 0; i < jobs.size(); i++)
		{
			Job *j = jobs.at(i);
			inst->SetCurrentJob(j);
			j->Unzip();
		}
	}
	catch (std::exception& e)
	{
		std::string message = e.what();
		inst->SetError(message);
	}
	catch (std::string& e)
	{
		inst->SetError(e);
	}
	catch (...)
	{
		std::string message = "Unknown error";
		inst->SetError(message);
	}

	inst->SetRunning(false);
	inst->SetCurrentJob(NULL);
	return NULL;
}
void Installer::ShowError()
{
	
	if (this->error != "")
	{
		GtkWidget* dialog = gtk_message_dialog_new(
			NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"%s",
			this->error.c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

static void cancel_cb(GtkWidget *widget, gpointer data)
{
	Installer* i = (Installer*) data;
	i->Cancel();
}

static void destroy_cb(GtkWidget *widget, gpointer data)
{
	Installer* i = (Installer*) data;
	i->SetWindow(NULL);
	i->Cancel();
}

int get_installation_type(std::string system_runtime_home)
{

	GtkWidget* dialog = gtk_dialog_new();

	/* Titanium icon */
	GdkColormap* colormap = gtk_widget_get_colormap(dialog);
	GdkBitmap *mask = NULL;
	GdkPixmap* icon = gdk_pixmap_colormap_create_from_xpm_d(
		NULL,
		colormap,
		&mask,
		NULL,
		(gchar**) titanium_xpm);
	GtkWidget* image = gtk_image_new_from_pixmap(icon, mask);

	/* Install dialog label */
	GtkWidget* label = gtk_label_new("This application needs additional components which will be downloaded from the network. Press install to continue.");
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

	/* Install type combobox */
	GtkListStore* store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, GTK_STOCK_HOME,
		1, "Install to my home directory", -1);
	std::string text = std::string("Install to ") + system_runtime_home;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		0, GTK_STOCK_DIALOG_AUTHENTICATION,
		1, text.c_str(), -1);
	GtkWidget* install_type = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));

	GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(install_type), renderer, FALSE);
	gtk_cell_layout_set_attributes(
		GTK_CELL_LAYOUT(install_type), renderer,
		"stock-id", 0, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(install_type), renderer, TRUE);
	gtk_cell_layout_set_attributes(
		GTK_CELL_LAYOUT(install_type), renderer,
		"text", 1, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(install_type), 0);

	/* Pack label and combobox into vbox */
	GtkWidget* labelbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(labelbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(labelbox), install_type, FALSE, FALSE, 10);

	/* Pack image and labelbox vbox into top_box */
	GtkWidget* top_box = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(top_box), 10);
	gtk_box_pack_start(GTK_BOX(top_box), image, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(top_box), labelbox, FALSE, FALSE, 5);

	GtkWidget* top_part = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_pack_start(GTK_BOX(top_part), top_box, FALSE, FALSE, 0); 

	GtkWidget* cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

	GtkWidget* install = gtk_button_new_with_label("install");
	GtkWidget* install_icon = gtk_image_new_from_stock(
		GTK_STOCK_OK,
		GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(install), install_icon);

	GtkWidget* action_area = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
	gtk_box_set_homogeneous(GTK_BOX(action_area), FALSE);

	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), cancel, CANCEL);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), install, CANCEL+1);

	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_title(GTK_WINDOW(dialog), "Additional application files required");
	gtk_window_set_default_size(GTK_WINDOW(dialog), 325, 100);

	gtk_widget_show_all(dialog);
	gint r = gtk_dialog_run(GTK_DIALOG(dialog));
	gint subr = gtk_combo_box_get_active(GTK_COMBO_BOX(install_type));

	if (r != CANCEL && subr == 0)
	{
		r = HOMEDIR_INSTALL;
	}
	else if (r != CANCEL && subr == 1)
	{
		r = SYSTEM_INSTALL;
	}
	gtk_widget_destroy(dialog);

	return r;
}

int do_install(std::string directory, int argc, char* argv[])
{
	printf("Installing to: %s\n", directory.c_str());
	std::string app_name = argv[4];
	std::string temp_dir = argv[5];

	Installer i = Installer(app_name);
	curl_global_init(CURL_GLOBAL_ALL);
	Job::Init(temp_dir, directory);

	int urls_start = 6;
	for (int u = urls_start; u < argc; u++)
	{
		printf("%s\n", argv[u]);
		i.AddJob(argv[u]);
	}

	i.Run();
	i.ClearJobs();
	Job::ShutdownDownloader();

	return 0;
}

// TODO: Switch to PolicyKit
int do_install_sudo(int argc, char* argv[])
{
	// Copy all but the first command-line arg
	std::vector<std::string> args;
	args.push_back("--description");
	args.push_back("Titanium Network Installer");

	args.push_back("--");
	args.push_back(argv[0]);
	args.push_back("--system");
	for (int i = 2; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	// Restart in a sudoed environment
	std::string cmd = "gksudo";
	int r = kroll::FileUtils::RunAndWait(cmd, args);
	if (r == 127)
	{
		// Erase gksudo specific options
		args.erase(args.begin());
		args.erase(args.begin());
		cmd = std::string("kdesudo");
		args.insert(args.begin(), "The Titanium network installer needs adminstrator privileges to run. Please enter your password.");
		args.insert(args.begin(), "--comment");
		args.insert(args.begin(), "-d");
		r = kroll::FileUtils::RunAndWait(cmd, args);
	}
	if (r == 127)
	{
		// Erase kdesudo specific option
		args.erase(args.begin());
		args.erase(args.begin());
		args.erase(args.begin());
		cmd = std::string("xterm");
		args.insert(args.begin(), "sudo");
		args.insert(args.begin(), "-e");
		r = kroll::FileUtils::RunAndWait(cmd, args);
	}
	return r;
}

int choose_install_path(int argc, char* argv[])
{
	std::string type = argv[1];
	std::string system_runtime_home = argv[2];
	std::string user_runtime_home = argv[3];

	if (type == "--initial")
	{
		int result = get_installation_type(system_runtime_home);
		if (result == HOMEDIR_INSTALL)
		{
			return do_install(user_runtime_home, argc, argv);
		}
		else if (result == SYSTEM_INSTALL)
		{
			return do_install_sudo(argc, argv);
		}
		else // cancelled
		{
			return 0; 
		}
	}
	else // We've entered after a sudo
	{
		return do_install(system_runtime_home, argc, argv);
	}
}

int main(int argc, char* argv[])
{
	gtk_init(&argc, &argv);

	if (argc < 6)
	{
		std::cerr << "Not enough arguments given, aborting." << std::endl;
		exit(2);
	}
	return choose_install_path(argc, argv);
}
