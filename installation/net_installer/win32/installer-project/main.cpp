/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <windows.h>
#include <new.h>
#include <objbase.h>
#include <Wininet.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils.h>
#include "Progress.h"
#include "Resource.h"
#include "IntroDialog.h"

using std::string;
using std::wstring;
using std::vector;
using kroll::Application;
using kroll::FileUtils;
using kroll::BootUtils;

HINSTANCE mainInstance;
HICON mainIcon;

Application* app;
string updateFile;
string appPath;
string runtimeHome;
string appInstallPath;
string componentInstallPath;
string temporaryPath;
bool doInstall = false;

wstring StringToWString(string in)
{
	wstring out(in.length(), L' ');
	copy(in.begin(), in.end(), out.begin());
	return out; 
}

string WStringToString(wstring in)
{
	// XXX: Not portable
	string s(in.begin(), in.end());
	s.assign(in.begin(), in.end());
	return s;
}

void ShowError(string msg)
{
	wstring wmsg = StringToWString(msg);
	MessageBoxW(
		GetDesktopWindow(),
		wmsg.c_str(),
		L"Installation Failed",
		MB_OK | MB_SYSTEMMODAL | MB_ICONEXCLAMATION);
}

std::wstring ParseQueryParam(string uri8, string key8)
{
	std::wstring uri = StringToWString(uri8);
	std::wstring key = StringToWString(key8);
	key+=L"=";
	size_t pos = uri.find(key);
	if (pos!=std::wstring::npos)
	{
		std::wstring p = uri.substr(pos + key.length());
		pos = p.find(L"&");
		if (pos!=std::wstring::npos)
		{
			p = p.substr(0,pos);
		}

		// decode
		WCHAR szOut[INTERNET_MAX_URL_LENGTH];
		DWORD cchDecodedUrl = INTERNET_MAX_URL_LENGTH;
		CoInternetParseUrl(p.c_str(), PARSE_UNESCAPE, 0, szOut, INTERNET_MAX_URL_LENGTH, &cchDecodedUrl, 0);
		p.assign(szOut);

		return p;
	}
	return L"";
}

std::wstring ProgressString(DWORD size, DWORD total)
{
	char str[1024];

#define KB 1024
#define MB KB * 1024

	if (total < KB) {
		sprintf(str, "%0.2f of %0.2f bytes", size, total);
	}

	else if (size < MB) {
		double totalKB = total/1024.0;
		double sizeKB = size/1024.0;
		sprintf(str, "%0.2f of %0.2f KB", sizeKB, totalKB);
	}
	
	else {
		// hopefully we shouldn't ever need to count more than 1023 in a single file!
		double totalMB = total/1024.0/1024.0;
		double sizeMB = size/1024.0/1024.0;
		sprintf(str, "%0.2f of %0.2f MB", size, total);
	}
	
	wchar_t wstr[1024];
	mbtowc(wstr, str, strlen(str));
	return std::wstring(wstr);
}

