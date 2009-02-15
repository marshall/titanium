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

Installer::Installer(
	std::string app_name,
	std::string confirm_title,
	std::string message)
	: app_name(app_name),
	  confirm_title(confirm_title),
	  message(message),
	  current_job(NULL),
	  cancel(false),
	  error("")
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
	for (size_t i = 0; i < jobs.size(); i++)
	{
		delete jobs.at(i);
	}
	jobs.clear();
}

void Installer::Run()
{

	this->SetRunning(true);

	GtkWidget* dialog = gtk_message_dialog_new(
		NULL,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		"%s",
		this->message.c_str());
	gtk_window_set_title(GTK_WINDOW(dialog), this->confirm_title.c_str());
	gint r = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if (r == GTK_RESPONSE_YES)
	{
	
		this->StartDownloading();

		int timer = g_timeout_add(100, watcher, this);
		gtk_widget_show_all(this->window);

		gdk_threads_enter();
		gtk_main();
		gdk_threads_leave();

		g_source_remove(timer);

	}

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

	if (i->DownloadFinished())
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


int main(int argc, char* argv[])
{
	gtk_init(&argc, &argv);
	curl_global_init(CURL_GLOBAL_ALL);

	std::string app_name = argv[1];
	std::string confirm_title = argv[2];
	std::string message = argv[3];
	std::string temp_dir = argv[4];
	std::string install_dir = argv[5];
	std::string unzipper = argv[6];
	int urls_start = 7;

	Installer i = Installer(app_name, confirm_title, message);

	Job::Init(temp_dir, install_dir);

	for (int u = urls_start; u < argc; u++)
		i.AddJob(argv[u]);

	i.Run();
	i.ClearJobs();

	Job::ShutdownDownloader();

}
