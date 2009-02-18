/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
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

	bool Win32Desktop::OpenApplication(std::string &name)
	{
		// this can actually open applications or documents (wordpad, notepad, file-test.txt, etc.)
		char *dir = NULL;
		if (FileUtils::IsFile(name))
		{
			// start with the current working directory as the directory of the program
			dir = (char*)FileUtils::GetDirectory(name).c_str();
		}
		long response = (long)ShellExecuteA(NULL, "open", name.c_str(), NULL, dir, SW_SHOWNORMAL);
		return (response > 32);
	}

	bool Win32Desktop::OpenURL(std::string &url)
	{
		long response = (long)ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return (response > 32);
	}

}