bool DownloadURL(Progress *p, HINTERNET hINet, std::wstring url, std::wstring outFilename)
{
	WCHAR szDecodedUrl[INTERNET_MAX_URL_LENGTH];
	DWORD cchDecodedUrl = INTERNET_MAX_URL_LENGTH;
	WCHAR szDomainName[INTERNET_MAX_URL_LENGTH];

	// parse the URL
	HRESULT hr = CoInternetParseUrl(url.c_str(), PARSE_DECODE, URL_ENCODING_NONE, szDecodedUrl, INTERNET_MAX_URL_LENGTH, &cchDecodedUrl, 0);
	if (hr != S_OK)
	{
		return false;
	}

	// figure out the domain/hostname
	hr = CoInternetParseUrl(szDecodedUrl, PARSE_DOMAIN, 0, szDomainName, INTERNET_MAX_URL_LENGTH, &cchDecodedUrl, 0);
	if (hr != S_OK)
	{
		return false;
	}
	
	// start the HTTP fetch
	HINTERNET hConnection = InternetConnectW( hINet, szDomainName, 80, L" ", L" ", INTERNET_SERVICE_HTTP, 0, 0 );
	if ( !hConnection )
	{
		return false;
	}
	
	std::wstring wurl(szDecodedUrl);
	std::wstring path = wurl.substr(wurl.find(szDomainName)+wcslen(szDomainName));
	//std::wstring queryString = url.substr(url.rfind("?")+1);
	//astd::wstring object = path + "?" + queryString;
	
	HINTERNET hRequest = HttpOpenRequestW( hConnection, L"GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_COOKIES|INTERNET_FLAG_NO_UI|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, 0 );

	if ( !hRequest )
	{
		InternetCloseHandle(hConnection);
		return false;
	}

	// now stream the resulting HTTP into a file
	std::ofstream ostr;
	ostr.open(outFilename.c_str(), std::ios_base::binary | std::ios_base::trunc);

	bool failed = false;
	CHAR buffer[2048];
	DWORD dwRead;
	DWORD total = 0;
	wchar_t msg[255];
	
	HttpSendRequest( hRequest, NULL, 0, NULL, 0);
	while( InternetReadFile( hRequest, buffer, 2047, &dwRead ) )
	{
		if ( dwRead == 0)
		{
			break;
		}
		if (p->IsCancelled())
		{
			failed = true;
			break;
		}
		buffer[dwRead] = '\0';
		total+=dwRead;
		ostr.write(buffer, dwRead);
		wsprintfW(msg,L"Downloaded %d KB",total/1024);
		p->SetLineText(2,msg,true);
	}
	ostr.close();
	InternetCloseHandle(hConnection);
	InternetCloseHandle(hRequest);

	return !failed;
}

void CreateDirectoryTree(string dir)
{
	if(!FileUtils::IsDirectory(dir))
	{
		int lastSlash = dir.find_last_of("\\");
		if(lastSlash != string::npos)
		{
			string parent = dir.substr(0, lastSlash);
			CreateDirectoryTree(parent);
		}
		CreateDirectoryA(dir.c_str(), NULL);
	}
}

void Install(string type, string name, string version, string path)
{
	string destination;
	if (type == "modules")
		destination = FileUtils::Join(
			componentInstallPath.c_str(), "modules", OS_NAME, name, version, NULL);
	else if (type == "runtime")
		destination = FileUtils::Join(
			componentInstallPath.c_str(), "runtime", OS_NAME, version, NULL);
	else if (type == "update")
		destination == app->path;

	// Recursively create directories
	CreateDirectoryTree(destination);
	FileUtils::Unzip(path, destination);
}

void ProcessUpdate(Progress *p, HINTERNET hINet)
{
	string type = "update";
	string version = app->version;
	string name = app->name;
	string url = app->GetUpdateURL();

	string path = type + "-";
	path.append("update-");
	path.append(version + ".zip");
	path = FileUtils::Join(temporaryPath.c_str(), path.c_str(), NULL);

		// Figure out the path and destination
	bool downloaded = DownloadURL(p, hINet, StringToWString(url), StringToWString(path));
	if (downloaded)
	{
		p->SetLineText(2, string("Installing ") + name + "-" + version + "...", true);
		Install(type, name, version, path);
	}
}

void ProcessURL(string url, Progress *p, HINTERNET hINet)
{
	std::wstring wuuid = ParseQueryParam(url, "uuid");
	std::wstring wname = ParseQueryParam(url, "name");
	std::wstring wversion = ParseQueryParam(url, "version");
	string uuid = WStringToString(wuuid);
	string name = WStringToString(wname);
	string version = WStringToString(wversion);
	string type = "";

	if (string(RUNTIME_UUID) == uuid)
		type = "runtime";
	else if (string(MODULE_UUID) == uuid)
		type = "modules";
	else
		return;

	string path = type + "-";
	path.append(name + "-");
	path.append(version + ".zip");
	path = FileUtils::Join(temporaryPath.c_str(), path.c_str(), NULL);

	// Figure out the path and destination
	bool downloaded = DownloadURL(p, hINet, StringToWString(url), StringToWString(path));
	if (downloaded)
	{
		p->SetLineText(2, string("Installing ") + name + "-" + version + "...", true);
		Install(type, name, version, path);
	}
}

void ProcessFile(string path, Progress *p)
{
	string type = "";
	string name = "";
	string version = "";

	size_t start, end;
	size_t lastSlash = path.find_last_of("\\");
	if (lastSlash == std::string::npos)
	{
		lastSlash = -1;
	}
	string basename = path.substr(lastSlash + 1);

	end = path.find("-");
	std::string partOne = path.substr(0, end);
	if (partOne == "runtime")
	{
		type = name = "runtime";
	}
	else if (partOne == "module")
	{
		type = "modules";
		start = end + 1;
		end = path.find("-", start);
		name = path.substr(start, end - start);
	}

	start = end + 1;
	end = path.find(".zip", start);
	version = path.substr(start, end - start);

	p->SetLineText(2, string("Installing ") + name + "-" + version + "...", true);
	Install(type, name, version, path);
}

bool InstallApplication(Progress *p)
{
	if (!app->IsInstalled())
	{
		p->SetLineText(2, string("Installing to ") + appInstallPath, true);
		FileUtils::CreateDirectory(appInstallPath);
		FileUtils::CopyRecursive(app->path, appInstallPath);
	}
	return true;
}


bool HandleAllJobs(vector<string> jobs, Progress* p)
{
	temporaryPath = FileUtils::GetTempDirectory();

	int count = jobs.size();
	bool success = true;

	// Create our progress indicator class
	p->SetTitle(L"Titanium Installer");
	p->SetCancelMessage(L"Cancelling, one moment...");

	wchar_t buf[255];
	wsprintfW(buf,L"Preparing to download %d file%s", count, (count > 1 ? L"s" : L""));
	p->SetLineText(2,std::wstring(buf),true);
	p->Update(0, count);

	// Initialize the Interent DLL
	HINTERNET hINet = InternetOpenW(
		L"Mozilla/5.0 (compatible; Titanium_Downloader/0.1; Win32)",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL, NULL, 0 );

	// For each URL, fetch the URL and then unzip it
	DWORD x = 0;
	for (int i = 0; i < jobs.size(); i++)
	{
		p->Update(x++, count);
		std::string url = jobs[i];

		if (url == string("update"))
			ProcessUpdate(p, hINet);
		if (FileUtils::IsFile(url))
			ProcessFile(url, p);
		else
			ProcessURL(url, p, hINet);
	}

	// done with iNet - so close it
	InternetCloseHandle(hINet);

	if (p->IsCancelled())
		success = false;

	if (!temporaryPath.empty()  && FileUtils::IsDirectory(temporaryPath))
		FileUtils::DeleteDirectory(temporaryPath);
	return success;
}

bool FinishInstallation()
{
	if (!app->IsInstalled())
	{
		string installedFile = FileUtils::Join(appInstallPath.c_str(), ".installed", NULL);
		FILE* file = fopen(installedFile.c_str(), "w");
		fprintf(file, "%s\n", appInstallPath.c_str());
		fclose(file);

		// Inform the boot where the application installed to
		installedFile = FileUtils::Join(app->path.c_str(), ".installedto", NULL);
		file = fopen(installedFile.c_str(), "w");
		fprintf(file, "%s\n", appInstallPath.c_str());
		fclose(file);
	}

	if(!updateFile.empty() && FileUtils::IsFile(updateFile))
	{
		DeleteFile(updateFile.c_str());
	}

	return true;
}

string GetDefaultInstallationDirectory()
{
	char path[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, path, CSIDL_PROGRAM_FILES, FALSE))
		return FileUtils::Join(path, app->name.c_str(), NULL);
	else // That would be really weird, but handle it
		return FileUtils::Join("C:", app->name.c_str(), NULL);
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	mainInstance = hInstance;
	mainIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));

	int argc = __argc;
	char** argv = __argv;
	vector<string> jobs;

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg == "-appPath" && argc > i+1)
		{
			i++;
			appPath = argv[i];
		}
		else if (arg == "-updateFile" && argc > i+1)
		{
			i++;
			updateFile = argv[i];
			jobs.push_back("update");
		}
		else
		{
			jobs.push_back(arg);
		}
	}

	if (appPath.empty())
	{
		ShowError("The installer was not given enough information to continue.");
		return __LINE__;
	}

	if (updateFile.empty())
	{
		app = BootUtils::ReadManifest(appPath);
	}
	else
	{
		app = BootUtils::ReadManifestFile(updateFile, appPath);
	}
	
	if (app == NULL)
	{
		ShowError("The installer could not read the application manifest.");
		return __LINE__;
	}

	if (!updateFile.empty())
	{
		appInstallPath = app->path;
	}
	else
	{
		appInstallPath = GetDefaultInstallationDirectory();
	}

	componentInstallPath = FileUtils::GetSystemRuntimeHomeDirectory();

	// Major WTF here, Redmond.
	LoadLibrary(TEXT("Riched20.dll"));
	CoInitialize(NULL);

	HWND introDialog = CreateDialog(
		hInstance,
		MAKEINTRESOURCE(IDD_INTRODIALOG),
		0,
		DialogProc);

	if (!introDialog)
	{
		int i = GetLastError();
		ShowError("The installer could not create the introductory dialog.");
		return __LINE__;
	}

	MSG msg;
	int status;
	while ((status = GetMessage(&msg, 0, 0, 0)) != 0)
	{
		if (status == -1)
		{
			char buf[2000];
			sprintf(buf, "Error: %i", GetLastError());
			ShowError(buf);
			return -1;
		}
		if (!IsDialogMessage(introDialog, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (doInstall)
	{
		Progress *p = new Progress;
		p->SetLineText(1, app->name, false);
		p->Show();
		bool success = 
			InstallApplication(p) &&
			HandleAllJobs(jobs, p) &&
			FinishInstallation();
		CoUninitialize();
		return success ? 0 : 1;
	}
	else
	{
		CoUninitialize();
		return 1;
	}
}
