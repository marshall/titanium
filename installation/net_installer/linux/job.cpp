/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "installer.h"
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

int Job::total = 0;
std::string Job::download_dir = "";
std::string Job::install_dir = "";
CURL* Job::curl = NULL;

int curl_progress_func(
	Job *fetcher,
	double t, /* dltotal */
	double d, /* dlnow */
	double ultotal,
	double ulnow);
size_t curl_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t curl_read_func(void *ptr, size_t size, size_t nmemb, FILE *stream);

void Job::Init(std::string download_dir, std::string install_dir)
{
	Job::curl = curl_easy_init();
	Job::download_dir = download_dir;
	Job::install_dir = install_dir;
}

void Job::ShutdownDownloader()
{
	if (Job::curl != NULL)
	{
		curl_easy_cleanup(Job::curl);
		curl_global_cleanup();
		Job::curl = NULL;
	}
}

Job::Job(std::string url, Installer* installer) :
	url(url),
	installer(installer),
	index(++Job::total),
	progress(0.0),
	type("unknown"),
	name("unknown"),
	version("unknown")
{
	this->ParseName(url);
	std::string filename = this->type + "-" + this->name + "-" + this->version + ".zip";
	this->out_filename = Job::download_dir + "/" + filename;
}

void Job::ParseName(std::string url)
{
	size_t start, end;
	start = url.find("name=");
	if (start != std::string::npos)
	{
		start += 5;
		end = url.find("&", start);
		if (end != std::string::npos)
			this->name = url.substr(start, end - start);
	}

	start = url.find("version=");
	if (start != std::string::npos)
	{
		start += 8;
		end = url.find("&", start);
		if (end != std::string::npos)
			this->version = url.substr(start, end - start);
	}

	if (this->name == std::string("runtime"))
		this->type = "runtime";
	else
		this->type = "modules";
}

void Job::Fetch()
{
	FILE *out = NULL;
	this->progress = 0.0;

	try
	{
		if (Job::curl == NULL)
			throw std::string("Download failed: could not initialize cURL.");

		FILE* out = fopen(out_filename.c_str(), "w");
		if (out == NULL)
			throw std::string("Download failed: could not open file for writing.");

		curl_easy_setopt(Job::curl, CURLOPT_URL, this->url.c_str());
		curl_easy_setopt(Job::curl, CURLOPT_WRITEDATA, out);
		curl_easy_setopt(Job::curl, CURLOPT_WRITEFUNCTION, curl_write_func);
		curl_easy_setopt(Job::curl, CURLOPT_READFUNCTION, curl_read_func);
		curl_easy_setopt(Job::curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(Job::curl, CURLOPT_PROGRESSFUNCTION, curl_progress_func);
		curl_easy_setopt(Job::curl, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(Job::curl, CURLOPT_FOLLOWLOCATION, 1);
		CURLcode result = curl_easy_perform(Job::curl);

		if (result != CURLE_OK)
			throw std::string("Download failJob::ed: some files failed to download.");
		fflush(out);
		fclose(out);

	}
	catch (...)
	{
		printf("Error\n");
		// Cleanup
		if (out != NULL)
		{
			fflush(out);
			fclose(out);
		}
		throw;
	}

	this->progress = 1.0;
}

void Job::Unzip()
{
	this->progress = ((double) this->index) / ((double) Job::total);

	std::string outdir = Job::install_dir;
	kroll::FileUtils::CreateDirectory(outdir);
	outdir.append("/" + this->type);
	kroll::FileUtils::CreateDirectory(outdir);
	outdir.append("/linux");
	kroll::FileUtils::CreateDirectory(outdir);

	if (this->type != "runtime")
	{
		outdir.append("/" + this->name);
		kroll::FileUtils::CreateDirectory(outdir);
	}

	outdir.append("/" + this->version);
	kroll::FileUtils::CreateDirectory(outdir);

	kroll::FileUtils::Unzip(this->out_filename, outdir);
}

Installer* Job::GetInstaller()
{
	return this->installer;
}

int Job::GetIndex()
{
	return this->index;
}

std::string Job::GetFilename()
{
	return this->out_filename;
}

void Job::SetProgress(double progress)
{
	this->progress = progress;
}

double Job::GetProgress()
{
	return this->progress;
}

size_t curl_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

size_t curl_read_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fread(ptr, size, nmemb, stream);
}

int curl_progress_func(
	Job *job,
	double t, /* dltotal */
	double d, /* dlnow */
	double ultotal,
	double ulnow)
{

	if (job->GetInstaller()->IsCancelled())
		return 1;

	if (t == 0)
		job->SetProgress(0);
	else
		job->SetProgress(d/t);
	return 0;
}

