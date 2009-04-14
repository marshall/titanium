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
#include <utils.h>

using kroll::BootUtils;
using kroll::FileUtils;
using kroll::EnvironmentUtils;
using std::string;
using std::vector;
using kroll::Application;
using kroll::KComponent;
class Installer;
class Job;
#include "job.h"

class Installer
{
	public:
	static Installer* instance;
	Installer(string, vector<Job*>, int);
	~Installer();
	void ResizeWindow(int width, int height);
	void CreateIntroView();
	void CreateProgressView();
	void CreateInfoBox(GtkWidget*);
	GtkWidget* GetTitaniumIcon();
	GtkWidget* GetApplicationIcon();

	void StartInstallProcess();
	void StartDownloading();
	void StartInstalling();
	void UpdateProgress();
	void ShowError();
	void FinishInstall();

	GtkWidget* GetWindow() { return this->window; }
	void SetWindow(GtkWidget* w) { this->window = w; }
	void Cancel();

	enum Stage
	{
		SUCCESS = 0,
		PREDOWNLOAD,
		DOWNLOADING,
		PREINSTALL,
		INSTALLING,
		CANCEL_REQUEST,
		SUDO_REQUEST,
		CANCELLED,
		ERROR
	};

	Stage GetStage()
	{
		return this->stage;
	}

	void SetStage(Stage stage)
	{
		this->stage = stage;
	}

	void SetError(std::string error)
	{
		this->error = error;
		this->stage = ERROR;
	}

	Job* CurrentJob()
	{
		return this->currentJob;
	}

	void SetCurrentJob(Job* job)
	{
		this->currentJob = job;
	}

	std::vector<Job*>& GetJobs()
	{
		return this->jobs;
	}

	std::string GetApplicationPath()
	{
		return this->applicationPath;
	}


	private:
	std::string applicationPath;
	std::vector<Job*> jobs;
	Application* app;
	int installType;
	Stage stage;

	Job* currentJob;
	bool cancel;
	std::string error;
	GtkWidget* window;
	GtkWidget* progressBar;
	GtkWidget* downloadingLabel;
	GtkWidget* installCombo;
	bool running;
	GThread* download_thread;

};
