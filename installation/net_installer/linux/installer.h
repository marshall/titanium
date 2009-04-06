/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <iostream>
#include <string>
#include <vector>
#include <gtk/gtk.h>
#include <glib.h>
#include <api/file_utils.h>

class Installer;
class Job;
#include "job.h"

class Installer
{
	public:
	Installer(std::string app_name);

	void AddJob(std::string url);
	void ClearJobs();
	void Run();
	void UpdateProgress();
	void ShowError();
	std::string GetInstallDir();

	GtkWidget* GetWindow() { return this->window; }
	void SetWindow(GtkWidget* w) { this->window = w; }

	void Cancel()
	{
		this->cancel = true;
	}
	bool IsCancelled()
	{
		return this->cancel;
	}
	bool IsRunning()
	{
		return this->running;
	}
	void SetRunning(bool running)
	{
		this->running = running;
	}
	void SetError(std::string error)
	{
		this->error = error;
	}
	Job* CurrentJob()
	{
		return this->current_job;
	}
	void SetCurrentJob(Job* job)
	{
		this->current_job = job;
	}
	std::vector<Job*>& GetJobs()
	{
		return this->jobs;
	}
	bool DownloadFinished()
	{
		return this->download_finished;
	}
	void DownloadDone()
	{
		this->download_finished = true;
	}

	void StartDownloading();
	void StartInstalling();

	private:
	std::string app_name;
	std::string system_runtime_home;
	std::string user_runtime_home;

	GtkWidget* window;
	GtkWidget* bar;
	GtkWidget* label;
	std::vector<Job*> jobs;
	Job* current_job;
	bool running;
	bool cancel;
	bool download_finished;
	GThread* download_thread;
	std::string error;
	int install_type;

};
