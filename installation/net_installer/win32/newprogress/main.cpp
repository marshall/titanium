/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <windows.h>
#include <objbase.h>
#include <Wininet.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include "Progress.h"
#include "Resource.h"

#define alert(msg) MessageBoxA(GetDesktopWindow(), msg, "Message", MB_OK);
#define walert(msg) MessageBoxW(GetDesktopWindow(), msg, L"Message", MB_OK);

#define DISTRIBUTION_UUID	L"7F7FA377-E695-4280-9F1F-96126F3D2C2A"
#define RUNTIME_UUID		L"A2AC5CB5-8C52-456C-9525-601A5B0725DA"
#define MODULE_UUID			L"1ACE5D3A-2B52-43FB-A136-007BD166CFD0"


std::wstring ParseQueryParam(std::wstring uri, std::wstring name)
{
	std::wstring key = name;
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
	HRESULT hr = CoInternetParseUrl(url.c_str(), PARSE_DECODE, URL_ENCODING_NONE, szDecodedUrl,
                    INTERNET_MAX_URL_LENGTH, &cchDecodedUrl, 0);
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

	return ! failed;
}

BOOL DirectoryExists(std::wstring dirName)
{
	DWORD attribs = ::GetFileAttributesW(dirName.c_str());

	if (attribs == INVALID_FILE_ATTRIBUTES) {
		return false;
	}

	return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

void CreateDirectoryTree(std::wstring dir)
{
	if(! DirectoryExists(dir))
	{
		// ensure parent dir exits
		int lastSlash = dir.find_last_of(L"\\");
		if(lastSlash != std::wstring::npos)
		{
			std::wstring parent = dir.substr(0, lastSlash);

			CreateDirectoryTree(parent);
		}

		// by this point, all parent dirs are created
		CreateDirectoryW(dir.c_str(), NULL);
	}
}

bool UnzipFile(std::wstring unzipper, std::wstring zipFile, std::wstring destdir)
{
	// ensure destdir exists
	CreateDirectoryTree(destdir);

	// now we're going to invoke back into the boot to unzip our file and install
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb = sizeof(si);

	std::wstring cmdline = L"\"";
	cmdline+=unzipper;
	cmdline+=L"\" --tiunzip \"";
	cmdline+=zipFile;
	cmdline+=L"\" \"";
	cmdline+=destdir;
	cmdline+=L"\"";

	// in win32, we just invoke back the same process and let him unzip
	if (!CreateProcessW(NULL,(LPWSTR)cmdline.c_str(),NULL,NULL,FALSE,NULL,NULL,NULL,&si,&pi))
	{
		return false;
	}

	// wait for the process to finish unzipping
	WaitForSingleObject(pi.hProcess,INFINITE);

	return true;
}
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
	//TODO: set the icon for this app and dialogs
//	HICON icon = LoadIcon(hInstance,MAKEINTRESOURCE(IDR_MAINFRAME));
//	SendMessage(NULL, WM_SETICON, (WPARAM)true, (LPARAM)icon);

	// get the command line
	LPWSTR cmdline = GetCommandLineW();
    int argcount = 0;
	LPWSTR *args = CommandLineToArgvW(cmdline,&argcount);
	
	// we must have at least the mandatory args + 1 URL
	if (argcount < 7)
	{
		MessageBoxW(GetDesktopWindow(),L"Invalid arguments passed to Installer",L"Application Error",MB_OK|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
		return 1;
	}

	std::wstring appname = args[1];
	std::wstring title = args[2];
	std::wstring message = args[3];
	std::wstring appTitle = appname + L" Installer";
	std::wstring tempdir = args[4];
	std::wstring installdir = args[5];
	std::wstring unzipper = args[6];

	// verify the installation
	std::wstring tempTitle = appname + L" - " + title;
	if (IDOK != MessageBoxW(GetDesktopWindow(),message.c_str(),tempTitle.c_str(),MB_ICONINFORMATION|MB_OKCANCEL|MB_TOPMOST))
	{
		MessageBoxW(GetDesktopWindow(),L"Installation Aborted. To install later, re-run the application again.", tempTitle.c_str(), MB_OK|MB_ICONWARNING|MB_TOPMOST);
		return 1;
	}

	int count = argcount - 7;
	int startAt = 7;

	CoInitialize(NULL);

	// create our progress indicator class
	Progress *p = new Progress;
	p->SetTitle(appTitle.c_str());
	p->SetCancelMessage(L"Cancelling, one moment...");

	wchar_t buf[255];
	wsprintfW(buf,L"Preparing to download %d file%s", count, (count > 1 ? L"s" : L""));

	p->SetLineText(1,std::wstring(buf),true);
	p->Show();
	p->Update(0,count);

	// initialize interent DLL
	HINTERNET hINet = InternetOpenW(L"Mozilla/5.0 (compatible; Titanium_Downloader/0.1; Win32)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );

	// for each URL, fetch the URL and then unzip it
	bool failed = false;
	DWORD x = 0;
	for (int c=startAt;c<argcount;c++)
	{
		p->Update(x++,count);
		std::wstring url = args[c];

		std::wstring uuid = ParseQueryParam(url,L"uuid");
		std::wstring name = ParseQueryParam(url,L"name");
		std::wstring version = ParseQueryParam(url,L"version");
		std::wstring filename = name;
		filename+=L"-";
		filename+=version;
		filename+=L".zip";

		// figure out the path and destination
		std::wstring path = tempdir + L"\\" + filename;
		std::wstring destdir;

		if (RUNTIME_UUID == uuid)
        {
			destdir = installdir + L"\\runtime\\win32\\" + version;
        }
        else if (MODULE_UUID == uuid)
        {
			destdir = installdir + L"\\modules\\win32\\" + name + L"\\" + version;
        }
        else
        {
			continue;
        }

		bool downloaded = DownloadURL(p, hINet, url, path);

		if(downloaded)
		{
			wchar_t msg[255];
			wsprintfW(msg, L"Installing %s/%s ...",name.c_str(),version.c_str());
			p->SetLineText(2,msg,true);
			UnzipFile(unzipper, path, destdir);
		}
	}

	// done with iNet - so close it
	InternetCloseHandle(hINet);

	if (p->IsCancelled())
	{
		failed = true;
	}

	// cleanup
	delete p;

	CoUninitialize();

	return (failed) ? 1 : 0;
}
