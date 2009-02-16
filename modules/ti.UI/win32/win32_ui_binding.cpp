/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"
#include "win32_menu_item_impl.h"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include "win32_tray_item.h"

namespace ti
{
	HMENU Win32UIBinding::contextMenuInUseHandle = NULL;

	Win32UIBinding::Win32UIBinding(Host *host) : UIBinding(host)
	{
	}

	Win32UIBinding::~Win32UIBinding()
	{
	}

	SharedPtr<MenuItem> Win32UIBinding::CreateMenu(bool trayMenu)
	{
		SharedPtr<MenuItem> menu = new Win32MenuItemImpl(NULL);
		return menu;
	}

	void Win32UIBinding::SetMenu(SharedPtr<MenuItem>)
	{
		// Notify all windows that the app menu has changed.
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			Win32UserWindow* wuw = dynamic_cast<Win32UserWindow*>(*i);
			if (wuw != NULL)
				wuw->AppMenuChanged();

			i++;
		}
	}

	void Win32UIBinding::SetContextMenu(SharedPtr<MenuItem> menu)
	{
		SharedPtr<Win32MenuItemImpl> menu_new = menu.cast<Win32MenuItemImpl>();

		// if same menu, just return
		if ((menu.isNull() && contextMenuInUse.isNull()) || (menu_new == contextMenuInUse))
		{
			return;
		}

		// delete old menu if available
		if(! contextMenuInUse.isNull())
		{
			contextMenuInUse->ClearRealization(contextMenuInUseHandle);
			contextMenuInUseHandle = NULL;
		}

		contextMenuInUse = menu_new;

		// create new menu if needed
		if(! contextMenuInUse.isNull())
		{
			contextMenuInUseHandle = contextMenuInUse->GetMenu();
		}
	}

	void Win32UIBinding::SetIcon(SharedString icon_path)
	{
		// Notify all windows that the app icon has changed
		// TODO this kind of notification should really be placed in UIBinding..
		std::vector<UserWindow*>& windows = UserWindow::GetWindows();
		std::vector<UserWindow*>::iterator i = windows.begin();
		while (i != windows.end())
		{
			Win32UserWindow* wuw = dynamic_cast<Win32UserWindow*>(*i);
			if (wuw != NULL)
				wuw->AppIconChanged();

			i++;
		}
	}

	SharedPtr<TrayItem> Win32UIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		SharedPtr<TrayItem> trayItem = new Win32TrayItem(icon_path, cb);
		return trayItem;
	}

	void Win32UIBinding::OpenFiles(
		SharedBoundMethod callback,
		bool multiple,
		bool files,
		bool directories,
		std::string& path,
		std::string& file,
		std::vector<std::string>& types)
	{
		// TODO this is not the logic followed by the osx
		//  desktop implementation, but as of right now
		// the windows implementation allows the user to
		// browse/select for file OR a directory, but not both
		SharedBoundList results;
		if(directories) {
			results = SelectDirectory(multiple, path, file);
		}
		else
		{
			results = SelectFile(callback, multiple, path, file, types);
		}

		ValueList args;
		args.push_back(Value::NewList(results));
		callback->Call(args);
	}

	SharedBoundList Win32UIBinding::SelectFile(
		SharedBoundMethod callback,
		bool multiple,
		std::string& path,
		std::string& file,
		std::vector<std::string>& types)
	{
		//std::string filterName = props->GetString("typesDescription", "Filtered Files");
		std::string filterName = "Filtered Files";
		std::string filter;

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

		if(file.size() == 0)
		{
			filen[0] = '\0';
		}
		else
		{
			strcpy(filen, file.c_str());
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

		if(multiple) ofn.Flags |= OFN_ALLOWMULTISELECT;

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
				results->Append(Value::NewString(tokens.at(0)));
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
					results->Append(Value::NewString(n));
				}
			}
		}
		return results;
	}

	SharedBoundList Win32UIBinding::SelectDirectory(
		bool multiple,
		std::string& path,
		std::string& file)
	{
		SharedBoundList results = new StaticBoundList();

		BROWSEINFO bi = { 0 };
		//std::string title("Select a directory");
		//bi.lpszTitle = title.c_str();
		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

		if(pidl != 0)
		{
			// get folder name
			TCHAR in_path[MAX_PATH];
			if(SHGetPathFromIDList(pidl, in_path))
			{
				results->Append(Value::NewString(std::string(in_path)));
			}
		}
		return results;
	}

	long Win32UIBinding::GetIdleTime()
	{
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		return (int)idleTicks;
	}

	void Win32UIBinding::ParseStringNullSeparated(const char *s, std::vector<std::string> &tokens)
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
