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
#include "Progress.h"
#include "Resource.h"


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
		return p;
	}
	return L"";
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
	LPWSTR cmdline = GetCommandLine();
    int argcount = 0;
	LPWSTR *args = CommandLineToArgvW(cmdline,&argcount);

	// we must have at least the mandatory args + 1 URL
	if (argcount < 8)
	{
		MessageBox(GetDesktopWindow(),L"Invalid arguments passed to Installer",L"Application Error",MB_OK|MB_SYSTEMMODAL|MB_ICONEXCLAMATION);
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
	if (IDOK != MessageBox(GetDesktopWindow(),message.c_str(),title.c_str(),MB_ICONINFORMATION|MB_OKCANCEL))
	{
		MessageBox(GetDesktopWindow(),L"Installation Aborted. To install later, re-run the application again.", title.c_str(), MB_OK|MB_ICONWARNING);
		return 1;
	}

	int count = argcount - 7;
	int startAt = 7;

	CoInitialize(NULL);

	// create our progress indicator class
	Progress *p = new Progress;
	p->SetTitle(appTitle.c_str());
	p->SetCancelMessage(L"Cancelling, one moment...");

	TCHAR buf[255];
	wsprintf(buf,L"Preparing to download %d file%s", count, (count > 1 ? L"s" : L""));

	p->SetLineText(1,std::wstring(buf),true);
	p->Show();
	p->Update(0,count);

	// for each URL, fetch the URL and then unzip it
	bool failed = false;
	DWORD x = 0;
	for (int c=startAt;c<argcount;c++)
	{
		p->Update(x++,count);
		std::wstring url = args[c];

		WCHAR szDecodedUrl[INTERNET_MAX_URL_LENGTH];
		DWORD cchDecodedUrl = INTERNET_MAX_URL_LENGTH;
		WCHAR szOut[INTERNET_MAX_URL_LENGTH];

		// parse the URL
		HRESULT hr = CoInternetParseUrl(url.c_str(), PARSE_DECODE, URL_ENCODING_NONE, szDecodedUrl, 
                        INTERNET_MAX_URL_LENGTH, &cchDecodedUrl, 0);
		if (hr != S_OK)
		{
			//TODO
			failed = true;
		}

		// figure out the domain/hostname
		hr = CoInternetParseUrl(szDecodedUrl, PARSE_DOMAIN, 0, szOut, INTERNET_MAX_URL_LENGTH, &cchDecodedUrl, 0);

		if (hr != S_OK)
		{
			//TODO
			failed = true;
		}

		// continue while the user doesn't press the Cancel button
		while ( !p->IsCancelled() )
		{
			// start the HTTP fetch
			HINTERNET hINet = InternetOpen(L"Mozilla/5.0 (compatible; Titanium_Downloader/0.1; Win32)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
			HINTERNET hConnection = InternetConnect( hINet, szOut, 80, L" ", L" ", INTERNET_SERVICE_HTTP, 0, 0 );
			if ( !hConnection )
			{
				failed = true;
				InternetCloseHandle(hINet);
				break;
			}
			HINTERNET hRequest = HttpOpenRequest( hConnection, L"GET", L"", NULL, NULL, NULL, INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_COOKIES|INTERNET_FLAG_NO_UI|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, 0 );
			if ( !hRequest )
			{
				InternetCloseHandle(hConnection);
				InternetCloseHandle(hINet);
				failed = true;
				break;
			}
	
			std::wstring uuid = ParseQueryParam(std::wstring(szDecodedUrl),L"uuid");
			std::wstring name = ParseQueryParam(std::wstring(szDecodedUrl),L"name");
			std::wstring version = ParseQueryParam(std::wstring(szDecodedUrl),L"version");
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

			// now stream the resulting HTTP into a file
			std::ofstream ostr;
			ostr.open(path.c_str(), std::ios_base::binary | std::ios_base::trunc);
			
			CHAR buffer[2048];
			DWORD dwRead;
			DWORD total = 0;
			TCHAR msg[255];
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
				buffer[dwRead] = 0;
				total+=dwRead;
				ostr << buffer;
				wsprintf(msg,L"Downloaded %d bytes",total);
				p->SetLineText(2,msg,true);
			}	
			ostr.close();
			InternetCloseHandle(hConnection);
			InternetCloseHandle(hINet);
			InternetCloseHandle(hRequest);

			// now we're going to invoke back into the boot to unzip our file and install
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory( &si, sizeof(si) );
			ZeroMemory( &pi, sizeof(pi) );
			si.cb = sizeof(si);

			std::wstring cmdline = L"\"";
			cmdline+=args[0];
			cmdline+=L"\" --tiunzip \"";
			cmdline+=path;
			cmdline+=L"\" ";
			cmdline+=destdir;
			cmdline+=L"\"";

			wsprintf(msg,L"Installing %s/%s ...",name.c_str(),version.c_str());
			p->SetLineText(2,msg,true);

            // in win32, we just invoke back the same process and let him unzip
			if (!CreateProcess(NULL,(LPWSTR)cmdline.c_str(),NULL,NULL,FALSE,NULL,NULL,NULL,&si,&pi))
			{
				//TODO	
				failed = true;
			}

			// wait for the process to finish unzipping
			WaitForSingleObject(pi.hProcess,INFINITE);
		}
	}

	if (p->IsCancelled())
	{
		failed = true;
	}

	// cleanup
	delete p;

	CoUninitialize();

	return (failed) ? 1 : 0;
}
