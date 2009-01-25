/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "win32_desktop.h"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>

namespace ti
{
	Win32Desktop::Win32Desktop()
	{
	}
	Win32Desktop::~Win32Desktop()
	{
	}
	bool Win32Desktop::CreateShortcut(std::string &from, std::string &to)
	{
		HRESULT hResult;
		IShellLink* psl;

		if(from.length() == 0 || to.length() == 0) {
			std::string ex = "Invalid arguments given to createShortcut()";

			throw ex;
		}

		hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);

		if(SUCCEEDED(hResult))
		{
			IPersistFile* ppf;

			// set path to the shortcut target and add description
			psl->SetPath(from.c_str());
			psl->SetDescription("Link description goes here");

			hResult = psl->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);

			if(SUCCEEDED(hResult))
			{
				// ensure to ends with .lnk
				to.append(".lnk");
				WCHAR wsz[MAX_PATH];

				// ensure string is unicode
				if(MultiByteToWideChar(CP_ACP, 0, to.c_str(), -1, wsz, MAX_PATH))
				{
					// save the link
					hResult = ppf->Save(wsz, TRUE);
					ppf->Release();

					if(SUCCEEDED(hResult))
					{
						return true;
					}
				}
			}
		}

		return false;
	}
	SharedBoundList Win32Desktop::OpenFiles(SharedBoundObject props)
	{
		// pass in a set of properties with each key being
		// the name of the property and a boolean for its setting
		// example:
		//
		// var selected = Titanium.Desktop.openFiles({
		//    multiple:true,
		//    files:false,
		//    directories:true,
		//    types:['js','html']
		// });
		//

		// TODO need to add logic to allow selection of directories
		//bool chooseFiles = props->GetBool("files", true);
		//bool chooseDirectories = props->GetBool("directories", false);
		bool multipleFiles = props->GetBool("multiple", false);
		std::string path = props->GetString("path", "");
		std::string filename = props->GetString("filename", "");
		std::string filterName = props->GetString("typesDescription", "Filtered Files");
		std::string filter;

		// get types
		std::vector<std::string> types;
		props->GetStringList("types", types);

		if(types.size() > 0)
		{
			//"All\0*.*\0Test\0*.TXT\0";
			filter.append(filterName);
			filter.push_back('\0');

			for(int i = 0; i < types.size(); i++)
			{
				std::string type = types.at(i);

				//multiple filters: "*.TXT;*.DOC;*.BAK"
				size_t found = type.find("*.");
				if(found != 0)
				{
					filter.append("*.");
				}
				filter.append(type);
				filter.append(";");
			}

			filter.push_back('\0');
		}

		OPENFILENAME ofn;
		char filen[255];
		HWND hWindow;		// NEED to get handle to main or current titanium user window

		ZeroMemory(&filen, sizeof(filen));

		if(filename.size() == 0)
		{
			filen[0] = '\0';
		}
		else
		{
			strcpy(filen, filename.c_str());
		}

		// init OPENFILE
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = filen;
		ofn.nMaxFile = sizeof(filen);
		ofn.lpstrFilter = (filter.length() == 0 ? NULL : filter.c_str());
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = (path.length() == 0 ? NULL : path.c_str());
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		if(multipleFiles) ofn.Flags |= OFN_ALLOWMULTISELECT;

		SharedBoundList results = new StaticBoundList();

		// display the open dialog box
		if(GetOpenFileName(&ofn) == TRUE)
		{
			// if the user selected multiple files, ofn.lpstrFile is a NULL-separated list of filenames
			// if the user only selected one file, ofn.lpstrFile is a normal string

			std::vector<std::string> tokens;
			ParseStringNullSeparated(ofn.lpstrFile, tokens);

			if(tokens.size() == 1)
			{
				std::string n(tokens.at(0));

				SharedValue f = Value::NewString(n.c_str());
				results->Append(f);
			}
			else if(tokens.size() > 1)
			{
				std::string directory(tokens.at(0));
				for(int i = 1; i < tokens.size(); i++)
				{
					std::string n;
					n.append(directory.c_str());
					n.append("\\");
					n.append(tokens.at(i).c_str());

					SharedValue f = Value::NewString(n.c_str());
					results->Append(f);
				}
			}
		}


		return results;
	}
	bool Win32Desktop::OpenApplication(std::string &name)
	{
		return false;
	}
	bool Win32Desktop::OpenURL(std::string &url)
	{
		long response = (long)ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return (response > 0);
	}
	int Win32Desktop::GetSystemIdleTime()
	{
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		return (int)idleTicks;
	}

	void Win32Desktop::ParseStringNullSeparated(const char *s, std::vector<std::string> &tokens)
	{
		std::string token;

		// input string is expected to be composed of single-NULL-separated tokens, and double-NULL terminated
		int i = 0;
		while(true)
		{
			char c;

			c = s[i++];

			if(c == '\0')
			{
				// finished reading a token, save it in tokens vectory
				tokens.push_back(token);
				token.clear();

				c = s[i];		// don't increment index because next token loop needs to read this char again

				// if next char is NULL, then break out of the while loop
				if(c == '\0')
				{
					break;	// out of while loop
				}
				else
				{
					continue;	// read next token
				}
			}

			token.push_back(c);
		}
	}
}
