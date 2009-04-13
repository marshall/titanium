/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <curl/curl.h>

class Job
{
	public:
	Job(std::string url);
	void Fetch();
	void Unzip();
	std::string GetFilename();
	int GetIndex();
	void SetProgress(double progress);
	double GetProgress();
	void ParseURL(std::string url);
	void ParseFile(std::string url);
	Installer* GetInstaller();

	static int total;
	static void InitDownloader();
	static void ShutdownDownloader();

	static std::string download_dir;
	static std::string install_dir;

	int Index()
	{
		return this->index;
	}

	private:
	std::string url;
	int index;
	std::string out_filename;
	double progress;
	std::string type;
	std::string name;
	std::string version;
	bool download;

	static CURL *curl;
	static char* curl_error;
};
