/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WIN32_MENU_H_
#define _WIN32_MENU_H_

#include "../menu.h"
#include <windows.h>

namespace ti
{
	class Win32Menu : public Menu
	{
	public:
		Win32Menu();
      	virtual ~Win32Menu();
		
		// main API
		virtual void SetTitle(std::string &title);
		virtual std::string GetTitle();
	};
}
#endif
